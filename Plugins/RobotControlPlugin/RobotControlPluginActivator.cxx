#include "RobotControlPluginActivator.h"
#include "RobotControlView.h"
#include "Res/R.h"

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new RobotControlPlugin_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char RobotControlPlugin_Activator_Activator_ID[] = "RobotControlPlugin_Activator_Activator_ID";

RobotControlPlugin_Activator::RobotControlPlugin_Activator(QF::IQF_Main* pMain)
{
   m_PMain = pMain; 
}

bool RobotControlPlugin_Activator::Init()
{
    m_pRobotControlView = new RobotControlView(m_PMain); 
    return true; 
}

const char* RobotControlPlugin_Activator::GetID()
{
    return RobotControlPlugin_Activator_Activator_ID; 
}

void RobotControlPlugin_Activator::Register(R* pR)
{
    m_pRobotControlView->InitResource(pR); 
   // pR->Instance()->registerCustomWidget("RobotControlWidget", m_pRobotControlView);
}