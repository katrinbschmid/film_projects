/** \defgroup VRAY_StampFile
 * @file
 * @author  katrin schmid <info@lo-motion.de>
 * @version 0.1.0
 *
 * 
 * @section DESCRIPTION
 * \brief VrayProcedural/mantra geo shader
 * that creates an instances of a given geometry
 * file on each point of object with the geometry shader applied. 
 * Applies color (as Cd attribute) and scaling to instanced objects.
 * The Instance color is taken from geometry objects point Cd or 
 * a random color if no Cd attribute is found.
 * Use from "stampFile2 .otl, see .hip for an example scene. 
 * WIP, has no motion blur yet.
 * Could add more features like variation based on user definded 
 * point attributes, think about hnadling textures, animation
 *
*/

#ifdef debug
#undef debug
#endif
//#define debug 
#include <time.h>

#include <GB/GB_ExtraMacros.h>
#include <GB/GB_AttributeDefines.h>
#include <GEO/GEO_Point.h>
#include <GEO/GEO_AttributeHandle.h>
#include <GU/GU_Detail.h>
#include <GU/GU_PrimPoly.h>

#include <SYS/SYS_Math.h>
#include <UT/UT_XformOrder.h>
#include <UT/UT_Vector3.h>
#include <UT/UT_Vector4.h>
#include <UT/UT_DSOVersion.h>
#include <UT/UT_Matrix4.h>
#include <UT/UT_Matrix3.h>

#include "VRAY_StampFile.h"


/// map value to min, max range
inline fpreal remapValue(fpreal minV, fpreal maxV, fpreal value)
{
    return (minV +(maxV- minV)*value);
}

/// "private" procedural used for instancing, user can't allocate these directly
class vray_InstanceFile : public VRAY_Procedural 
{
public:
    vray_InstanceFile(UT_Vector3 pCenter, fpreal pSize, int pSizeRandom,
        UT_Vector3 pNormal, GU_Detail *pCFileGDP, UT_Vector3 pPointCd)
   : mInstCenter(pCenter), mInstSizeFactor(pSize),
          mInstSizeRandomPerc(pSizeRandom), mOrientUp(pNormal),
          mInstFileGDP(pCFileGDP),mInstDiffColor(pPointCd)
    {
        fpreal doubleSize = mInstSizeFactor * 2.;
        mInstBox.expandBounds(doubleSize, doubleSize, doubleSize);
    }
    virtual const char *getClassName();
    virtual void getBoundingBox(UT_BoundingBox &box);
    virtual int initialize(const UT_BoundingBox *);
    virtual void render();

private:
    int mInstSizeRandomPerc;
    fpreal mInstSizeFactor;
    UT_BoundingBox   mInstBox;
    GU_Detail *mInstFileGDP;
    UT_Vector3 mInstDiffColor, mInstCenter, mOrientUp;
};

static VRAY_ProceduralArg theArgs[] =
{
    VRAY_ProceduralArg("minbound", "real", "-1 -1 -1"),
    VRAY_ProceduralArg("maxbound", "real", "1 1 1"),
    VRAY_ProceduralArg("geoscale", "real", "1"),
    VRAY_ProceduralArg("randscale", "int", "1"),
    VRAY_ProceduralArg("geofile", "string", "test"),
    VRAY_ProceduralArg("blurfile", "string", ""),
    VRAY_ProceduralArg("velocityblur", "int", "0"),
    VRAY_ProceduralArg("shutter", "real", "1"),
    VRAY_ProceduralArg()
};

VRAY_Procedural * allocProcedural(const char *)
{
    return new VRAY_StampFile();
}

const VRAY_ProceduralArg * getProceduralArgs(const char *)
{
    return theArgs;
}

VRAY_StampFile::VRAY_StampFile()
{
    mBox.initBounds(0, 0, 0);
}

VRAY_StampFile::~VRAY_StampFile()
{}

const char * VRAY_StampFile::getClassName()
{
    return "VRAY_StampFile";
}

int VRAY_StampFile::initialize(const UT_BoundingBox *)
{
    fpreal val[3] = {-1., -1., -1.};
    import("minbound", val, 3);
    mBox.initBounds(val[0], val[1], val[2]);

    val[0] = val[1] = val[2] = 1;
    import("maxbound", val, 3);
    mBox.enlargeBounds(val[0], val[1], val[2]);

    if (!import("geoscale", &mGeoSizeFactor, 1))
        mGeoSizeFactor = 1;
    if (!import("randscale", &mSizeRandomPerc, 1))
        mSizeRandomPerc = 0;

    import("geofile", mFile);
    import("blurfile", mBlurFile);

    return 1;
}

void VRAY_StampFile::getBoundingBox(UT_BoundingBox &box)
{
    box = mBox;
    box.enlargeBounds(0, mGeoSizeFactor);
}

void VRAY_StampFile::render()
{
    UT_Vector3 pointCd(-1., -1. , -1.);
    UT_Vector4 pValue, nValue;

    GEO_AttributeHandle pHandle, nHandle;
    const GEO_Point * ppt = NULL;
    GU_Detail *pGuDetail;

    void *handle = queryObject(NULL);
    handle = queryObject(mObject);
    if (!handle)
    {
        fprintf(stderr, "VRAY_StampFile: Unable to find handle for %s\n", (const char *)mObject);
        return;
    }
    const GU_Detail   *sgdp;
    sgdp = queryGeometry(handle, 0);
    if (!sgdp)
    {
        fprintf(stderr, "VRAY_StampFile: There's no geometry for %s\n",   (const char *)mObject);
        return;
    }

    pGuDetail = allocateGeometry();
    // Load geo from the disk file
    if (pGuDetail->load(mFile, 0) < 0)
    {
        fprintf(stderr, "VRAY_StampFile: Unable to load geometry[0]: '%s'\n",(const char *)mFile);
        freeGeometry(pGuDetail);
        return;
    }
    int npts = sgdp->points().entries();

#ifdef debug
    std::cout <<" parent npts:    "<< npts << std::endl;
#endif
    pHandle = sgdp->getAttribute(GEO_POINT_DICT, "P");
    if (!pHandle.isAttributeValid())
    {
        fprintf(stderr, "VRAY_StampFile: no points in geometry: '%s'\n",(const char *)mFile);
        return;
    }
    nHandle = sgdp->getAttribute(GEO_POINT_DICT, "N");
    int pCdIndex = sgdp->findDiffuseAttribute (GEO_POINT_DICT);
    mBox.enlargeBounds(mGeoSizeFactor, mGeoSizeFactor, mGeoSizeFactor);
    
    for (int i = 0; i < npts; i++)// FOR_ALL_GPOINTS (gdp, ppt)
    {
        ppt = (sgdp->points()(i));
        pHandle.setElement(ppt);
        nHandle.setElement(ppt);
        if (pCdIndex >= 0)
          pointCd = *ppt->castAttribData<UT_Vector3> (pCdIndex);
        pValue = pHandle.getV3();
        nValue = nHandle.getV3();
#ifdef debug
        std::cout<< i << " i: " << nValue.x() << " Nv " << nValue.x()<< " " << nValue.z()
        << " pCdIndex: " << pCdIndex<<" " << pointCd[0] << std::endl;

#endif
        openProceduralObject();
        addProcedural(new vray_InstanceFile(pValue, mGeoSizeFactor, mSizeRandomPerc,
           nValue, pGuDetail, pointCd));
        closeObject();
    }
}

// vray_InstanceFile methods
const char * vray_InstanceFile::getClassName()
{
    return "vray_InstanceFile";
}

/// Since the procedural is generated by the Stamp, this method should never be called
int vray_InstanceFile::initialize(const UT_BoundingBox *)
{
    fprintf(stderr, "VRAY_StampFile: This method should never be called\n");
    return 0;
}

void vray_InstanceFile::getBoundingBox(UT_BoundingBox &box)
{
    box.initBounds(mInstCenter);
    box.enlargeBounds(0, mInstSizeFactor);
}

/// setup instance color and transformations, add geometry
void vray_InstanceFile::render()
{
    GU_Detail gdp(mInstFileGDP);
    GU_Detail *instGdp = NULL;
    GEO_Primitive *prim = NULL;

    UT_Matrix4 rXform(1.0);// identity
    UT_Matrix4 instXform(1.0);
    UT_Matrix3 rotXform(1.0);
    UT_Vector3 *iCd = NULL;
    instGdp = allocateGeometry();

    // set instance color
    int pCdIndex = gdp.addDiffuseAttribute(GEO_PRIMITIVE_DICT);
#ifdef debug
    std::cout<<"rgb: " << mInstDiffColor[0] <<" "<< mInstDiffColor[1] << " "<<
        mInstDiffColor[2] << std::endl;
#endif
    if (pCdIndex >= 0)
    {
        if (mInstDiffColor[0] < 0)
        {
            // TODO: use something better for seed srand
            unsigned int seed = static_cast <unsigned>(time(NULL)/mInstCenter.x()+ 1000*mInstCenter.z());
            mInstDiffColor[0] = SYSfastRandom(seed);// float random number in range [0, 1]
            mInstDiffColor[1] = SYSfastRandom(seed);
            mInstDiffColor[2] = SYSfastRandom(seed);
        }

        FOR_ALL_PRIMITIVES(&gdp, prim)
        {
            iCd = prim->castAttribData<UT_Vector3> (pCdIndex);
            iCd->assign(mInstDiffColor[0], mInstDiffColor[1], mInstDiffColor[2]);
        }
    }
    
    // transform instance, SRT tranform order
    if (mInstSizeRandomPerc > 0)
    {
        unsigned int seed = static_cast <unsigned>(time(NULL)*mInstCenter.y() - mInstCenter.x()*1000);
        fpreal dsx = SYSfastRandom(seed);// random number in range [0, 1]
        fpreal dsy = SYSfastRandom(seed);
        fpreal dsz = SYSfastRandom(seed);
#ifdef debug
    std::cout << mInstSizeRandomPerc << " dsx1: "<< dsx <<" "<< dsy<< " "<<
       dsz << std::endl;
#endif
        dsx = remapValue(-mInstSizeRandomPerc, mInstSizeRandomPerc, dsx);
        dsy = remapValue(-mInstSizeRandomPerc, mInstSizeRandomPerc, dsy);
        dsz = remapValue(-mInstSizeRandomPerc, mInstSizeRandomPerc, dsz);
#ifdef debug
        std::cout << mInstSizeRandomPerc << " dsx2: "<< dsx <<" "<< dsy<< " "<<
          dsz << std::endl;
#endif
        instXform.scale(mInstSizeFactor+dsx/100, mInstSizeFactor+dsy/100, mInstSizeFactor+dsz/100);
    }
    else
        instXform.scale(mInstSizeFactor, mInstSizeFactor, mInstSizeFactor);

    mOrientUp.normalize();
    UT_Vector3 iUp = UT_Vector3(0, 1, 0);// get up axis from object?
    rotXform.orient(iUp, mOrientUp);
    UT_Matrix4 tXform(1.0);
    tXform = rotXform;
    instXform = instXform*tXform;

#ifdef debug
    std::cout << "myDir:"<< time(NULL) <<std::endl << std::endl;
#endif

    instXform.translate(mInstCenter.x(), mInstCenter.y(), mInstCenter.z());
    gdp.transform(instXform);

    instGdp->merge(gdp);
#ifdef debug
    unsigned int inpts = instGdp->points().entries();
    unsigned int npts = gdp.points().entries();
    std::cout << mInstSizeFactor <<" child npts: " << npts << " : " << inpts <<
       " m_FileGDP: " << std::endl;
#endif

    // TODO: add motion blur
    openGeometryObject();
    addGeometry(instGdp, 0);
    closeObject();
}
