#include "CQF_CVAMessage.h"
#include "MitkMain/mitk_main_msg.h"
#include <string.h>
#include "iqf_main.h"

CQF_CVAMessage::CQF_CVAMessage(QF::IQF_Main* pMain) :m_pMain(pMain)
{

}

CQF_CVAMessage::~CQF_CVAMessage()
{

}

void CQF_CVAMessage::Release()
{
    delete this;
}

int CQF_CVAMessage::GetMessageCount()
{
    return 0;
}

const char* CQF_CVAMessage::GetMessageID(int iIndex)
{
    switch (iIndex)
    {
    case 0:
        return "";
    }
    return "";
}

void CQF_CVAMessage::OnMessage(const char* szMessage, int iValue, void *pValue)
{
    if (strcmp(szMessage, "") == 0)
    {
    }
}