#include "ItkAlgorithmSetViewActivator.h"
#include "ItkAlgorithmSetView.h"

#include "Utils/QObjectFactory.h"


QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new ItkAlgorithmSetView_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}


const char ItkAlgorithmSetView_Activator_ID[] = "ItkAlgorithmSetView_Activator";

ItkAlgorithmSetView_Activator::ItkAlgorithmSetView_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
}

bool ItkAlgorithmSetView_Activator::Init()
{
    return true;
}

const char* ItkAlgorithmSetView_Activator::GetID()
{
    return ItkAlgorithmSetView_Activator_ID;
}

void ItkAlgorithmSetView_Activator::Register(R* pR)
{
    REGISTER_CLASS("ItkAlgorithmSetWidget", ItkAlgorithmSetView);
}
