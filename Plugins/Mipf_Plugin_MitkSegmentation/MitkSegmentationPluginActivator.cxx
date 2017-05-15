#include "MitkSegmentationPluginActivator.h"
#include "MitkSegmentation.h"
#include "Res/R.h"

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new MitkSegmentationPlugin_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char MitkSegmentationPlugin_Activator_Activator_ID[] = "MitkSegmentationPlugin_Activator_Activator_ID";

MitkSegmentationPlugin_Activator::MitkSegmentationPlugin_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
}

bool MitkSegmentationPlugin_Activator::Init()
{
    m_pMitkSegmentation = new MitkSegmentation(m_pMain); 
    return true; 
}

const char* MitkSegmentationPlugin_Activator::GetID()
{
    return MitkSegmentationPlugin_Activator_Activator_ID; 
}

void MitkSegmentationPlugin_Activator::Register(R* pR)
{
    m_pMitkSegmentation->InitResource(pR); 
	pR->registerCustomWidget("SegmentationWidget", m_pMitkSegmentation);
}