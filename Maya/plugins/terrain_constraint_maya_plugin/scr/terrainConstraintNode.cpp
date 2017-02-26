/**
 * Maya height contstraint node to y constraint transforms on a mesh 
 * Can handle multiple input shapes
 * 
 * Expected connections setup:
 * TerrainShape.worldMesh to terrain.inMesh[i]
 * 
 * Per transform:
 * transform[i].tx to terrainConstraintNode.tpx[i]
 * transform[i].tz to terrainConstraintNode.tpz[i]
 * transform[i].rotatePivot to terrainConstraintNode.rotPi[i]
 * transform[i].rotatePivotTranslate to terrainConstraintNode.rotPivTrans[i]
 * transform[i].parentMatrix to terrainConstraintNode.parent[i]
 * transform[i].parentInverseMatrix to terrainConstraintNode.parentInv[i]
 * terrainConstraintNode.positionY[i] to transform[i].translateY
 * 
 * example scene in test folder
 */
#include <maya/MIOStream.h>
#include <maya/MDGMessage.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MFloatVector.h>

#include "terrainConstraintNode.h"
#include "utils.h"

#ifdef ENABLE_TIMINGS
#include "Timer.h"
#endif

// define class's static data members
MTypeId terrainConstraintNode::id(0x00105480);
MObject terrainConstraintNode::aInMesh;

MObject terrainConstraintNode::aTrans;
MObject terrainConstraintNode::aPositionX;
MObject terrainConstraintNode::aPositionZ;

MObject terrainConstraintNode::aRotPiv;
MObject terrainConstraintNode::aRotPivTrans;
MObject terrainConstraintNode::aParent;
MObject terrainConstraintNode::aParentInv;

MObject terrainConstraintNode::aPositionY;
MObject terrainConstraintNode::aOffset;
MObject terrainConstraintNode::aVersion;
MObject terrainConstraintNode::aLogLevel;

MObject terrainConstraintNode::aDeactivate;
bool defaultValue = 1;

namespace
{
//  MAJOR      MINOR    PATCH

#ifdef HAS_OGL
unsigned int MAJOR = 1;
#else
unsigned int MAJOR = 2;
#endif

const static int MINOR = 0;
const static int PATCH = 3;
const static int VERSION = (MAJOR * 10000 + MINOR * 100 + PATCH);

std::string getVersionString()
{
   char buf[80];
   snprintf(buf, 80, "%02d.%02d.%02d", MAJOR, MINOR, PATCH);
   return buf;
}

const static std::string VERSION_STRING = getVersionString();

#ifdef ENABLE_TIMINGS
int s_FrameCount;

void FrameStart( MTime & a_Time, void * a_ClientData )
{
   LOG( PRNT, ( "=={ %6d }====================================", s_FrameCount++ ) );
   const map< string, XTimer > & timers = TimerManager::Instance().GetXTimers();
   for( map< string, XTimer >::const_iterator it = timers.begin();
         it != timers.end(); ++it )
   {
      const string & name = (*it).first;
      const XTimer & timer = (*it).second;

      LOG( PRNT, ( "T: %20.5fms (%4d) : %s", timer.GetTotalElapsed()*1000.0f, timer.GetCount(), name.c_str() ) );
   }
   TimerManager::Instance().Reset();
}

static bool setup;
#endif // ENABLE_TIMINGS
}

terrainConstraintNode::terrainConstraintNode()
{
m_swFirstRun = 1;
#ifdef ENABLE_TIMINGS
if ( !setup )
{
   MStatus status;
   MCallbackId id = MDGMessage::addTimeChangeCallback( FrameStart, NULL, &status );
   LOG( PRNT, ( "Installed callback #%d", id ) );
   setup = true;
}
#endif // ENABLE_TIMINGS
}

terrainConstraintNode::~terrainConstraintNode()
{
}

int terrainConstraintNode::GET_VERSION()
{
return VERSION;
}

const char * terrainConstraintNode::GET_VERSION_STRING()
{
return VERSION_STRING.c_str();
}

// for creating an instance of this node
void *terrainConstraintNode::creator()
{
return new terrainConstraintNode();
}

MStatus terrainConstraintNode::initializeGL()
{
return MStatus::kFailure;
}

MStatus terrainConstraintNode::initialize()
{
// CREATE AND ADD ".inMesh" ATTRIBUTE:
MFnTypedAttribute typedAttrFn;
MFnNumericAttribute numAttrFn;
MFnMatrixAttribute matAttrFn;
MFnEnumAttribute eAttrFn;

aInMesh = typedAttrFn.create("inMesh", "im", MFnData::kMesh); //world mesh
CHECK_MSTATUS(typedAttrFn.setArray(true));
CHECK_MSTATUS(typedAttrFn.setKeyable(false));
CHECK_MSTATUS(typedAttrFn.setWritable(true));
CHECK_MSTATUS(typedAttrFn.setReadable(false));
CHECK_MSTATUS(typedAttrFn.setHidden(true));
CHECK_MSTATUS(typedAttrFn.setCached(false));
CHECK_MSTATUS(typedAttrFn.setStorable(false));
CHECK_MSTATUS(typedAttrFn.setIndexMatters(false));
CHECK_MSTATUS (addAttribute(aInMesh));

aPositionY = numAttrFn.create("positionY", "tpy", MFnNumericData::kDouble);
CHECK_MSTATUS(numAttrFn.setArray(true));
CHECK_MSTATUS(numAttrFn.setStorable(true));
CHECK_MSTATUS(numAttrFn.setCached(true));
CHECK_MSTATUS(numAttrFn.setKeyable(false));
CHECK_MSTATUS(numAttrFn.setWritable(false));
CHECK_MSTATUS(numAttrFn.setReadable(true));
CHECK_MSTATUS (addAttribute(aPositionY) );

aTrans = numAttrFn.createPoint("trans", "tt"); //?
CHECK_MSTATUS(numAttrFn.setHidden(true));
CHECK_MSTATUS(numAttrFn.setArray(true));
CHECK_MSTATUS(numAttrFn.setKeyable(false));
CHECK_MSTATUS(numAttrFn.setReadable(false));
CHECK_MSTATUS(numAttrFn.setWritable(true));
CHECK_MSTATUS(numAttrFn.setCached(true));
CHECK_MSTATUS(numAttrFn.setIndexMatters(true));
CHECK_MSTATUS (addAttribute(aTrans));

aPositionX = numAttrFn.create("positionX", "tpx", MFnNumericData::kDouble);
CHECK_MSTATUS(numAttrFn.setArray(true));
CHECK_MSTATUS(numAttrFn.setKeyable(false));
CHECK_MSTATUS(numAttrFn.setReadable(false));
CHECK_MSTATUS(numAttrFn.setWritable(true));
CHECK_MSTATUS(numAttrFn.setCached(true));
CHECK_MSTATUS(numAttrFn.setIndexMatters(true));
CHECK_MSTATUS (addAttribute(aPositionX));

aPositionZ = numAttrFn.create("positionZ", "tpz", MFnNumericData::kDouble);
CHECK_MSTATUS(numAttrFn.setArray(true));
CHECK_MSTATUS(numAttrFn.setKeyable(false));
CHECK_MSTATUS(numAttrFn.setReadable(false));
CHECK_MSTATUS(numAttrFn.setWritable(true));
CHECK_MSTATUS(numAttrFn.setCached(true));
CHECK_MSTATUS(numAttrFn.setIndexMatters(true));
CHECK_MSTATUS (addAttribute(aPositionZ));

aRotPiv = numAttrFn.createPoint("rotPiv", "trp");
numAttrFn.setDefault(1.0, 0.0, 0.0);
//CHECK_MSTATUS(numAttrFn.setHidden(true));
CHECK_MSTATUS(numAttrFn.setArray(true));
CHECK_MSTATUS(numAttrFn.setKeyable(false));
CHECK_MSTATUS(numAttrFn.setReadable(false));
CHECK_MSTATUS(numAttrFn.setWritable(true));
CHECK_MSTATUS(numAttrFn.setCached(true));
CHECK_MSTATUS(numAttrFn.setIndexMatters(true));
CHECK_MSTATUS (addAttribute(aRotPiv));

aRotPivTrans = numAttrFn.createPoint("rotPivTrans", "trt");
numAttrFn.setDefault(1.0, 0.0, 0.0);
//CHECK_MSTATUS(numAttrFn.setHidden(true));
CHECK_MSTATUS(numAttrFn.setArray(true));
CHECK_MSTATUS(numAttrFn.setKeyable(false));
CHECK_MSTATUS(numAttrFn.setReadable(false));
CHECK_MSTATUS(numAttrFn.setWritable(true));
CHECK_MSTATUS(numAttrFn.setCached(true));
CHECK_MSTATUS(numAttrFn.setIndexMatters(true));
CHECK_MSTATUS (addAttribute(aRotPivTrans));

//parent matrix
aParent = matAttrFn.create("parent", "tpr");
CHECK_MSTATUS(matAttrFn.setHidden(true));
CHECK_MSTATUS(matAttrFn.setArray(true));
CHECK_MSTATUS(matAttrFn.setKeyable(false));
CHECK_MSTATUS(matAttrFn.setWritable(true));
CHECK_MSTATUS(matAttrFn.setCached(true));
CHECK_MSTATUS(matAttrFn.setReadable(false));
CHECK_MSTATUS(matAttrFn.setIndexMatters(true));
CHECK_MSTATUS (addAttribute(aParent) );

aParentInv = matAttrFn.create("parentInv", "tpi");
CHECK_MSTATUS(matAttrFn.setHidden(true));
CHECK_MSTATUS(matAttrFn.setArray(true));
CHECK_MSTATUS(matAttrFn.setKeyable(false));
CHECK_MSTATUS(matAttrFn.setWritable(true));
CHECK_MSTATUS(matAttrFn.setCached(true));
CHECK_MSTATUS(matAttrFn.setReadable(false));
CHECK_MSTATUS(matAttrFn.setIndexMatters(true));
CHECK_MSTATUS (addAttribute(aParentInv) );

aOffset = numAttrFn.create("offset", "of", MFnNumericData::kDouble, 0.0);
CHECK_MSTATUS(numAttrFn.setChannelBox(true));
CHECK_MSTATUS(numAttrFn.setStorable(true));
CHECK_MSTATUS(numAttrFn.setKeyable(false));
CHECK_MSTATUS(numAttrFn.setKeyable(false));
CHECK_MSTATUS(numAttrFn.setWritable(true));
CHECK_MSTATUS(numAttrFn.setCached(true));
CHECK_MSTATUS (addAttribute(aOffset) );

aVersion = numAttrFn.create("version", "v", MFnNumericData::kInt, GET_VERSION());
CHECK_MSTATUS(numAttrFn.setStorable(false));
CHECK_MSTATUS(numAttrFn.setKeyable(false));
CHECK_MSTATUS(numAttrFn.setWritable(true));
CHECK_MSTATUS(numAttrFn.setCached(false));
CHECK_MSTATUS (addAttribute(aVersion) );

aLogLevel = eAttrFn.create("logLevel", "ll", LOG_LEVEL());
eAttrFn.addField("debug", DEBG_LEVEL);
eAttrFn.addField("info", INFO_LEVEL);
eAttrFn.addField("warn", WARN_LEVEL);
eAttrFn.addField("errr", ERRR_LEVEL);
CHECK_MSTATUS(eAttrFn.setChannelBox(true));
CHECK_MSTATUS(eAttrFn.setStorable(false));
CHECK_MSTATUS(eAttrFn.setKeyable(false));
CHECK_MSTATUS(eAttrFn.setWritable(true));
CHECK_MSTATUS(eAttrFn.setCached(false));
CHECK_MSTATUS (addAttribute(aLogLevel) );

CHECK_MSTATUS (attributeAffects(aPositionX, aPositionY));CHECK_MSTATUS
(attributeAffects(aPositionZ, aPositionY));CHECK_MSTATUS
(attributeAffects(aParent, aPositionY));CHECK_MSTATUS
(attributeAffects(aRotPiv, aPositionY));CHECK_MSTATUS
(attributeAffects(aRotPivTrans, aPositionY));CHECK_MSTATUS
(attributeAffects(aOffset, aPositionY));

CHECK_MSTATUS (attributeAffects(aLogLevel, aPositionY));

return MS::kSuccess;
}

MStatus terrainConstraintNode::connectionBroken(const MPlug& plug,
   const MPlug& otherPlug, bool asSrc)
{
m_swFirstRun = 1;
LOG(DEBG, ("connectionBroken plug:", plug.name().asChar()));

return MS::kSuccess;
}

MStatus terrainConstraintNode::connectionMade(const MPlug &plug,
   const MPlug &otherPlug, bool asSrc)
{
if (plug == aTrans)
{
   LOG(DEBG, ("Trying to connect to aTrans, quick, do something!!"));
   m_swFirstRun = 1;
   return MS::kSuccess;
}
return MS::kUnknownParameter;
}

MStatus terrainConstraintNode::setDependentsDirty(const MPlug &plugBeingDirtied,
   MPlugArray &affectedPlugs)
{
MStatus status;

if (plugBeingDirtied.partialName() == "im")
{
   m_swFirstRun = 1;
   //LOG(DEBG,( "pB ### setDependentsDirty.: %s",  pB.name().asChar() ));
}
return status;
}

MStatus terrainConstraintNode::compute(const MPlug &plug, MDataBlock &data)
{

MStatus status = MStatus::kSuccess;

LOG(DEBG, ("%s start Compute: %d", plug.name().asChar(), m_swFirstRun));
if (plug == aPositionY || m_swFirstRun == 1)
{
   double offset = 0;
   float posX = 0., posZ = 0., Y = 0.;
   MMatrix parentMatrix, parentInv;
   MPoint rotPiv(MFloatVector::zero);
   MPoint rotPivTrans(MFloatVector::zero);
   MDataHandle tmp_handle;

   MObject thisNode = thisMObject();

   //set log level
   int logLevel = false;
   MPlug logLevelPlug(thisNode, aLogLevel);
   logLevelPlug.getValue(logLevel);
   LOG_LEVEL(logLevel);

   int j = plug.logicalIndex();
   MPlug offsetPlug(thisNode, aOffset);
   offsetPlug.getValue(offset);

   MPlug meshPlug(thisNode, aInMesh);
   //only set bounding box and ray tracing stuff on first run
   if (m_swFirstRun == 1)
   {
      bool bb;
      bb = m_swLookUp.setBoundingBox(meshPlug);
      m_swFirstRun = 0;
      LOG(DEBG, ("%d  Setup bounding box and ray tracing stuff: \n", bb));
   }
   else if (m_swFirstRun == 0 && j != -1)
   {
#ifdef ENABLE_TIMINGS
      TIMER_START(ISLoutput);
#endif
      MPlug posXPlug(thisNode, aPositionX);
      MPlug tmp_plug = posXPlug.elementByLogicalIndex(j, &status);
      if (status != 0)
         tmp_plug.getValue(posX);

      MPlug posZPlug(thisNode, aPositionZ);
      tmp_plug = posZPlug.elementByLogicalIndex(j, &status);
      if (status != 0)
         tmp_plug.getValue(posZ);

#ifdef ENABLE_TIMINGS
      TIMER_STOP(ISLoutput)
      TIMER_START(ISLoutput1);
#endif
      // you are supposed to use the inputArrayValue here,
      // but this cleans up dirty plugs first and takes for ages
      // MArrayDataHandle handleP = data.inputArrayValue( aRotPiv, &status );
      MArrayDataHandle handleP = data.outputArrayValue(aRotPiv, &status);
      status = handleP.jumpToElement(j);
      if (status != 0)
      {
         tmp_handle = handleP.inputValue(&status);
         if (status != 0)
            rotPiv = tmp_handle.asFloatVector();
      }

      MArrayDataHandle handleR = data.outputArrayValue(aRotPivTrans, &status);
      //MArrayDataHandle handleR = data.inputArrayValue( aRotPivTrans, &status);
      status = handleR.jumpToElement(j);
      if (status != 0)
      {
         tmp_handle = handleR.inputValue(&status);
         if (status != 0)
            rotPivTrans = tmp_handle.asFloatVector();
      }
      //get matrix
      MArrayDataHandle handlePa = data.outputArrayValue(aParent, &status);
      //MArrayDataHandle handlePa = data.inputArrayValue(aParent, &status);
      status = handlePa.jumpToElement(j);
      if (status != 0)
         parentMatrix = handlePa.inputValue(&status).asMatrix();
      else
         parentMatrix.setToIdentity();

#ifdef ENABLE_TIMINGS
      TIMER_STOP(ISLoutput1);
      TIMER_START(ISLoutput2);
#endif

      MPoint trans(posX, 0.0, posZ);
      MPoint pivotOffset = rotPiv + rotPivTrans;
      MPoint world = (trans + pivotOffset) * parentMatrix;

#ifdef ENABLE_TIMINGS
      TIMER_STOP(ISLoutput2);
      TIMER_START(RTLookup);
#endif

      status = m_swLookUp.rayTraceLookUp(meshPlug, world[0], Y, world[2], 0); //ray trace look up value
#ifdef ENABLE_TIMINGS
            TIMER_STOP(RTLookup)
            TIMER_START(SetResult);
#endif

      MArrayDataHandle handle = data.outputArrayValue(aParentInv);
      //MArrayDataHandle handlePi  = data.inputArrayValue(aParentInv);
      status = handle.jumpToElement(j);
      if (status != 0)
         parentInv = handle.inputValue().asMatrix();
      else
         parentInv.setToIdentity();

      //set
      MPoint world_y(world[0], Y, world[2]);
      LOG(DEBG,
            ("%d  m_softwLUPlugIndex offset: %f %f, %f \n", j, world[0], world[2], offset));

      MDataHandle floatDataHandle = data.outputValue(plug);
      floatDataHandle.set(
            (world_y * parentInv - (rotPiv + rotPivTrans))[1] + offset);
#ifdef ENABLE_TIMINGS
      TIMER_STOP(SetResult);
#endif
   }

   data.setClean(plug);
}
else
{
   return MS::kUnknownParameter;
}

return MS::kSuccess;
}

