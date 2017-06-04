#ifndef SOFTWARELOOKUP_H_
#define SOFTWARELOOKUP_H_

#include <maya/MGlobal.h>
#include <maya/MPoint.h>
#include <maya/MPlug.h>
#include <maya/MDagPath.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MPointArray.h>
#include <maya/MFnMesh.h>
#include <maya/MBoundingBox.h>
#include <maya/MStatus.h>
#include <maya/MObjectArray.h>
#include <maya/MObjectHandle.h>

#include <vector>
using namespace std;

class softwareLookUp
{
   public:
      softwareLookUp();
      ~softwareLookUp();

      MStatus rayTraceLookUp(const MPlug & a_Meshes, float X, float & Y,
            float Z, unsigned int intersection);
      bool setBoundingBox(const MPlug & a_Meshes);

   private:
      bool bbFromMesh(const MPlug & a_Meshes);
      bool m_bbSetup;

      MObjectHandle m_MObjectHandle;
      MFnMesh m_fnMesh;
      MFloatVector m_fvRayDir;
      MMeshIsectAccelParams m_mmAccelParams;
      MBoundingBox m_BBox;
      MObjectArray m_meshObjects;
      vector<MMeshIsectAccelParams> m_accelParamsObjects;
      MObject m_mesh;
};

#endif /* SOFTWARELOOKUP_H_ */

