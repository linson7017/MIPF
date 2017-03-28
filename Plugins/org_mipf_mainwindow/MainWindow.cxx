#include "MainWindow.h"

#include <QtWidgets>
#include <QtCore>
#include <QtGui>
#include "MultiViews.h"

#include "Res/R.h"
#include "Common/app_env.h"
#include "Utils/variant.h"

#include "iqf_main.h"
#include "QmitkDataStorageComboBox.h"
#include "QmitkDataStorageTreeModel.h"
#include "QmitkHistogramWidget.h"

#include "MitkMain/IQF_MitkDataStorage.h"


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
        m_pMultiViews->ChangeLayout(index);
    }
    else if (strcmp(szMessage, "main.ResetView") == 0)
    {
        m_pMultiViews->ResetView();
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
    QString fileDirectory = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
        "C:/",
        QFileDialog::ShowDirsOnly
        | QFileDialog::DontResolveSymlinks);
    if (fileDirectory.isEmpty())
    {
        return;
    }
    m_pMitkDataStorage->Load(fileDirectory.toLocal8Bit().constData());
}

void MainWindow::OpenMetaImage()
{
    QString filename = QFileDialog::getOpenFileName(this, "Select one or more files to open",
        "D:/",
        "Images (*.mha *.mhd)");
    if (filename.isEmpty())
    {
        return;
    }
    m_pMitkDataStorage->Load(filename.toLocal8Bit().constData());
}

void MainWindow::SetupWidgets(const char* xmlfile)
{
    m_pMultiViews = new MultiViews;
    R::Instance()->registerCustomWidget("VtkMultiView", m_pMultiViews);
    QmitkHistogramWidget* levelWindow = new QmitkHistogramWidget(this);
    R::Instance()->registerCustomWidget("LevelWindow", levelWindow);


    //init the data manager
    //Init the dataStorage
    m_pMitkDataStorage = (IQF_MitkDataStorage*)m_pMain->GetInterfacePtr("QF_MitkMain_DataStorage");
    if (m_pMitkDataStorage)
    {
        m_pMitkDataStorage->Init();
        QTreeView* treeWidget = new QTreeView;
        QmitkDataStorageTreeModel* dataStorageModel = new QmitkDataStorageTreeModel(m_pMitkDataStorage->GetDataStorage());

        m_pMultiViews->SetDataStorage(m_pMitkDataStorage->GetDataStorage());
        m_pMultiViews->Initialize();
        treeWidget->setModel(dataStorageModel);
        
        R::Instance()->registerCustomWidget("DataManager", treeWidget);

      //  levelWindow->SetHistogram(m_pMitkDataStorage->GetDataStorage());
    }

    setContentView(xmlfile);

}