#include "CQF_SegmentationFactory.h"
#include "FastGrowCut/FastGrowCutSegmenter.h"
#include "Airway/AirwaySegmentation.h"
#include "Graphcut/GraphCut.h"


CQF_SegmentationFactory::CQF_SegmentationFactory()
{
}


CQF_SegmentationFactory::~CQF_SegmentationFactory()
{
}

IQF_FastGrowCutSegmentation* CQF_SegmentationFactory::CreateFastGrowCutSegmentationMethod()
{
    return new FGC::FastGrowCut();
}

IQF_AirwaySegmentation* CQF_SegmentationFactory::CreateAirwaySegmentationMethod()
{
    return new AirwaySegmentation();
}

IQF_GraphcutSegmentation* CQF_SegmentationFactory::CreateGraphcutSegmentationMethod()
{
    return new GraphCut();
}
