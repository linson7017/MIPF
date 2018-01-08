#ifndef CQF_SegmentationFactory_h__
#define CQF_SegmentationFactory_h__

#pragma once

#include "Segmentation/IQF_SegmentationMethodFactory.h"

class CQF_SegmentationFactory : public IQF_SegmentationMethodFactory
{
public:
    CQF_SegmentationFactory();
    ~CQF_SegmentationFactory();
    
    virtual IQF_FastGrowCutSegmentation* CreateFastGrowCutSegmentationMethod();
    virtual IQF_AirwaySegmentation* CreateAirwaySegmentationMethod();

};
#endif // CQF_SegmentationFactory_h__

