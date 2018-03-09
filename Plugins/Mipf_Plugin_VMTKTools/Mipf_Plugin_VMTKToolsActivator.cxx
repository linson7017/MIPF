#include "Mipf_Plugin_VMTKToolsActivator.h"

#include "CenterLineExtractView.h"
#include "VesselnessFilteringView.h"

#include "Res/R.h"

#include "Utils/QObjectFactory.h"

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Plugin_VMTKTools_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Plugin_VMTKTools_Activator_ID[] = "Mipf_Plugin_VMTKTools_Activator_ID";

Mipf_Plugin_VMTKTools_Activator::Mipf_Plugin_VMTKTools_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
   
}

bool Mipf_Plugin_VMTKTools_Activator::Init()
{
    return true; 
}

const char* Mipf_Plugin_VMTKTools_Activator::GetID()
{
    return Mipf_Plugin_VMTKTools_Activator_ID; 
}

void Mipf_Plugin_VMTKTools_Activator::Register()
{

    REGISTER_QOBJECT("CenterLineExtractWidget", CenterLineExtractView);
    REGISTER_QOBJECT("VesselnessFilteringWidget", VesselnessFilteringView);

    
    //m_pCenterLineExtractView->InitResource(); 
   // pR->registerCustomWidget("CenterLineExtractView", m_pCenterLineExtractView); 
}