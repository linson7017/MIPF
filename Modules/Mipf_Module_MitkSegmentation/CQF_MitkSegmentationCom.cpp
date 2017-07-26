#include "CQF_MitkSegmentationCom.h"
#include <string>
#include <assert.h>

#include "CQF_MainCommand.h"
//#include "CQF_MainMessage.h"
#include "CQF_ManualSegmentation.h"
#include "CQF_SurfaceTool.h"

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
    //m_pMainMessage->Release();
    delete m_pSegmentation;
    delete m_pSurfaceTool;
}


void CQF_MitkSegmentationCom::Release()
{
	delete this;
}

bool CQF_MitkSegmentationCom::Init()
{
	m_pMainCommand = new CQF_MainCommand(m_pMain);
    //m_pMainMessage = new CQF_MainMessage(m_pMain);

    m_pSegmentation = new CQF_ManualSegmentation(m_pMain);
    m_pSurfaceTool = new CQF_SurfaceTool();
    m_pMainCommand->SetSegmentationImp(m_pSegmentation);
    //m_pMainMessage->SetSegmentationImp(m_pSegmentation);
	return true;
}


int CQF_MitkSegmentationCom::GetInterfaceCount()
{
	return 3;

}

const char* CQF_MitkSegmentationCom::GetInterfaceID(int iID)
{

	switch (iID)
	{
	case 0:
		return QF_INTERFACE_MAIN_COMMAND;
    case 1:
        return QF_MitkSegmentation_Tool;
    case 2:
        return QF_MitkSurface_Tool;
	default:
		break;
	}
	return "";
}

void* CQF_MitkSegmentationCom::GetInterfacePtr(const char* szInterfaceID)
{
	if (strcmp(szInterfaceID, QF_INTERFACE_MAIN_COMMAND) == 0)
	{
		return m_pMainCommand;
	}
    else if (strcmp(szInterfaceID, QF_MitkSegmentation_Tool) == 0)
    {
        return m_pSegmentation;
    }
    else if (strcmp(szInterfaceID, QF_MitkSurface_Tool) == 0)
    {
        return m_pSurfaceTool;
    }
	else
		return NULL;
}