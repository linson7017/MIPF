#include "Mipf_Plugin_SFLSSegmentationActivator.h"
#include "SFLSSegmentationView.h"
#include "Utils/PluginFactory.h"

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Plugin_SFLSSegmentation_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Plugin_SFLSSegmentation_Activator_Activator_ID[] = "Mipf_Plugin_SFLSSegmentation_Activator_Activator_ID";

Mipf_Plugin_SFLSSegmentation_Activator::Mipf_Plugin_SFLSSegmentation_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
   
}

bool Mipf_Plugin_SFLSSegmentation_Activator::Init()
{ 
    return true; 
}

const char* Mipf_Plugin_SFLSSegmentation_Activator::GetID()
{
    return Mipf_Plugin_SFLSSegmentation_Activator_Activator_ID; 
}

void Mipf_Plugin_SFLSSegmentation_Activator::Register()
{
    REGISTER_PLUGIN("SFLSSegmentationWidget", SFLSSegmentationView);
}