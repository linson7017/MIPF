#include "CQF_TestCommand.h"
#include <string.h>
#include "iqf_main.h"

CQF_TestCommand::CQF_TestCommand(QF::IQF_Main* pMain)
{
    m_pMain = pMain;
}

CQF_TestCommand::~CQF_TestCommand()
{
}

void CQF_TestCommand::Release()
{
   delete this;
}

bool CQF_TestCommand::ExecuteCommand(const char* szCommandID, QF::IQF_PropertySet* pInParam, QF::IQF_PropertySet* pOutParam)
{
    if (strcmp(szCommandID, "") == 0)
    {      
        return true;
    }
    else
    {
        return false;
    }
}

int CQF_TestCommand::GetCommandCount()
{
    return 1;
}

const char* CQF_TestCommand::GetCommandID(int iIndex)
{
    switch (iIndex)
    {
    case 0:
        return "";
    default:
        return "";
        break;
    }
}

