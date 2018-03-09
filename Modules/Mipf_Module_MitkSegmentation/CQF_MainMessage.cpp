#include "CQF_MainMessage.h"

#include "MitkMain/mitk_main_msg.h"
#include "MitkMain/IQF_MitkDataManager.h"
#include <string.h>
#include "iqf_main.h"


#include "iqf_properties.h"

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
    return 3;
}

const char* CQF_MainMessage::GetMessageID(int iIndex)
{
    switch (iIndex)
    {
    case 0:
        return MITK_MESSAGE_NODE_SELECTION_CHANGED;
    case 1:
        return MITK_MESSAGE_NODE_ADDED;
    case 2:
        return MITK_MESSAGE_NODE_REMOVED;
    }

    return 0;
}

void CQF_MainMessage::OnMessage(const char* szMessage, int iValue, void *pValue)
{
    if (strcmp(szMessage, MITK_MESSAGE_NODE_SELECTION_CHANGED)==0)
    {
        IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)pValue;
    }
    else if (strcmp(szMessage, MITK_MESSAGE_NODE_ADDED) == 0)
    {
        //do what you want for the message
        mitk::DataNode* node = (mitk::DataNode*)pValue;
    }
    else if (strcmp(szMessage, MITK_MESSAGE_NODE_REMOVED) == 0)
    {
        //do what you want for the message
        mitk::DataNode* node = (mitk::DataNode*)pValue;
    }
}