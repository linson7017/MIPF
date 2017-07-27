#include "MainWindow.h"

#include <QtWidgets>
#include <QtCore>
#include <QtGui>

#include "Res/R.h"
#include "Common/app_env.h"
#include "Utils/variant.h"

#include "MitkMain/IQF_MitkIO.h"
#include "MitkMain/IQF_MitkReference.h"



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
    IQF_MitkIO* pMitkIO = (IQF_MitkIO*)m_pMain->GetInterfacePtr(QF_MitkMain_IO);
    pMitkIO->LoadFiles();
}

void MainWindow::OpenMetaImage()
{
    IQF_MitkIO* pMitkIO = (IQF_MitkIO*)m_pMain->GetInterfacePtr(QF_MitkMain_IO);
    IQF_MitkReference* pMitkReference = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);
    QString defaultOpenFilePath = pMitkReference->GetString("LastOpenFilePath");
    QString filename = QFileDialog::getOpenFileName(this, "Select one or more files to open",
        defaultOpenFilePath,
        "Images (*.mha *.mhd)");
    if (filename.isEmpty())
    {
        return;
    }
    pMitkIO->Load(filename.toLocal8Bit().constData());
    pMitkReference->SetString("LastOpenFilePath", QFileInfo(filename).absolutePath().toStdString().c_str());
}

void MainWindow::SetupWidgets(const char* xmlfile)
{
    setContentView(xmlfile);
    R::Instance()->Constructed();
}