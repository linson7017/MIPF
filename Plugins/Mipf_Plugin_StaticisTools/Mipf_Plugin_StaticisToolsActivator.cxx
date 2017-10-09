#include "Mipf_Plugin_StaticisToolsActivator.h"
#include "HistogramView.h"
#include "ProfileGrayScaleDistributionView.h"

#include "Res/R.h"
#include "Utils/QObjectFactory.h"

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Plugin_StaticisTools_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Plugin_StaticisTools_Activator_ID[] = "Mipf_Plugin_StaticisTools_Activator_ID";

Mipf_Plugin_StaticisTools_Activator::Mipf_Plugin_StaticisTools_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
   
}

bool Mipf_Plugin_StaticisTools_Activator::Init()
{
    return true; 
}

const char* Mipf_Plugin_StaticisTools_Activator::GetID()
{
    return Mipf_Plugin_StaticisTools_Activator_ID; 
}

void Mipf_Plugin_StaticisTools_Activator::Register(R* pR)
{
    REGISTER_QOBJECT("HistogramWidget", HistogramView);
    REGISTER_QOBJECT("ProfileGrayScaleDistributionWidget", ProfileGrayScaleDistributionView);
}