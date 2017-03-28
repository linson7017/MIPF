#include "ItkAlgorithmSetViewActivator.h"
#include "ItkAlgorithmSetView.h"


QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new ItkAlgorithmSetView_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}


const char ItkAlgorithmSetView_Activator_ID[] = "ItkAlgorithmSetView_Activator";

ItkAlgorithmSetView_Activator::ItkAlgorithmSetView_Activator(QF::IQF_Main* pMain)
{
    m_PMain = pMain;
}

bool ItkAlgorithmSetView_Activator::Init()
{
    m_pView = new ItkAlgorithmSetView(m_PMain);
    return true;
}

const char* ItkAlgorithmSetView_Activator::GetID()
{
    return ItkAlgorithmSetView_Activator_ID;
}

void ItkAlgorithmSetView_Activator::Register(R* pR)
{
    m_pView->InitResource(pR);
}
