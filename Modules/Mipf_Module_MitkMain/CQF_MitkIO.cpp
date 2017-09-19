#include "CQF_MitkIO.h"

#include <mitkIOUtil.h>
#include "QmitkIOUtil.h"
#include "mitkStandaloneDataStorage.h"
#include "mitkRenderingManager.h"
#include "mitkSceneIO.h"
#include "mitkProgressBar.h"
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>


#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>

#include "MitkMain/IQF_MitkRenderWindow.h"
#include "MitkMain/IQF_MitkReference.h"
#include "MitkMain/IQF_MitkDataManager.h"

#include "iqf_main.h"

CQF_MitkIO::CQF_MitkIO(QF::IQF_Main* pMain):m_pMain(pMain)
{
}


CQF_MitkIO::~CQF_MitkIO()
{
}

mitk::DataNode* CQF_MitkIO::Load(const char* filename)
{
    // Load datanode (eg. many image formats, surface formats, etc.)
    IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    if (!pMitkDataManager)
    {
        return nullptr;
    }
    mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(filename, *pMitkDataManager->GetDataStorage());
    if (dataNodes->empty())
    {
        fprintf(stderr, "Could not open file %s \n\n", filename);
        return nullptr;
    }
    if (dataNodes->at(0)->GetData())
    {
        mitk::TimeGeometry::Pointer geo = pMitkDataManager->GetDataStorage()->ComputeBoundingGeometry3D(pMitkDataManager->GetDataStorage()->GetAll());
        mitk::RenderingManager::GetInstance()->InitializeViews(geo);
        return dataNodes->at(0);
    }
    else
    {
        fprintf(stderr, "File %s is empty! \n\n", filename);
        return nullptr;
    }
}

void CQF_MitkIO::LoadFiles()
{
    IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    IQF_MitkRenderWindow* pMitkRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
    if (!pMitkDataManager || !pMitkRenderWindow)
    {
        return;
    }
    IQF_MitkReference* pMitkReference = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);
    QString defaultOpenFilePath = pMitkReference->GetString("LastOpenDirectory");

    QStringList fileNames = QFileDialog::getOpenFileNames(NULL, "Open",
        defaultOpenFilePath,
        QmitkIOUtil::GetFileOpenFilterString());
    if (fileNames.empty())
        return;
    try
    {
        QmitkIOUtil::Load(fileNames, *pMitkDataManager->GetDataStorage());
    }
    catch (const mitk::Exception& e)
    {
        MITK_INFO << e;
        return;
    }
    mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(pMitkDataManager->GetDataStorage());
    pMitkReference->SetString("LastOpenDirectory", QFileInfo(fileNames.back()).absolutePath().toStdString().c_str());
}

void CQF_MitkIO::SaveProject()
{
    try
    {
        /**
        * @brief stores the last path of last saved file
        */
        IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        if (!pMitkDataManager)
        {
            return;
        }
        static QString m_LastPath;
        {
            if (pMitkDataManager->GetDataStorage().IsNull())
            {
                QString msg = "IDataStorageService service not available. Unable to open files.";
                MITK_WARN << msg.toStdString();
                QMessageBox::warning(QApplication::activeWindow(), "Unable to open files", msg);
                return;
            }
        }

        mitk::DataStorage::Pointer storage = pMitkDataManager->GetDataStorage();

        QString dialogTitle = "Save MITK Scene (%1)";
        QString fileName = QFileDialog::getSaveFileName(NULL,
            dialogTitle.arg(""),
            m_LastPath,
            "MITK scene files (*.mitk)",
            NULL);

        if (fileName.isEmpty())
            return;

        // remember the location
        m_LastPath = fileName;

        if (fileName.right(5) != ".mitk")
            fileName += ".mitk";

        mitk::SceneIO::Pointer sceneIO = mitk::SceneIO::New();

        mitk::ProgressBar::GetInstance()->AddStepsToDo(2);

        /* Build list of nodes that should be saved */
        mitk::NodePredicateNot::Pointer isNotHelperObject =
            mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)));
        mitk::DataStorage::SetOfObjects::ConstPointer nodesToBeSaved = storage->GetSubset(isNotHelperObject);

        if (!sceneIO->SaveScene(nodesToBeSaved, storage, fileName.toStdString()))
        {
            QMessageBox::information(NULL,
                "Scene saving",
                "Scene could not be written completely. Please check the log.",
                QMessageBox::Ok);

        }
        mitk::ProgressBar::GetInstance()->Progress(2);

        mitk::SceneIO::FailedBaseDataListType::ConstPointer failedNodes = sceneIO->GetFailedNodes();
        if (!failedNodes->empty())
        {
            std::stringstream ss;
            ss << "The following nodes could not be serialized:" << std::endl;
            for (mitk::SceneIO::FailedBaseDataListType::const_iterator iter = failedNodes->begin();
                iter != failedNodes->end();
                ++iter)
            {
                ss << " - ";
                if (mitk::BaseData* data = (*iter)->GetData())
                {
                    ss << data->GetNameOfClass();
                }
                else
                {
                    ss << "(NULL)";
                }

                ss << " contained in node '" << (*iter)->GetName() << "'" << std::endl;
            }

            MITK_WARN << ss.str();
        }

        mitk::PropertyList::ConstPointer failedProperties = sceneIO->GetFailedProperties();
        if (!failedProperties->GetMap()->empty())
        {
            std::stringstream ss;
            ss << "The following properties could not be serialized:" << std::endl;
            const mitk::PropertyList::PropertyMap* propmap = failedProperties->GetMap();
            for (mitk::PropertyList::PropertyMap::const_iterator iter = propmap->begin();
                iter != propmap->end();
                ++iter)
            {
                ss << " - " << iter->second->GetNameOfClass() << " associated to key '" << iter->first << "'" << std::endl;
            }

            MITK_WARN << ss.str();
        }
    }
    catch (std::exception& e)
    {
        MITK_ERROR << "Exception caught during scene saving: " << e.what();
    }
}

void CQF_MitkIO::Save(const mitk::BaseData *data)
{
    /*  try
      {
          IQF_MitkReference* pReference = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);

          QStringList fileNames = QmitkIOUtil::Save(data, szName, pReference ? pReference->GetString("LastFileSavePath") : "",
              this);
          if (!fileNames.empty() && pReference)
          {
              pReference->SetString("LastFileSavePath", QFileInfo(fileNames.back()).absolutePath().toStdString().c_str());
          }
      }
      catch (const mitk::Exception& e)
      {
          MITK_INFO << e;
          return;
      }*/
}

void CQF_MitkIO::Save(const std::vector<const mitk::BaseData *> &data)
{

}
