#include "TestPluginActivator.h"
#include "TestView.h"

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new TestPlugin_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char TestPlugin_Activator_Activator_ID[] = "TestPlugin_Activator_Activator_ID";

TestPlugin_Activator::TestPlugin_Activator(QF::IQF_Main* pMain)
{
   m_PMain = pMain; 
}

bool TestPlugin_Activator::Init()
{
    m_pTestView = new TestView(m_PMain); 
    return true; 
}

const char* TestPlugin_Activator::GetID()
{
    return TestPlugin_Activator_Activator_ID; 
}

void TestPlugin_Activator::Register(R* pR)
{
    m_pView->InitResource(pR); 
}