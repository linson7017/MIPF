#include "Mipf_Plugin_ImageBatchProcessingActivator.h"
#include "ImageNormalizationView.h" 
#include "Utils/PluginFactory.h" 

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Plugin_ImageBatchProcessing_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Plugin_ImageBatchProcessing_Activator_ID[] = "Mipf_Plugin_ImageBatchProcessing_Activator_ID";

Mipf_Plugin_ImageBatchProcessing_Activator::Mipf_Plugin_ImageBatchProcessing_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
   
}

bool Mipf_Plugin_ImageBatchProcessing_Activator::Init()
{
    return true; 
}

const char* Mipf_Plugin_ImageBatchProcessing_Activator::GetID()
{
    return Mipf_Plugin_ImageBatchProcessing_Activator_ID; 
}

void Mipf_Plugin_ImageBatchProcessing_Activator::Register()
{
    REGISTER_PLUGIN("ImageNormalizationWidget", ImageNormalizationView);
}