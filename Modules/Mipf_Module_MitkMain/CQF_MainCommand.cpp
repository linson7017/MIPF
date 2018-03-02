#include "CQF_MainCommand.h"
#include "MitkMain/mitk_command_def.h"
#include <string.h>

//mitk
#include "MitkMain/IQF_MitkDataManager.h"
#include "MitkMain/IQF_MitkRenderWindow.h"
#include "MitkMain/IQF_MitkReference.h"
#include "MitkMain/IQF_MitkIO.h"

#include "mitkRenderingManager.h"
#include "QmitkStdMultiWidget.h"
#include "mitkDataNode.h"

#include "QmitkIOUtil.h"
#include <QFileDialog>

#include "iqf_main.h"
#include "iqf_properties.h"
#include "iqf_property.h"

//qt
#include <QMessageBox>

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

bool CQF_MainCommand::ExecuteCommand(const char* szCommandID, QF::IQF_Properties* pInParam, QF::IQF_Properties* pOutParam)
{
    if (strcmp(szCommandID, MITK_MAIN_COMMAND_LOADDATA) == 0)
    {
        IQF_MitkReference* pMitkReference = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);
        IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        QString defaultOpenFilePath = pMitkReference->GetString("LastOpenDirectory");

        QStringList fileNames = QFileDialog::getOpenFileNames(NULL, "Open",
            defaultOpenFilePath,
            QmitkIOUtil::GetFileOpenFilterString());
        if (fileNames.empty())
            return false;
        try
        {
            QmitkIOUtil::Load(fileNames, *pMitkDataManager->GetDataStorage());           
        }
        catch (const mitk::Exception& e)
        {
            MITK_INFO << e;
            return false;
        }
        mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(pMitkDataManager->GetDataStorage());
        pMitkReference->SetString("LastOpenDirectory", QFileInfo(fileNames.back()).absolutePath().toStdString().c_str());
        return true;
    }
    else if (strcmp(szCommandID, MITK_MAIN_COMMAND_VOLUME_VISUALIZATION) == 0)
    {
        IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        if (pMitkDataManager)
        {
            if (pMitkDataManager->GetCurrentNode())
            {
                bool volumeRenderingEnabled = true;
                pMitkDataManager->GetCurrentNode()->GetBoolProperty("volumerendering", volumeRenderingEnabled);
                pMitkDataManager->GetCurrentNode()->SetBoolProperty("volumerendering", !volumeRenderingEnabled);
                mitk::RenderingManager::GetInstance()->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
            }
        }
    }
    else if (strcmp(szCommandID, MITK_MAIN_COMMAND_ENABLE_VTK_WARNING) == 0)
    {
        if (pInParam)
        {
            vtkObject::SetGlobalWarningDisplay(pInParam->GetProperty("EnableVtkWarning")->GetBool());
        }
        else
        {
            MITK_ERROR << "Command does not pass property EnableVtkWarning!";
        }
        return true;
    }
    else if (strcmp(szCommandID, MITK_MAIN_COMMAND_CHANGE_CROSSHAIR_GAP_SIZE)==0)
    {
        MITK_INFO << "MITK_MAIN_COMMAND_CHANGE_CROSSHAIR_GAP_SIZE";
        if (pInParam)
        {
            IQF_MitkRenderWindow* pMitkRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
            if (pMitkRenderWindow)
            {
                QmitkStdMultiWidget* pMultiWidget = pMitkRenderWindow->GetMitkStdMultiWidget();
                if (pMultiWidget)
                {
                    pMultiWidget->GetWidgetPlane1()->SetIntProperty("Crosshair.Gap Size", pInParam->GetProperty("CrosshairGapSize")->GetInt());
                    pMultiWidget->GetWidgetPlane2()->SetIntProperty("Crosshair.Gap Size", pInParam->GetProperty("CrosshairGapSize")->GetInt());
                    pMultiWidget->GetWidgetPlane3()->SetIntProperty("Crosshair.Gap Size", pInParam->GetProperty("CrosshairGapSize")->GetInt());
                    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            MITK_ERROR << "Command does not pass property CrosshairGapSize!";
        }
        return true;
    }
    else
    {
        return false;
    }
}

int CQF_MainCommand::GetCommandCount()
{
    return 4;
}

const char* CQF_MainCommand::GetCommandID(int iIndex)
{
    switch (iIndex)
    {
    case 0:
        return MITK_MAIN_COMMAND_LOADDATA;
    case 1:
        return MITK_MAIN_COMMAND_ENABLE_VTK_WARNING;
    case 2:
        return MITK_MAIN_COMMAND_CHANGE_CROSSHAIR_GAP_SIZE;
    case 3:
        return MITK_MAIN_COMMAND_VOLUME_VISUALIZATION;
    default:
        return "";
        break;
    }
}
