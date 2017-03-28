#include "CQF_MitkMainCom.h"
#include <string>
#include <assert.h>
#include "CQF_MitkDataManager.h"
#include "CQF_MitkRenderWindow.h"
#include "CQF_MitkReference.h"

QF::IQF_Component* QF::QF_CreateComponentObject(QF::IQF_Main* pMain)
{
    QF::IQF_Component* pComponent = new CQF_MitkMain(pMain);
    assert(pComponent);
    return pComponent;
}

CQF_MitkMain::CQF_MitkMain(QF::IQF_Main* pMain):m_pMain(pMain)
{

}


CQF_MitkMain::~CQF_MitkMain()
{
}


void CQF_MitkMain::Release()
{
    delete this;
}

bool CQF_MitkMain::Init()
{
    m_pMitkDataManager = new CQF_MitkDataManager(m_pMain);
    m_pMitkRenderWindow = new CQF_MitkRenderWindow;
    m_pMitkReference = new CQF_MitkReference;
    return true;
}


int CQF_MitkMain::GetInterfaceCount()
{
    return 3;

}

const char* CQF_MitkMain::GetInterfaceID(int iID)
{

    switch (iID)
    {
    case 0:
        return QF_MitkMain_DataManager;
    case 1:
        return QF_MitkMain_RenderWindow;
    case 2:
        return QF_MitkMain_Reference;
    default:
        break;
    }
    return "";
}

void* CQF_MitkMain::GetInterfacePtr(const char* szInterfaceID)
{
    if (strcmp(szInterfaceID, QF_MitkMain_DataManager) == 0)
    {
        return m_pMitkDataManager;
    }
    else if (strcmp(szInterfaceID, QF_MitkMain_RenderWindow) == 0)
    {
        return m_pMitkRenderWindow;
    }
    else if (strcmp(szInterfaceID, QF_MitkMain_Reference) == 0)
    {
        return m_pMitkReference;
    }
    else
        return NULL;
}