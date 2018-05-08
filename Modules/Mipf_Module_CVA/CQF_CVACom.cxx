#include "CQF_CVACom.h"
#include <string>
#include <assert.h>

#include "CQF_CVACommand.h"
#include "CQF_CVAMessage.h"
#include "internal/qf_interfacedef.h"

#include "Core/IQF_ObjectFactory.h"
#include "CQF_CVAlgorithms.h"
#include "CQF_DSATool.h"
#include "GuideWireMoulding.h"

#include "iqf_main.h"

QF::IQF_Component* QF::QF_CreateComponentObject(QF::IQF_Main* pMain)
{
    QF::IQF_Component* pComponent = new CQF_CVACom(pMain);
    assert(pComponent);
    return pComponent;
}

CQF_CVACom::CQF_CVACom(QF::IQF_Main* pMain) :m_pMain(pMain)
{

}

CQF_CVACom::~CQF_CVACom()
{
    m_pMainCommand->Release();
    m_pMainMessage->Release();
    m_pDSATool->Release();
}

void CQF_CVACom::Release()
{
    delete this;
}

bool CQF_CVACom::Init()
{
    m_pMainCommand = new CQF_CVACommand(m_pMain);
    m_pMainMessage = new CQF_CVAMessage(m_pMain);
    m_pDSATool = new CQF_DSATool;
    IQF_ObjectFactory* pObjectFactory = (IQF_ObjectFactory*)m_pMain->GetInterfacePtr(QF_Core_ObjectFactory);
    if (pObjectFactory)
    {
        pObjectFactory->Register(Object_ID_CVAlgorithms, NEW_INSTANCE(CQF_CVAlgorithms));
        pObjectFactory->Register(Object_ID_GuideWireMoulding, NEW_INSTANCE(GuideWireMoulding));
    }
    return true;
}

int CQF_CVACom::GetInterfaceCount()
{
    return 3;
}

const char* CQF_CVACom::GetInterfaceID(int iID)
{
    switch (iID)
    {
    case 0:
        return QF_INTERFACE_MAIN_COMMAND;
    case 1:
        return QF_INTERFACE_MAIN_MESSAGE;
    case 2:
        return QF_INTERFACE_DSA_TOOL;
    default:
        break;
    }
    return "";
}

void* CQF_CVACom::GetInterfacePtr(const char* szInterfaceID)
{
    if (strcmp(szInterfaceID, QF_INTERFACE_MAIN_COMMAND) == 0)
    {
        return m_pMainCommand;
    }
    else if (strcmp(szInterfaceID, QF_INTERFACE_MAIN_MESSAGE) == 0)
    {
        return m_pMainMessage;
    }
    else if (strcmp(szInterfaceID, QF_INTERFACE_DSA_TOOL) == 0)
    {
        return m_pDSATool;
    }
    else
        return NULL;
}