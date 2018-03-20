#include "Mipf_Truncus_SegmentationActivator.h"
#include "LungSegmentationView.h" 
#include "BoneExtract.h"
#include "SkinExtractView.h"
#include "Utils/PluginFactory.h" 

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Truncus_Segmentation_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Truncus_Segmentation_Activator_ID[] = "Mipf_Truncus_Segmentation_Activator_ID";

Mipf_Truncus_Segmentation_Activator::Mipf_Truncus_Segmentation_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
   
}

bool Mipf_Truncus_Segmentation_Activator::Init()
{
    return true; 
}

const char* Mipf_Truncus_Segmentation_Activator::GetID()
{
    return Mipf_Truncus_Segmentation_Activator_ID; 
}

void Mipf_Truncus_Segmentation_Activator::Register()
{
    REGISTER_PLUGIN("LungSegmentationWidget", LungSegmentationView);
    REGISTER_PLUGIN("BoneExtractWidget", BoneExtract);
    REGISTER_PLUGIN("SkinExtractWidget", SkinExtractView);
}