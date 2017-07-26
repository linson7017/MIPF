#include "CQF_MitkStdCom.h"
#include <string>
#include <assert.h>

#include "CQF_MitkStdCommand.h"
#include "CQF_MitkStdMessage.h"
#include "PointList.h"
#include "internal/qf_interfacedef.h"

QF::IQF_Component* QF::QF_CreateComponentObject(QF::IQF_Main* pMain)
{
    QF::IQF_Component* pComponent = new CQF_MitkStdCom(pMain);
    assert(pComponent);
    return pComponent;
}

CQF_MitkStdCom::CQF_MitkStdCom(QF::IQF_Main* pMain) :m_pMain(pMain)
{

}

CQF_MitkStdCom::~CQF_MitkStdCom()
{
    m_pMainCommand->Release();
    delete m_pPointListFactory;
}

void CQF_MitkStdCom::Release()
{
    delete this;
}

bool CQF_MitkStdCom::Init()
{
    m_pMainCommand = new CQF_MitkStdCommand(m_pMain);
    m_pPointListFactory = new PointListFactory(m_pMain);
    return true;
}

int CQF_MitkStdCom::GetInterfaceCount()
{
    return 2;
}

const char* CQF_MitkStdCom::GetInterfaceID(int iID)
{
    switch (iID)
    {
    case 0:
        return QF_INTERFACE_MAIN_COMMAND;
    case 1:
        return QF_MitkStd_PointListFactory;
    default:
        break;
    }
    return "";
}

void* CQF_MitkStdCom::GetInterfacePtr(const char* szInterfaceID)
{
    if (strcmp(szInterfaceID, QF_INTERFACE_MAIN_COMMAND) == 0)
    {
        return m_pMainCommand;
    }
    else if (strcmp(szInterfaceID, QF_MitkStd_PointListFactory) == 0)
    {
        return m_pPointListFactory;
    }
    else
        return NULL;
}