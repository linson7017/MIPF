#include "Mipf_Plugin_GrowcutActivator.h"
#include "GrowcutView.h"
#include "Res/R.h"

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Plugin_Growcut_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Plugin_Growcut_Activator_ID[] = "Mipf_Plugin_Growcut_Activator_ID";

Mipf_Plugin_Growcut_Activator::Mipf_Plugin_Growcut_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
   
}

bool Mipf_Plugin_Growcut_Activator::Init()
{
    m_pGrowcutView = new GrowcutView(m_pMain); 
    return true; 
}

const char* Mipf_Plugin_Growcut_Activator::GetID()
{
    return Mipf_Plugin_Growcut_Activator_ID; 
}

void Mipf_Plugin_Growcut_Activator::Register(R* pR)
{
    m_pGrowcutView->InitResource(pR); 
   // pR->registerCustomWidget("GrowcutView", m_pGrowcutView); 
}

void Mipf_Plugin_Growcut_Activator::Constructed(R* pR)
{
	m_pGrowcutView->Contructed(pR);
}