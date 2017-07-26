#include "Mipf_Plugin_VesselSegmentationActivator.h"
#include "VesselSegmentationView.h"
#include "Res/R.h"
#include "Utils/QObjectFactory.h"

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Plugin_VesselSegmentation_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Plugin_VesselSegmentation_Activator_ID[] = "Mipf_Plugin_VesselSegmentation_Activator_ID";

Mipf_Plugin_VesselSegmentation_Activator::Mipf_Plugin_VesselSegmentation_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
   
}

bool Mipf_Plugin_VesselSegmentation_Activator::Init()
{
   // m_pVesselSegmentationView = new VesselSegmentationView(m_pMain); 
    return true; 
}

const char* Mipf_Plugin_VesselSegmentation_Activator::GetID()
{
    return Mipf_Plugin_VesselSegmentation_Activator_ID; 
}

void Mipf_Plugin_VesselSegmentation_Activator::Register(R* pR)
{
   // m_pVesselSegmentationView->InitResource(pR); 
   // pR->registerCustomWidget("VesselSegmentationView", m_pVesselSegmentationView); 
    REGISTER_CLASS("VesselSegmentationWidget", VesselSegmentationView);
}