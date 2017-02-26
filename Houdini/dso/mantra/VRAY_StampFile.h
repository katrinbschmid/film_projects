/*
 * VRAY_StampFile.h
 *
 *  Created on: Aug 4, 2010
 *      Author: katrin.schmid
*/


#ifndef __VRAY_StampFile__
#define __VRAY_StampFile__

#include <UT/UT_String.h>
#include <UT/UT_BoundingBox.h>
#include <VRAY/VRAY_Procedural.h>


class VRAY_StampFile : public VRAY_Procedural 
{
public:
             VRAY_StampFile();
    virtual ~VRAY_StampFile();

    virtual const char  *getClassName();
    virtual void  getBoundingBox(UT_BoundingBox &box);

    virtual void  render();
    virtual int  initialize(const UT_BoundingBox *);

private:
    UT_String  mBlurFile, mFile, mObject;
    fpreal mGeoSizeFactor;
    int mSizeRandomPerc;
    UT_BoundingBox   mBox;
};
#endif

