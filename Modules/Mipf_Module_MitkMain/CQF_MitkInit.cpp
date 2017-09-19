#include "CQF_MitkInit.h"

#include "MitkMain/IQF_MitkDataManager.h"

#include "iqf_main.h"


CQF_MitkInit::CQF_MitkInit(QF::IQF_Main* pMain) :m_pMain(pMain)
{
}


CQF_MitkInit::~CQF_MitkInit()
{
}

void CQF_MitkInit::Init(mitk::DataStorage* dataStorager)
{
    IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    if (pDataManager)
    {
        if (dataStorager)
        {
            pDataManager->SetDataStorage(dataStorager);
        }
        pDataManager->Init();
    }
}
