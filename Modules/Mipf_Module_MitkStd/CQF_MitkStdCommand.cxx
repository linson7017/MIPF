#include "CQF_MitkStdCommand.h"
#include <string.h>
#include "iqf_main.h"
#include "PointList.h"
#include <MitkMain/IQF_MitkRenderWindow.h>
#include <MitkMain/IQF_MitkDataManager.h>

CQF_MitkStdCommand::CQF_MitkStdCommand(QF::IQF_Main* pMain)
{
    m_pMain = pMain;
   // m_pPointList = new PointList(pMain);
}

CQF_MitkStdCommand::~CQF_MitkStdCommand()
{
}

void CQF_MitkStdCommand::Release()
{
   delete this;
}

bool CQF_MitkStdCommand::ExecuteCommand(const char* szCommandID, QF::IQF_Properties* pInParam, QF::IQF_Properties* pOutParam)
{
    if (strcmp(szCommandID, "MITK_MAIN_COMMAND_STARTPICKPOINT") == 0)
    {      
        //m_pPointList->StartAddPoint();
        return true;
    }
    else if (strcmp(szCommandID, "MITK_MAIN_COMMAND_ENDPICKPOINIT") == 0)
    {
        //m_pPointList->EndAddPoint();
        return true;
    }
    else if (strcmp(szCommandID, "MITK_MAIN_COMMAND_INITPOINTLIST") == 0)
    {
        /*IQF_MitkRenderWindow* pRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
        IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        
        m_pPointList->Initialize();
        mitk::DataNode::Pointer pointset = mitk::DataNode::New();
        m_pPointList->CreateNewPointSetNode(pointset);
        m_pPointList->SetMultiWidget(pRenderWindow->GetMitkStdMultiWidget());*/
        return true;
    }
    else if (strcmp(szCommandID, "MITK_MAIN_COMMAND_SAVEPOINTS") == 0)
    {
        //m_pPointList->SavePoints();
        return true;
    }
    else
    {
        return false;
    }
}

int CQF_MitkStdCommand::GetCommandCount()
{
    return 4;
}

const char* CQF_MitkStdCommand::GetCommandID(int iIndex)
{
    switch (iIndex)
    {
    case 0:
        return "MITK_MAIN_COMMAND_STARTPICKPOINT";
    case 1:
        return "MITK_MAIN_COMMAND_ENDPICKPOINIT";
    case 2:
        return "MITK_MAIN_COMMAND_INITPOINTLIST";
    case 3:
        return "MITK_MAIN_COMMAND_SAVEPOINTS";
    default:
        return "";
        break;
    }
}

