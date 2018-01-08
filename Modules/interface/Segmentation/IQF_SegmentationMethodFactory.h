#ifndef IQF_SegmentationMethodFactory_h__
#define IQF_SegmentationMethodFactory_h__

#include "iconfig.h"
class IQF_FastGrowCutSegmentation;
class IQF_AirwaySegmentation;
class IQF_GraphcutSegmentation;

const char QF_Segmentation_Factory[] = "QF_Segmentation_Factory";

class  IQF_SegmentationMethodFactory
{
    BUILD_INTERFACE(IQF_SegmentationMethodFactory)
public:
    virtual IQF_FastGrowCutSegmentation* CreateFastGrowCutSegmentationMethod()=0;
    virtual IQF_AirwaySegmentation* CreateAirwaySegmentationMethod()=0;
    virtual IQF_GraphcutSegmentation* CreateGraphcutSegmentationMethod()=0;
};


#endif // IQF_SegmentationMethodFactory_h__
