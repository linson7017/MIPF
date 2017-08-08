#include "Mipf_Plugin_AirwaySegmentationActivator.h"
#include "AirwaySegmentationView.h"

#include "Utils/QObjectFactory.h"
#include "Res/R.h"

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Plugin_AirwaySegmentation_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Plugin_AirwaySegmentation_Activator_Activator_ID[] = "Mipf_Plugin_AirwaySegmentation_Activator_Activator_ID";

Mipf_Plugin_AirwaySegmentation_Activator::Mipf_Plugin_AirwaySegmentation_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
   
}

bool Mipf_Plugin_AirwaySegmentation_Activator::Init()
{
    return true; 
}

const char* Mipf_Plugin_AirwaySegmentation_Activator::GetID()
{
    return Mipf_Plugin_AirwaySegmentation_Activator_Activator_ID; 
}

void Mipf_Plugin_AirwaySegmentation_Activator::Register(R* pR)
{
    REGISTER_CLASS("AirwaySegmentationWidget", AirwaySegmentationView);
}
