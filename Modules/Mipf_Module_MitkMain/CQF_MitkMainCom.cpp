#include "CQF_MitkMainCom.h"
#include <string>
#include <assert.h>
#include "CQF_MitkDataManager.h"
#include "CQF_MitkRenderWindow.h"
#include "CQF_MitkReference.h"
#include "CQF_MitkIO.h"
#include "CQF_MitkDisplayOption.h"
#include "CQF_MitkInit.h"

#include "CQF_MainCommand.h"
#include "internal/qf_interfacedef.h"

//mitk
#include "QmitkRegisterClasses.h"

#include "vtkObject.h"

QF::IQF_Component* QF::QF_CreateComponentObject(QF::IQF_Main* pMain)
{
    QF::IQF_Component* pComponent = new CQF_MitkMain(pMain);
    assert(pComponent);
    return pComponent;
}

CQF_MitkMain::CQF_MitkMain(QF::IQF_Main* pMain):m_pMain(pMain)
{

}


CQF_MitkMain::~CQF_MitkMain()
{
    delete m_pMitkDataManager;
    delete m_pMitkRenderWindow;
    delete m_pMitkReference;
    delete m_pMitkIO;
    delete m_pDisplayOption;
    delete m_pMitkInit;
    delete m_pMainCommand;
}


void CQF_MitkMain::Release()
{
    delete this;
}

bool CQF_MitkMain::Init()
{
   m_pMitkDataManager = new CQF_MitkDataManager(m_pMain);
    m_pMitkRenderWindow = new CQF_MitkRenderWindow;
    m_pMitkReference = new CQF_MitkReference(m_pMain);
    m_pDisplayOption = new CQF_MitkDisplayOption(m_pMain);
    m_pMitkInit = new CQF_MitkInit(m_pMain);
    m_pMitkIO = new CQF_MitkIO(m_pMain);

	m_pMainCommand = new CQF_MainCommand(m_pMain);
    return true;
}


int CQF_MitkMain::GetInterfaceCount()
{
    return 7;

}

const char* CQF_MitkMain::GetInterfaceID(int iID)
{
    switch (iID)
    {
    case 0:
        return QF_INTERFACE_MAIN_COMMAND;
    case 1:
        return QF_MitkMain_RenderWindow;
    case 2:
        return QF_MitkMain_Reference;
	case 3:
		return QF_MitkMain_DataManager;
    case 4:
        return QF_MitkMain_IO;
    case 5:
        return QF_MitkMain_DisplayOption;
    case 6:
        return QF_MitkMain_Init;
    default:
        break;
    }
    return "";
}

void* CQF_MitkMain::GetInterfacePtr(const char* szInterfaceID)
{
    if (strcmp(szInterfaceID, QF_INTERFACE_MAIN_COMMAND) == 0)
    {
        return m_pMainCommand;
    }
    else if (strcmp(szInterfaceID, QF_MitkMain_RenderWindow) == 0)
    {
        return m_pMitkRenderWindow;
    }
    else if (strcmp(szInterfaceID, QF_MitkMain_Reference) == 0)
    {
        return m_pMitkReference;
    }
	else if (strcmp(szInterfaceID, QF_MitkMain_DataManager) == 0)
	{
		return m_pMitkDataManager;
	}
    else if (strcmp(szInterfaceID, QF_MitkMain_IO) == 0)
    {
        return m_pMitkIO;
    }
    else if (strcmp(szInterfaceID, QF_MitkMain_DisplayOption) == 0)
    {
        return m_pDisplayOption;
    }
    else if (strcmp(szInterfaceID, QF_MitkMain_Init) == 0)
    {
        return m_pMitkInit;
    }
    else
        return NULL;
}