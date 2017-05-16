#include "CQF_MainCommand.h"
#include "MitkMain/mitk_command_def.h"
#include <string.h>

//mitk
#include "MitkMain/IQF_MitkDataManager.h"
#include "MitkMain/IQF_MitkRenderWindow.h"
#include "MitkMain/IQF_MitkReference.h"

#include "mitkRenderingManager.h"
#include "mitkToolManager.h"
#include <mitkToolManagerProvider.h>
#include "mitkApplicationCursor.h"
#include "mitkSegmentationObjectFactory.h"
#include "mitkCameraController.h"
#include "mitkLabelSetImage.h"

#include "QmitkIOUtil.h"
#include <QFileDialog>

#include "iqf_main.h"

//qt
#include <QMessageBox>

#include "QmitkNewSegmentationDialog.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkSegmentationOrganNamesHandling.cpp"

#include "usModuleResource.h"
#include "usModuleResourceStream.h"

#include "CMitkSegmentation.h"

CQF_MainCommand::CQF_MainCommand(QF::IQF_Main* pMain)
{
    m_pMain = pMain;
}


CQF_MainCommand::~CQF_MainCommand()
{
}

void CQF_MainCommand::Release()
{
    delete this;
}

int CQF_MainCommand::GetCommandCount()
{
    return 3;
}

const char* CQF_MainCommand::GetCommandID(int iIndex)
{
    switch (iIndex)
    {
    case 0:
        return "MITK_MAIN_COMMAND_TOOLADD";
    case 1:
        return "MITK_MAIN_COMMAND_CREATE_NEW_SEGMENTATION";
    case 2:
        return "MITK_MAIN_COMMAND_INIT_SEGMENTATION";
    default:
        return "";
        break;
    }
}

bool CQF_MainCommand::ExecuteCommand(const char* szCommandID, QF::IQF_PropertySet* pInParam, QF::IQF_PropertySet* pOutParam)
{
    if (strcmp(szCommandID, "MITK_MAIN_COMMAND_TOOLADD") == 0)
    {
        IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        m_pSegmentation->SetReferenceNode(pMitkDataManager->GetCurrentNode());
        m_pSegmentation->SetWorkingNode(m_workingNode);
        m_pSegmentation->SelectTool(1);
        return true;
    }
    else if (strcmp(szCommandID, "MITK_MAIN_COMMAND_CREATE_NEW_SEGMENTATION") == 0)
    {
        m_workingNode = m_pSegmentation->CreateSegmentationNode();
        if (m_workingNode)
        {
            return true;
        }
        else
        {
            return false;
        }      
    }
    else if (strcmp(szCommandID, "MITK_MAIN_COMMAND_INIT_SEGMENTATION") == 0)
    {
        m_pSegmentation->Init();
    }
    else
    {
        return false;
    }
}
