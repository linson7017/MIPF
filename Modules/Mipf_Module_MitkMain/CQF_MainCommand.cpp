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
#include "mitkDicomSeriesReader.h"
#include "mitkProgressBar.h"
#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateProperty.h"

#include "QmitkIOUtil.h"
#include <QFileDialog>

#include "iqf_main.h"
#include "iqf_properties.h"
#include "iqf_property.h"
#include "qf_log.h"
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

void LoadDicomSeriesCallBack(float value)
{
    mitk::ProgressBar::GetInstance()->Progress(100*value);
}

bool CQF_MainCommand::ExecuteCommand(const char* szCommandID, QF::IQF_Properties* pInParam, QF::IQF_Properties* pOutParam)
{
    if (strcmp(szCommandID, MITK_MAIN_COMMAND_LOAD_DATA) == 0)
    {
        std::string datastorageID = pInParam->GetStringProperty("DataStorage","");
        IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        mitk::DataStorage::Pointer pDataStorage = pMitkDataManager->GetDataStorage(datastorageID);
        if (pDataStorage.IsNull())
        {
            QF_ERROR << "Data storage with id \""<<datastorageID<<"\" does not exist!";
            return false;
        }
        IQF_MitkReference* pMitkReference = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);
        QString defaultOpenFilePath = pMitkReference->GetString("LastOpenDirectory");

        QStringList fileNames = QFileDialog::getOpenFileNames(NULL, "Open",
            defaultOpenFilePath,
            QmitkIOUtil::GetFileOpenFilterString());
        if (fileNames.empty())
            return false;
        try
        {
            if (pInParam->GetBoolProperty("SingleMode", false))
            {
                pDataStorage->Remove(pDataStorage->GetSubset(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))));
            }
            mitk::DataStorage::SetOfObjects::Pointer results = QmitkIOUtil::Load(fileNames, *pDataStorage);
            if (pOutParam)
            {
                pOutParam->SetPtrProperty("LastLoadedDataNode", results->front().GetPointer());
            }
        }
        catch (const mitk::Exception& e)
        {
            QF_ERROR << e;
            return false;
        }
        mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(pDataStorage);
        pMitkReference->SetString("LastOpenDirectory", QFileInfo(fileNames.back()).absolutePath().toStdString().c_str());
        return true;
    }
    else if (strcmp(szCommandID, MITK_MAIN_COMMAND_OPEN_PROJECT) == 0)
    {
        IQF_MitkIO* pMitkIO = (IQF_MitkIO*)m_pMain->GetInterfacePtr(QF_MitkMain_IO);
        pMitkIO->OpenProject();
        return true;
    }
    else if (strcmp(szCommandID, MITK_MAIN_COMMAND_SAVE_PROJECT)==0)
    {
        IQF_MitkIO* pMitkIO = (IQF_MitkIO*)m_pMain->GetInterfacePtr(QF_MitkMain_IO);
        pMitkIO->SaveProject();
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
        if (pInParam->HasProperty("EnableVtkWarning"))
        {
            vtkObject::SetGlobalWarningDisplay(pInParam->GetProperty("EnableVtkWarning")->GetBool());  
        }
        else
        {
            vtkObject::SetGlobalWarningDisplay(vtkObject::GetGlobalWarningDisplay());
        }   
        return true;
    }
    else if (strcmp(szCommandID, MITK_MAIN_COMMAND_CHANGE_CROSSHAIR_GAP_SIZE)==0)
    {
        IQF_MitkRenderWindow* pMitkRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
        if (pMitkRenderWindow)
        {
            QmitkStdMultiWidget* pMultiWidget = pMitkRenderWindow->GetMitkStdMultiWidget(pInParam->GetStringProperty("MultiViewID",""));
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
        return true;
    }
    else if (strcmp(szCommandID, MITK_MAIN_COMMAND_CHANGE_MULTIVIEW_LAYOUT) == 0)
    {
        IQF_MitkRenderWindow* pRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
        if (pRenderWindow)
        {
            QmitkStdMultiWidget* pMultiWidget = pRenderWindow->GetMitkStdMultiWidget(pInParam->GetStringProperty("MultiViewID",""));
            if (pMultiWidget)
            {
                switch (pInParam->GetProperty("LayoutMode")->GetInt())
                {
                case 0:
                    pMultiWidget->changeLayoutToDefault();
                    return true;
                case 1:
                    pMultiWidget->changeLayoutToWidget1();
                    return true;
                case 2:
                    pMultiWidget->changeLayoutToWidget2();
                    return true;
                case 3:
                    pMultiWidget->changeLayoutToWidget3();
                    return true;
                case 4:
                    pMultiWidget->changeLayoutToBig3D();
                    return true;
                default:
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
            return false;
        }
    }
    else if (strcmp(szCommandID, MITK_MAIN_COMMAND_RESET_MULTIVIEW) == 0)
    {
        IQF_MitkRenderWindow* pRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
        if (pRenderWindow)
        {
            QmitkStdMultiWidget* pMultiWidget = pRenderWindow->GetMitkStdMultiWidget(pInParam->GetStringProperty("MultiViewID", ""));
            if (pMultiWidget)
            {
                pMultiWidget->ResetCrosshair();
                return true;
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
    else if (strcmp(szCommandID, MITK_MAIN_COMMAND_ENABLE_ORIENTATION_MARKER) == 0)
    {
        std::string datastorageID = pInParam->GetStringProperty("DataStorage", "");
        IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        mitk::DataStorage::Pointer pDataStorage = pMitkDataManager->GetDataStorage(datastorageID);
        if (pDataStorage.IsNull())
        {
            QF_WARN << "Data storage with id \"" << datastorageID << "\" does not exist!";
            return false;
        }
        mitk::DataNode* markerNode = pDataStorage->GetNamedNode("orientation marker");
        if (markerNode)
        {
            IQF_MitkRenderWindow* pMitkRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
            if (pInParam->HasProperty("RenderWindowID"))
            {
                QmitkRenderWindow* renderWindow = pMitkRenderWindow->GetQmitkRenderWindow(pInParam->GetStringProperty("RenderWindowID",""));
                if (renderWindow)
                {
                    bool visible = false;
                    markerNode->GetVisibility(visible, renderWindow->GetRenderer());
                    markerNode->SetVisibility(pInParam->GetBoolProperty("Visible", visible));
                    mitk::RenderingManager::GetInstance()->RequestUpdate(renderWindow->GetRenderWindow());
                }
            }
            if (pInParam->HasProperty("MultiViewID"))
            {
                QmitkStdMultiWidget* pMultiWidget = pMitkRenderWindow->GetMitkStdMultiWidget(pInParam->GetStringProperty("MultiViewID", ""));
                if (pMultiWidget)
                {
                    bool visible = false;
                    markerNode->GetVisibility(visible, pMultiWidget->GetRenderWindow1()->GetRenderer());
                    visible = pInParam->GetBoolProperty("Visible", visible);
                    markerNode->SetVisibility(visible, pMultiWidget->GetRenderWindow1()->GetRenderer());
                    markerNode->SetVisibility(visible, pMultiWidget->GetRenderWindow2()->GetRenderer());
                    markerNode->SetVisibility(visible, pMultiWidget->GetRenderWindow3()->GetRenderer());
                    markerNode->SetVisibility(visible, pMultiWidget->GetRenderWindow4()->GetRenderer());

                    mitk::RenderingManager::GetInstance()->RequestUpdate(pMultiWidget->GetRenderWindow1()->GetRenderWindow());
                    mitk::RenderingManager::GetInstance()->RequestUpdate(pMultiWidget->GetRenderWindow2()->GetRenderWindow());
                    mitk::RenderingManager::GetInstance()->RequestUpdate(pMultiWidget->GetRenderWindow3()->GetRenderWindow());
                    mitk::RenderingManager::GetInstance()->RequestUpdate(pMultiWidget->GetRenderWindow4()->GetRenderWindow());
                }
            }
            
            return true;
        }
        else
        {
            QF_WARN << "Orientation marker in datastorage " << datastorageID << " does not exist! Please assign the path of Orientation-Marker!";
            return false;
        }
    }
    else if (strcmp(szCommandID, MITK_MAIN_COMMAND_LOAD_DICOMS) == 0)
    {
        std::string datastorageID = pInParam->GetStringProperty("DataStorage", "");
        IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        mitk::DataStorage::Pointer pDataStorage = pMitkDataManager->GetDataStorage(datastorageID);
        if (pDataStorage.IsNull())
        {
            MBI_ERROR << "Data storage with id \"" << datastorageID << "\" does not exist!";
            return false;
        }
        IQF_MitkReference* pMitkReference = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);
        QString defaultOpenFilePath = pMitkReference->GetString("LastOpenDirectory");

        QString fileName = QFileDialog::getOpenFileName(NULL, "Open Dicom",
            defaultOpenFilePath,
            "DICOM (*.*)");
        if (fileName.isEmpty())
            return false;
        try
        {
            QFileInfo fi(fileName);
            if (fi.exists())
            {
                QDir dir = fi.absoluteDir();
                QFileInfoList filenames = dir.entryInfoList(QDir::Files, QDir::Name);
                mitk::DicomSeriesReader::StringContainer  files;
                foreach(QFileInfo file, filenames)
                {
                    files.push_back(file.absoluteFilePath().toStdString());
                }
                mitk::ProgressBar::GetInstance()->AddStepsToDo(100);
                mitk::DataNode::Pointer node = mitk::DicomSeriesReader::LoadDicomSeries(files,true,true,true, LoadDicomSeriesCallBack);
                if (node.IsNotNull())
                {
                    if (pInParam->GetBoolProperty("SingleMode", false))
                    {
                        pDataStorage->Remove(pDataStorage->GetSubset(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))));
                    }
                    pDataStorage->Add(node);
                    if (pOutParam)
                    {
                        pOutParam->SetPtrProperty("LastLoadedDicomNode",node.GetPointer());
                    }
                }
            }    
        }
        catch (const mitk::Exception& e)
        {
            QF_ERROR << e;
            return false;
        }
        mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(pDataStorage);
        pMitkReference->SetString("LastOpenDirectory", QFileInfo(fileName).absolutePath().toStdString().c_str());
        return true;
    }
    else
    {
        return false;
    }
}

int CQF_MainCommand::GetCommandCount()
{
    return 10;
}

const char* CQF_MainCommand::GetCommandID(int iIndex)
{
    switch (iIndex)
    {
    case 0:
        return MITK_MAIN_COMMAND_LOAD_DATA;
    case 1:
        return MITK_MAIN_COMMAND_ENABLE_VTK_WARNING;
    case 2:
        return MITK_MAIN_COMMAND_CHANGE_CROSSHAIR_GAP_SIZE;
    case 3:
        return MITK_MAIN_COMMAND_VOLUME_VISUALIZATION;
    case 4:
        return MITK_MAIN_COMMAND_CHANGE_MULTIVIEW_LAYOUT;
    case 5:
        return MITK_MAIN_COMMAND_RESET_MULTIVIEW;
    case 6:
        return MITK_MAIN_COMMAND_SAVE_PROJECT;
    case 7:
        return MITK_MAIN_COMMAND_OPEN_PROJECT;
    case 8:
        return MITK_MAIN_COMMAND_ENABLE_ORIENTATION_MARKER;
    case 9:
        return MITK_MAIN_COMMAND_LOAD_DICOMS;
    default:
        return "";
        break;
    }
}
