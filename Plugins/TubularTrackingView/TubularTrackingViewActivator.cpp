#include "TubularTrackingViewActivator.h"
#include "TubularTrackingView.h"


QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new TubularTrackingView_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}


const char SliceBySliceTrackingView_Activator_ID[] = "SliceBySliceTrackingView_Activator";

TubularTrackingView_Activator::TubularTrackingView_Activator(QF::IQF_Main* pMain)
{
    m_PMain = pMain;
}

bool TubularTrackingView_Activator::Init()
{
    m_pView = new TubularTrackingView(m_PMain);
    return true;
}

const char* TubularTrackingView_Activator::GetID()
{
    return SliceBySliceTrackingView_Activator_ID;
}

void TubularTrackingView_Activator::Register(R* pR)
{
    m_pView->InitResource(pR);
}
