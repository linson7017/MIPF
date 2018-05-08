#include "Mipf_Plugin_DSAToolActivator.h"
#include "DSAReader.h" 
#include "DSAHistogramView.h"
#include "DSARemoveBoneView.h"
#include "Utils/PluginFactory.h" 

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Plugin_DSATool_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Plugin_DSATool_Activator_ID[] = "Mipf_Plugin_DSATool_Activator_ID";

Mipf_Plugin_DSATool_Activator::Mipf_Plugin_DSATool_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
   
}

bool Mipf_Plugin_DSATool_Activator::Init()
{
    return true; 
}

const char* Mipf_Plugin_DSATool_Activator::GetID()
{
    return Mipf_Plugin_DSATool_Activator_ID; 
}

void Mipf_Plugin_DSATool_Activator::Register()
{
    REGISTER_PLUGIN("DSAReaderWidget", DSAReader);    
    REGISTER_PLUGIN("DSAHistogramWidget", DSAHistogramView);   
    REGISTER_PLUGIN("DSARemoveBoneWidget", DSARemoveBoneView);
}