/**
 * Plugin definition
 */
#include <maya/MFnPlugin.h>

#include "terrainConstraintNode.h"

#ifdef linux
#ifdef ENABLE_TIMINGS
#include "Timer.h"
#endif
#endif

MStatus initializePlugin(MObject obj)
{
   MStatus status;

   MFnPlugin plugin(obj, "", terrainConstraintNode::GET_VERSION_STRING(),
         "Any");

   status = plugin.registerNode("terrainConstraintNode",
         terrainConstraintNode::id, terrainConstraintNode::creator,
         terrainConstraintNode::initialize);
   if (!status)
   {
      status.perror("registerNode");
      return status;
   }
#ifdef ENABLE_TIMINGS
   new TimerManager();
#endif
   return status;
}

MStatus uninitializePlugin(MObject obj)
{
   MStatus status;

   MFnPlugin plugin(obj);
   status = plugin.deregisterNode(terrainConstraintNode::id);
   if (!status)
   {
      status.perror("deregisterNode");
      return status;
   }
#ifdef ENABLE_TIMINGS
   delete &TimerManager::Instance();
#endif
   return status;
}

