#include "SliceBySliceTrackingActivator.h"
#include "SliceBySliceTrackingView.h"


QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new SliceBySliceTrackingView_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}


const char SliceBySliceTrackingView_Activator_ID[] = "SliceBySliceTrackingView_Activator";

SliceBySliceTrackingView_Activator::SliceBySliceTrackingView_Activator(QF::IQF_Main* pMain)
{
    m_PMain = pMain;
}

bool SliceBySliceTrackingView_Activator::Init()
{
    m_pView = new SliceBySliceTrackingView(m_PMain);
    return true;
}

const char* SliceBySliceTrackingView_Activator::GetID()
{
    return SliceBySliceTrackingView_Activator_ID;
}

void SliceBySliceTrackingView_Activator::Register(R* pR)
{
    m_pView->InitResource(pR);
}
