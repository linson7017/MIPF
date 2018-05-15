#include "Mipf_Plugin_DicomActivator.h"
#include "DicomTagsView.h" 
#include "Utils/PluginFactory.h" 

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Plugin_Dicom_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Plugin_Dicom_Activator_ID[] = "Mipf_Plugin_Dicom_Activator_ID";

Mipf_Plugin_Dicom_Activator::Mipf_Plugin_Dicom_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
   
}

bool Mipf_Plugin_Dicom_Activator::Init()
{
    return true; 
}

const char* Mipf_Plugin_Dicom_Activator::GetID()
{
    return Mipf_Plugin_Dicom_Activator_ID; 
}

void Mipf_Plugin_Dicom_Activator::Register()
{
    REGISTER_PLUGIN("DicomTagsWidget", DicomTagsView);
}