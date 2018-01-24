#include "Mipf_Plugin_TestActivator.h"
#include "TestView.h" 
#include "Utils/PluginFactory.h" 

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Plugin_Test_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Plugin_Test_Activator_ID[] = "Mipf_Plugin_Test_Activator_ID";

Mipf_Plugin_Test_Activator::Mipf_Plugin_Test_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
   
}

bool Mipf_Plugin_Test_Activator::Init()
{
    return true; 
}

const char* Mipf_Plugin_Test_Activator::GetID()
{
    return Mipf_Plugin_Test_Activator_ID; 
}

void Mipf_Plugin_Test_Activator::Register(R* pR)
{
    REGISTER_PLUGIN("TestWidget", TestView);
}