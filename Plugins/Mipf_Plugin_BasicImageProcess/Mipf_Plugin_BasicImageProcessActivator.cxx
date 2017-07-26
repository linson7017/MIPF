#include "Mipf_Plugin_BasicImageProcessActivator.h"
#include "BasicImageProcessView.h"
#include "Res/R.h"
#include "Utils/QObjectFactory.h"

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Plugin_BasicImageProcess_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Plugin_BasicImageProcess_Activator_ID[] = "Mipf_Plugin_BasicImageProcess_Activator_ID";

Mipf_Plugin_BasicImageProcess_Activator::Mipf_Plugin_BasicImageProcess_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
   
}

bool Mipf_Plugin_BasicImageProcess_Activator::Init()
{
    //m_pBasicImageProcessView = new BasicImageProcessView(m_pMain); 
    return true; 
}

const char* Mipf_Plugin_BasicImageProcess_Activator::GetID()
{
    return Mipf_Plugin_BasicImageProcess_Activator_ID; 
}

void Mipf_Plugin_BasicImageProcess_Activator::Register(R* pR)
{
    //m_pBasicImageProcessView->InitResource(pR); 
   // pR->registerCustomWidget("BasicImageProcessWidget", m_pBasicImageProcessView);
    REGISTER_CLASS("BasicImageProcessWidget", BasicImageProcessView);
}