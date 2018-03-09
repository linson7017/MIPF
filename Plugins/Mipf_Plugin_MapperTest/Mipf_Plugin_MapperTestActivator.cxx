#include "Mipf_Plugin_MapperTestActivator.h"
#include "MapperTestView.h"
#include "Res/R.h"
#include "Utils/QObjectFactory.h"

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Plugin_MapperTest_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Plugin_MapperTest_Activator_ID[] = "Mipf_Plugin_MapperTest_Activator_ID";

Mipf_Plugin_MapperTest_Activator::Mipf_Plugin_MapperTest_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
   
}

bool Mipf_Plugin_MapperTest_Activator::Init()
{
    return true; 
}

const char* Mipf_Plugin_MapperTest_Activator::GetID()
{
    return Mipf_Plugin_MapperTest_Activator_ID; 
}

void Mipf_Plugin_MapperTest_Activator::Register()
{
    REGISTER_QOBJECT("MapperTestWidget", MapperTestView);
}