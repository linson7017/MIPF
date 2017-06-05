#include "Mipf_Plugin_GraphcutSegmentationActivator.h"
#include "GraphcutSegmentationView.h"

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Plugin_GraphcutSegmentation_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Plugin_GraphcutSegmentation_Activator_Activator_ID[] = "Mipf_Plugin_GraphcutSegmentation_Activator_Activator_ID";

Mipf_Plugin_GraphcutSegmentation_Activator::Mipf_Plugin_GraphcutSegmentation_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
}

bool Mipf_Plugin_GraphcutSegmentation_Activator::Init()
{
    m_pGraphcutSegmentationView = new GraphcutSegmentationView(m_pMain); 
    return true; 
}

const char* Mipf_Plugin_GraphcutSegmentation_Activator::GetID()
{
    return Mipf_Plugin_GraphcutSegmentation_Activator_Activator_ID; 
}

void Mipf_Plugin_GraphcutSegmentation_Activator::Register(R* pR)
{
    m_pGraphcutSegmentationView->InitResource(pR); 
}

void Mipf_Plugin_GraphcutSegmentation_Activator::Contructed(R* pR)
{
    m_pGraphcutSegmentationView->Contructed(pR);
}