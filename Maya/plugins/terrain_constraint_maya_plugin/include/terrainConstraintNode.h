

#include <maya/MPxNode.h>
#include <maya/MTypeId.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>

#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnMatrixData.h>

#include <maya/MDagPath.h>
#include <maya/MPlug.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <maya/MPlugArray.h>

#include <vector>
using namespace std;

#include "softwareLookUp.h"

class terrainConstraintNode: public MPxNode
{
   public:
      terrainConstraintNode();
      virtual ~terrainConstraintNode();
      static void *creator();
      static MStatus initialize();
      static MStatus initializeGL();

      virtual MStatus setDependentsDirty(const MPlug &plugBeingDirtied,
            MPlugArray &affectedPlugs);
      virtual MStatus connectionMade(const MPlug &plug, const MPlug &otherPlug,
            bool asSrc);
      virtual MStatus connectionBroken(const MPlug& plug,
            const MPlug& otherPlug, bool asSrc);
      virtual MStatus compute(const MPlug &plug, MDataBlock &data);

      static int GET_VERSION();
      static const char * GET_VERSION_STRING();

      // needs to be public
      static MTypeId id;

   private:
      static MObject aInMesh;
      static MObject aTrans;
      static MObject aPositionX;
      static MObject aPositionZ;
      static MObject aRotPiv;
      static MObject aRotPivTrans;
      static MObject aParent;
      static MObject aParentInv;

      //out
      static MObject aPositionY;

      // params
      static MObject aOffset;
      static MObject aVersion;
      static MObject aLogLevel;
      static MObject aDeactivate;

      //these are not used but kept for compatibility with earlier OpenGl releases
      /*
       static MObject aTerrainMap;
       static MObject aTerrainMapGenerated;
       static MObject aHeigthMap;
       static MObject aHeigthMapGenerated;
       */
      bool m_swFirstRun;
      softwareLookUp m_swLookUp;
      float m_lastFrame;

};
