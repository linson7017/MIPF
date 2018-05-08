#include "CQF_TestCom.h"
#include <string>
#include <assert.h>

#include "CQF_TestCommand.h"
#include "CQF_TestMessage.h"
#include "Test.h"
#include "internal/qf_interfacedef.h"

QF::IQF_Component* QF::QF_CreateComponentObject(QF::IQF_Main* pMain)
{
    QF::IQF_Component* pComponent = new CQF_TestCom(pMain);
    assert(pComponent);
    return pComponent;
}

CQF_TestCom::CQF_TestCom(QF::IQF_Main* pMain) :m_pMain(pMain)
{

}

CQF_TestCom::~CQF_TestCom()
{
    m_pMainCommand->Release();
    m_pMainMessage->Release();
}

void CQF_TestCom::Release()
{
    delete this;
}

bool CQF_TestCom::Init()
{
    m_pMainCommand = new CQF_TestCommand(m_pMain);
    m_pMainMessage = new CQF_TestMessage(m_pMain);
    m_pTest = new Test;
    return true;
}

int CQF_TestCom::GetInterfaceCount()
{
    return 3;
}

const char* CQF_TestCom::GetInterfaceID(int iID)
{
    switch (iID)
    {
    case 0:
        return QF_INTERFACE_MAIN_COMMAND;
    case 1:
        return QF_INTERFACE_MAIN_MESSAGE;
    case 2:
        return QF_INTERFACE_TEST;
    default:
        break;
    }
    return "";
}

void* CQF_TestCom::GetInterfacePtr(const char* szInterfaceID)
{
    if (strcmp(szInterfaceID, QF_INTERFACE_MAIN_COMMAND) == 0)
    {
        return m_pMainCommand;
    }
    else if (strcmp(szInterfaceID, QF_INTERFACE_MAIN_MESSAGE) == 0)
    {
        return m_pMainMessage;
    }
    else if (strcmp(szInterfaceID, QF_INTERFACE_TEST) == 0)
    {
        return m_pTest;
    }
    else
        return NULL;
}