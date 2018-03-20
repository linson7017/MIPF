#ifndef IQF_SegmentationMethodFactory_h__
#define IQF_SegmentationMethodFactory_h__

class IQF_FastGrowCutSegmentation;
class IQF_AirwaySegmentation;
class IQF_GraphcutSegmentation;
class IQF_RSSSegmentation;

const char QF_Segmentation_Factory[] = "QF_Segmentation_Factory";

class  IQF_SegmentationMethodFactory
{
public:
    virtual IQF_FastGrowCutSegmentation* CreateFastGrowCutSegmentationMethod()=0;
    virtual IQF_AirwaySegmentation* CreateAirwaySegmentationMethod()=0;
    virtual IQF_GraphcutSegmentation* CreateGraphcutSegmentationMethod()=0;
    virtual IQF_RSSSegmentation* CreateRSSSegmentationMethod() = 0;
};


#endif // IQF_SegmentationMethodFactory_h__
