#include "CQF_MitkIO.h"

#include <mitkIOUtil.h>
#include "QmitkIOUtil.h"
#include "mitkStandaloneDataStorage.h"
#include "mitkRenderingManager.h"


#include <QFileDialog>

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

bool CQF_MitkIO::Load(const char* filename)
{
    // Load datanode (eg. many image formats, surface formats, etc.)
    IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    if (!pMitkDataManager)
    {
        return false;
    }
    mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(filename, *pMitkDataManager->GetDataStorage());
    if (dataNodes->empty())
    {
        fprintf(stderr, "Could not open file %s \n\n", filename);
        return false;
    }
    mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(dataNodes->at(0)->GetData());
    if ((image.IsNotNull()))
    {
        mitk::TimeGeometry::Pointer geo = pMitkDataManager->GetDataStorage()->ComputeBoundingGeometry3D(pMitkDataManager->GetDataStorage()->GetAll());
        mitk::RenderingManager::GetInstance()->InitializeViews(geo);
        return true;
    }
    else
    {
        fprintf(stderr, "Image file %s is empty! \n\n", filename);
        return false;
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
