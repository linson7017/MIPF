#include "MainWindow.h"

#include <QtWidgets>
#include <QtCore>
#include <QtGui>
#include "MultiViews.h"

#include "Res/R.h"
#include "Common/app_env.h"
#include "Utils/variant.h"

#include "mitkRenderingManager.h"
#include "QmitkDataStorageComboBox.h"
#include "QmitkDataStorageTreeModel.h"
#include "QmitkHistogramWidget.h"

#include "MitkMain/IQF_MitkDataManager.h"
#include "MitkMain/IQF_MitkRenderWindow.h"
#include "MitkMain/IQF_MitkReference.h"

#include "ctkPluginFrameworkLauncher.h"
#include "ctkServiceReference.h"
#include "ctkPluginContext.h"
#include "ctkPluginException.h"

#include <vtkQuaternion.h>
#include <vtkMatrix3x3.h>

#include "QmitkIOUtil.h"
//#include <DataManager.h>

#include "iqf_main.h"

MainWindow::MainWindow(const char* xmlfile)
{
    m_pMain = (QF::IQF_Main*)app_env::getMainPtr();
    m_pMain->Attach(this);
    SetupWidgets(xmlfile);
}


MainWindow::~MainWindow()
{
}

void MainWindow::Update(const char* szMessage, int iValue, void* pValue)
{


    if (strcmp(szMessage, "main.ChangeView") == 0)
    {
        VarientMap vmp = *(VarientMap*)pValue;
        variant v = variant::GetVariant(vmp, "parameterIndex");
        int index = v.getInt();
       // m_pMultiViews->ChangeLayout(index);
    }
    else if (strcmp(szMessage, "main.ResetView") == 0)
    {
       // m_pMultiViews->ResetView();
    }
    else if (strcmp(szMessage, "main.LoadData") == 0)
    {
        VarientMap vmp = *(VarientMap*)pValue;
        variant v = variant::GetVariant(vmp, "currentText");
        QString name = v.getString();
        if (name.contains("dicoms", Qt::CaseInsensitive))
        {
            OpenDicom();
        }
        else if (name.contains("meta", Qt::CaseInsensitive))
        {
            OpenMetaImage();
        }
    }
}

void MainWindow::OpenDicom()
{
    IQF_MitkReference* pMitkReference = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);
    IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    IQF_MitkRenderWindow* pMitkRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
    QString defaultOpenFilePath = pMitkReference->GetStringConfig("LastOpenDirectory");

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
    pMitkReference->SetStringConfig("LastOpenDirectory", defaultOpenFilePath.toStdString().c_str());
}

void MainWindow::OpenMetaImage()
{
    IQF_MitkReference* pMitkReference = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);
    QString defaultOpenFilePath = pMitkReference->GetStringConfig("LastOpenFilePath");
    QString filename = QFileDialog::getOpenFileName(this, "Select one or more files to open",
        defaultOpenFilePath,
        "Images (*.mha *.mhd)");
    if (filename.isEmpty())
    {
        return;
    }
    m_pMitkDataManager->Load(filename.toLocal8Bit().constData());
    pMitkReference->SetStringConfig("LastOpenFilePath", filename.toStdString().c_str());
}

void MainWindow::SetupWidgets(const char* xmlfile)
{
    m_pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    if (m_pMitkDataManager)
    {
        m_pMitkDataManager->Init();
    }
  //  ctkPluginFrameworkLauncher::addSearchPath("E:/codes/MIPF-build/bin/plugins/Release",true);
 //   QStringList symbolicNames = ctkPluginFrameworkLauncher::getPluginSymbolicNames("E:/codes/MIPF-build/bin/plugins/Release");
    
    //bool SUCCEEDED = false;
    //try
    //{
    //  
    //    ctkPluginFrameworkLauncher::start("org.mitk.core.ext");
    //    
    //}
    //catch (ctkPluginException &e)
    //{
    //    //std::cout << "Error in " << pluginSybolName << " " << e.message().toStdString() << std::endl;
    //    const ctkException* e2 = e.cause();

    //    if (e2)
    //        std::cout << e2->message().toStdString() << std::endl;
    //   // return LOAD_FAILED;
    //}

    //catch (ctkRuntimeException &e)
    //{
    //   // std::cout << "Error in " << pluginSybolName << " " << e.what() << std::endl;
    //    const ctkException* e2 = e.cause();

    //    if (e2)
    //        std::cout << e2->message().toStdString() << std::endl;
    //   // return LOAD_FAILED;
    //}
    //catch (...)
    //{
    //   // std::cout << "Error in " << pluginSybolName << std::endl;
    //   // return UNKNOW_EXCEPTION;
    //}
    //ctkServiceReference reference = ctkPluginFrameworkLauncher::getPluginContext()->getServiceReference("org.mitk.views.datamanager");
    //if (reference)
    //{
    //    QWidget* datamanagerWidget = qobject_cast<QWidget*>( ctkPluginFrameworkLauncher::getPluginContext()->getService(reference));
    //    R::Instance()->registerCustomWidget("DataManager", datamanagerWidget);
    //}
    setContentView(xmlfile);

}