#include "Mipf_Plugin_GraphcutSegmentationActivator.h"
#include "GraphcutSegmentationView.h"
#include "GraphcutSegmentationViewUi.h"

#include "Utils/PluginFactory.h"

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
    return true; 
}

const char* Mipf_Plugin_GraphcutSegmentation_Activator::GetID()
{
    return Mipf_Plugin_GraphcutSegmentation_Activator_Activator_ID; 
}

void Mipf_Plugin_GraphcutSegmentation_Activator::Register()
{
    //m_pGraphcutSegmentationView->InitResource(); 
    REGISTER_PLUGIN("GraphcutSegmentationWidget", GraphcutSegmentationViewUi);
    REGISTER_PLUGIN("GraphcutSegmentation", GraphcutSegmentationView);
}

void Mipf_Plugin_GraphcutSegmentation_Activator::Constructed()
{
}