#include "CQF_TestMessage.h"
#include "MitkMain/mitk_main_msg.h"
#include <string.h>
#include "iqf_main.h"

CQF_TestMessage::CQF_TestMessage(QF::IQF_Main* pMain) :m_pMain(pMain)
{

}

CQF_TestMessage::~CQF_TestMessage()
{

}

void CQF_TestMessage::Release()
{
    delete this;
}

int CQF_TestMessage::GetMessageCount()
{
    return 1;
}

const char* CQF_TestMessage::GetMessageID(int iIndex)
{
    switch (iIndex)
    {
    case 0:
        return "";
    }
    return "";
}

void CQF_TestMessage::OnMessage(const char* szMessage, int iValue, void *pValue)
{
    if (strcmp(szMessage, "") == 0)
    {
    }
}