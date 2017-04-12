#include "NDIPluginActivator.h"
#include "NDIView.h"
#include "Res/R.h"

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new NDIPlugin_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char NDIPlugin_Activator_Activator_ID[] = "NDIPlugin_Activator_Activator_ID";

NDIPlugin_Activator::NDIPlugin_Activator(QF::IQF_Main* pMain)
{
   m_PMain = pMain; 
}

bool NDIPlugin_Activator::Init()
{
    m_pNDIView = new NDIView(m_PMain); 
    return true; 
}

const char* NDIPlugin_Activator::GetID()
{
    return NDIPlugin_Activator_Activator_ID; 
}

void NDIPlugin_Activator::Register(R* pR)
{
    m_pNDIView->InitResource(pR);
    pR->registerCustomWidget("NDIWidget", m_pNDIView);
}