#include "Mipf_Plugin_ManualSegmentationTestActivator.h"
#include "ManualSegmentationTestView.h"
#include "Res/R.h"

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Plugin_ManualSegmentationTest_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Plugin_ManualSegmentationTest_Activator_ID[] = "Mipf_Plugin_ManualSegmentationTest_Activator_ID";

Mipf_Plugin_ManualSegmentationTest_Activator::Mipf_Plugin_ManualSegmentationTest_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
   
}

bool Mipf_Plugin_ManualSegmentationTest_Activator::Init()
{
    m_pManualSegmentationTestView = new ManualSegmentationTestView(m_pMain); 
    return true; 
}

const char* Mipf_Plugin_ManualSegmentationTest_Activator::GetID()
{
    return Mipf_Plugin_ManualSegmentationTest_Activator_ID; 
}

void Mipf_Plugin_ManualSegmentationTest_Activator::Register(R* pR)
{
    m_pManualSegmentationTestView->InitResource(pR); 
   // pR->registerCustomWidget("ManualSegmentationTestView", m_pManualSegmentationTestView); 
}

void Mipf_Plugin_ManualSegmentationTest_Activator::Constructed(R* pR)
{
    m_pManualSegmentationTestView->Constructed(pR);
}