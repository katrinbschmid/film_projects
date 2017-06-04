/**
* Bounding box calculation and ray intersection
*/
#include <maya/MFnMesh.h>
#include <maya/MFloatPoint.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVector.h>
#include <maya/MFloatArray.h>
#include <maya/MIntArray.h>

#include "softwareLookUp.h"
#include "utils.h"

#define ENLARGE 0.01

softwareLookUp::softwareLookUp()
{
   m_bbSetup = 0;
   m_meshObjects.clear();
   m_accelParamsObjects.clear();
}

softwareLookUp::~softwareLookUp()
{
   m_bbSetup = 0;
}

/** Brief description which ends at this dot. Details follow
 *  here.
 */
MStatus softwareLookUp::rayTraceLookUp(const MPlug & a_Meshes, float X,
      float & Y, float Z, unsigned int intersection)
{
   LOG(DEBG, ("%d: 1a_Meshes.numElements() ", a_Meshes.numElements()));
   MStatus status = MStatus::kSuccess;
   if (a_Meshes.numElements() == 0)
      return MStatus::kFailure;

   if (m_bbSetup == 1)
   {
      MPlug elementPlug;
      float maxY = m_BBox.max()[1];
      Y = (m_BBox.min())[1];
      MFloatPoint pSource = MFloatPoint(X, maxY, Z);

      MFloatPoint hitPoint;
      float fHitRayParam, fHitBary1, fHitBary2;
      int nHitFace, nHitTriangle;
      bool bIntersection = 0;
      vector<MMeshIsectAccelParams>::iterator mVectorIterator;

      if (m_meshObjects.length() != m_accelParamsObjects.size()
            || a_Meshes.numElements() != m_meshObjects.length()
            || a_Meshes.numElements() != m_accelParamsObjects.size())
      {
         setBoundingBox(a_Meshes);
      }
      LOG(DEBG,
            ("%d: %d a_Meshes.numElements() Y, %d ", a_Meshes.numElements(),
                  m_accelParamsObjects.size(), m_meshObjects.length()));

      for (int i = 0; i < m_meshObjects.length(); i++)
      {
         elementPlug = a_Meshes[i];
         // ignore deconnected mesh plugs
         if (elementPlug.isConnected())
         {
            mVectorIterator = m_accelParamsObjects.begin() + i;
            m_MObjectHandle = m_meshObjects[i];
            if (!m_MObjectHandle.isValid())
               setBoundingBox(a_Meshes);

            status = m_fnMesh.setObject(m_MObjectHandle.object());
            if (status != 0)
            {
               bool is_im = m_fnMesh.isIntermediateObject(&status);
               LOG(DEBG,
               ("%d rayTraceLookUp: setBoundingBox( is intermediate object: ", is_im));
               bIntersection = m_fnMesh.closestIntersection(pSource, m_fvRayDir,
                     NULL, NULL, false, MSpace::kWorld, (float) 9999, false,
                     &(*mVectorIterator), hitPoint, &fHitRayParam, &nHitFace,
                     &nHitTriangle, &fHitBary1, &fHitBary2, (float) 1e-6,
                     &status);
               if (bIntersection == 1)
               {
                  LOG(DEBG, ("%f: successful value Y, %f ", Y, hitPoint[1]));
                  if (hitPoint[1] > Y)
                     Y = hitPoint[1];
                  //break;
               }
               else
               {
                  LOG(DEBG, ("%d: fail value Y.", i));
               }
            }
            else
            {
               LOG(WARN, ("%d: Mesh failed. ", i));
            }
         }
      }
   }
   return status;
}

/** Brief   in scene bounding box from meshes
 *
 */
bool softwareLookUp::bbFromMesh(const MPlug & a_Meshes)
{
   MStatus status = MStatus::kSuccess;
   if (a_Meshes.numElements() == 0)
   {
      return false;
   }

   MPointArray triVerts;
   MPointArray vertexList;
   MIntArray vertexIdxs;
   MIntArray triCount;
   MPlug elementPlug;

   m_BBox = MBoundingBox();

   for (unsigned int j = 0; j < a_Meshes.numElements(); j++)
   {
      elementPlug = a_Meshes[j];
      if (elementPlug.isConnected()) // ignore deconnected mesh plugs
      {
         m_mesh = a_Meshes[j].asMObject();
         status = m_fnMesh.setObject(m_mesh);
         m_fnMesh.getPoints(vertexList, MSpace::kWorld);
         m_fnMesh.getTriangles(triCount, vertexIdxs);

         for (unsigned int i = 0; i < vertexIdxs.length(); i += 3)
         {
            m_BBox.expand(vertexList[vertexIdxs[i]]);
            triVerts.append(vertexList[vertexIdxs[i]]);
            m_BBox.expand(vertexList[vertexIdxs[i + 1]]);
            triVerts.append(vertexList[vertexIdxs[i + 1]]);
            m_BBox.expand(vertexList[vertexIdxs[i + 2]]);
            triVerts.append(vertexList[vertexIdxs[i + 2]]);
         }
      }
   }

   return true;
}

/** Brief description set bounding boxes
 *
 */
bool softwareLookUp::setBoundingBox(const MPlug & a_Meshes)
{
   MStatus status = MStatus::kSuccess;

   m_accelParamsObjects.clear();
   m_meshObjects.clear();

   bbFromMesh(a_Meshes);
   if (NearZero(m_BBox.width()) || NearZero(m_BBox.height())
         || NearZero(m_BBox.depth()))
   {
      LOG(WARN,
            ("NearZero bounding box: %9.5f, %9.5f, %9.5f, ", m_BBox.width(),
                  m_BBox.height(), m_BBox.depth()));
      MPoint center = m_BBox.center();
      m_BBox.expand(center + MPoint(ENLARGE, ENLARGE, ENLARGE));
   }

   m_bbSetup = 1;
   // setup for raytracing
   for (unsigned int i = 0; i < a_Meshes.numElements(); i++) //for each mesh
   {
      if (a_Meshes[i].isConnected())
      {
         m_mesh = a_Meshes[i].asMObject();
         m_meshObjects.append(a_Meshes[i].asMObject());
         m_MObjectHandle = m_mesh;

         status = m_fnMesh.setObject(m_mesh);
         if (status != 0)
         {
            bool is_im = m_fnMesh.isIntermediateObject(&status);
            LOG(DEBG, ("%d setBoundingBox( is intermediate object: ", is_im));
            m_mmAccelParams = m_fnMesh.autoUniformGridParams();
            m_accelParamsObjects.push_back(m_mmAccelParams);
         }
      }
   }
   // straight from above
   m_fvRayDir = MFloatVector(0., -1., 0.);

   return 1;
}

