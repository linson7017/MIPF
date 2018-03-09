#include "TubularTrackingViewActivator.h"
#include "TubularTrackingView.h"
#include "Utils/PluginFactory.h"


QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new TubularTrackingView_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}


const char SliceBySliceTrackingView_Activator_ID[] = "SliceBySliceTrackingView_Activator";

TubularTrackingView_Activator::TubularTrackingView_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
}

bool TubularTrackingView_Activator::Init()
{
    return true;
}

const char* TubularTrackingView_Activator::GetID()
{
    return SliceBySliceTrackingView_Activator_ID;
}

void TubularTrackingView_Activator::Register()
{
    REGISTER_PLUGIN("TubularTracking", TubularTrackingView);
}
