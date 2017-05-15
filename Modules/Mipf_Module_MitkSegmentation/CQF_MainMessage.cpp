#include "CQF_MainMessage.h"

#include "MitkMain/mitk_main_msg.h"
#include <string.h>
#include "iqf_main.h"

#include "IQF_PropertySet.h"

CQF_MainMessage::CQF_MainMessage(QF::IQF_Main* pMain):m_pMain(pMain)
{

}

CQF_MainMessage::~CQF_MainMessage()
{

}

void CQF_MainMessage::Release()
{
    delete this;
}

int CQF_MainMessage::GetMessageCount()
{
    return 1;
}

const char* CQF_MainMessage::GetMessageID(int iIndex)
{
    switch (iIndex)
    {
    case 0:
        return MITK_MESSAGE_SELECTION_CHANGED;
    }

    return 0;
}

void CQF_MainMessage::OnMessage(const char* szMessage, int iValue, void *pValue)
{
    if (strcmp(szMessage, MITK_MESSAGE_SELECTION_CHANGED)==0)
    {
        m_pMain->ExecuteCommand("MITK_COMMAND_SEGMENTATION_SELECTION_CHANGED",(QF::IQF_PropertySet*)pValue,0);
    }
}