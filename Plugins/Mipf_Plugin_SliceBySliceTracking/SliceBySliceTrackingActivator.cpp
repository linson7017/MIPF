#include "SliceBySliceTrackingActivator.h"
#include "SliceBySliceTrackingView.h"

#include "Utils/PluginFactory.h"

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new SliceBySliceTrackingView_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}


const char SliceBySliceTrackingView_Activator_ID[] = "SliceBySliceTrackingView_Activator";

SliceBySliceTrackingView_Activator::SliceBySliceTrackingView_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
}

bool SliceBySliceTrackingView_Activator::Init()
{
    return true;
}

const char* SliceBySliceTrackingView_Activator::GetID()
{
    return SliceBySliceTrackingView_Activator_ID;
}

void SliceBySliceTrackingView_Activator::Register()
{
    REGISTER_PLUGIN("SliceBySliceTracking", SliceBySliceTrackingView);
}

void SliceBySliceTrackingView_Activator::Constructed()
{
    //m_pView->Constructed();
}
