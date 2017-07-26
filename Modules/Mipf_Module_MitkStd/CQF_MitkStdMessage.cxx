#include "CQF_MitkStdMessage.h"
#include "MitkMain/mitk_main_msg.h"
#include <string.h>
#include "iqf_main.h"

CQF_MitkStdMessage::CQF_MitkStdMessage(QF::IQF_Main* pMain) :m_pMain(pMain)
{

}

CQF_MitkStdMessage::~CQF_MitkStdMessage()
{

}

void CQF_MitkStdMessage::Release()
{
    delete this;
}

int CQF_MitkStdMessage::GetMessageCount()
{
    return 1;
}

const char* CQF_MitkStdMessage::GetMessageID(int iIndex)
{
    switch (iIndex)
    {
    case 0:
        return "";
    }
    return "";
}

void CQF_MitkStdMessage::OnMessage(const char* szMessage, int iValue, void *pValue)
{
    if (strcmp(szMessage, "") == 0)
    {
    }
}