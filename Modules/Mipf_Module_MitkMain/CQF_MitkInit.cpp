#include "CQF_MitkInit.h"

#include "MitkMain/IQF_MitkDataManager.h"
#include "MitkMain/IQF_MitkRenderWindow.h"

#include "iqf_main.h"

//mitk
#include "QmitkRegisterClasses.h"
#include "mitkStandaloneDataStorage.h"

//qt
#include <QSettings>


CQF_MitkInit::CQF_MitkInit(QF::IQF_Main* pMain) :m_pMain(pMain)
{
}


CQF_MitkInit::~CQF_MitkInit()
{
}

void CQF_MitkInit::Init(mitk::DataStorage* dataStorager)
{
    vtkObject::GlobalWarningDisplayOff();
    QmitkRegisterClasses();
    IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    if (pDataManager)
    {
        if (dataStorager)
        {
            pDataManager->SetDataStorage(dataStorager);
        }
        pDataManager->Init();

        QSettings set(QString(m_pMain->GetConfigPath()).append("/mitk/config.ini"), QSettings::IniFormat);
        set.beginGroup("DataStorage");
        foreach(QString str,set.childKeys())
        {
            if (set.value(str,false).toBool())
            {
                mitk::DataStorage::Pointer newDataStorage = mitk::StandaloneDataStorage::New();
                pDataManager->SetDataStorage(newDataStorage, str.toLocal8Bit().constData());
            }
        }
        set.endGroup();
    }
}

void CQF_MitkInit::SetStdMultiWidget(QmitkStdMultiWidget* stdMultiWidget)
{
    IQF_MitkRenderWindow* pRenderWindowManager = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
    if (pRenderWindowManager&&stdMultiWidget)
    {
        pRenderWindowManager->SetMitkStdMultiWidget(stdMultiWidget);
    }
}
