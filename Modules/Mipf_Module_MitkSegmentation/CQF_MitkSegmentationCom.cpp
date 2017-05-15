#include "CQF_MitkSegmentationCom.h"
#include <string>
#include <assert.h>

#include "CQF_MainCommand.h"
#include "CQF_MainMessage.h"
#include "internal/qf_interfacedef.h"

QF::IQF_Component* QF::QF_CreateComponentObject(QF::IQF_Main* pMain)
{
	QF::IQF_Component* pComponent = new CQF_MitkSegmentationCom(pMain);
	assert(pComponent);
	return pComponent;
}

CQF_MitkSegmentationCom::CQF_MitkSegmentationCom(QF::IQF_Main* pMain) :m_pMain(pMain)
{
    
}


CQF_MitkSegmentationCom::~CQF_MitkSegmentationCom()
{
    m_pMainCommand->Release();
    m_pMainMessage->Release();
}


void CQF_MitkSegmentationCom::Release()
{
	delete this;
}

bool CQF_MitkSegmentationCom::Init()
{
	m_pMainCommand = new CQF_MainCommand(m_pMain);
    m_pMainMessage = new CQF_MainMessage(m_pMain);
	return true;
}


int CQF_MitkSegmentationCom::GetInterfaceCount()
{
	return 2;

}

const char* CQF_MitkSegmentationCom::GetInterfaceID(int iID)
{

	switch (iID)
	{
	case 0:
		return QF_INTERFACCE_MAIN_COMMAND;
    case 1:
        return QF_INTERFACCE_MAIN_MESSAGE;
	default:
		break;
	}
	return "";
}

void* CQF_MitkSegmentationCom::GetInterfacePtr(const char* szInterfaceID)
{
	if (strcmp(szInterfaceID, QF_INTERFACCE_MAIN_COMMAND) == 0)
	{
		return m_pMainCommand;
	}
    else if (strcmp(szInterfaceID, QF_INTERFACCE_MAIN_MESSAGE) == 0)
    {
        return m_pMainMessage;
    }
	else
		return NULL;
}