#include "widget.h"

#include <QtWidgets>
#include <QApplication>

#include "Utils/QObjectFactory.h"
#include "UIs/QF_Plugin.h"
#include "MitkMain/IQF_MitkDataManager.h"
#include "MitkMain/IQF_MitkIO.h"
#include "MitkMain/IQF_MitkInit.h"
#include "iqf_main.h"

//mitk
#include "mitkStandaloneDataStorage.h"

widget::widget(QF::IQF_Main* pMain)
{
    //主控也可以通过QVariant的方式获得
   // QF::IQF_Main* pMain = (QF::IQF_Main*)qApp->property("MainPtr").value<void*>();

    QVBoxLayout* mainLayout = new QVBoxLayout;
    QHBoxLayout* layout = new QHBoxLayout;
    QVBoxLayout* vlayout = new QVBoxLayout;

    //初始化Mitk
    IQF_MitkInit* pMitkInit = (IQF_MitkInit*)pMain->GetInterfacePtr(QF_MitkMain_Init);
    pMitkInit->Init(mitk::StandaloneDataStorage::New());

    //创建插件
    QF::QF_Plugin* multiViewPlugin = dynamic_cast<QF::QF_Plugin*>(QObjectFactory::Instance()->Create("MultiViewsWidget"));
    QF::QF_Plugin* dataManagerPlugin = dynamic_cast<QF::QF_Plugin*>(QObjectFactory::Instance()->Create("DataManagerWidget"));  
    QF::QF_Plugin* navigatePlugin = dynamic_cast<QF::QF_Plugin*>(QObjectFactory::Instance()->Create("ImageNavigatorWidget"));
    QF::QF_Plugin* volumeVisualizationPlugin = dynamic_cast<QF::QF_Plugin*>(QObjectFactory::Instance()->Create("VolumeVisualizationWidget"));
    QF::QF_Plugin* measurementPlugin = dynamic_cast<QF::QF_Plugin*>(QObjectFactory::Instance()->Create("ManualSegmentationWidget"));

    //初始化插件
    multiViewPlugin->SetMainPtr(pMain);
    multiViewPlugin->InitResource(NULL);
    dataManagerPlugin->SetMainPtr(pMain);
    dataManagerPlugin->InitResource(NULL);
    navigatePlugin->SetMainPtr(pMain);
    navigatePlugin->InitResource(NULL);
    volumeVisualizationPlugin->SetMainPtr(pMain);
    volumeVisualizationPlugin->InitResource(NULL);
    measurementPlugin->SetMainPtr(pMain);
    measurementPlugin->InitResource(NULL);

    //创建界面
    vlayout->addWidget(dynamic_cast<QWidget*>(dataManagerPlugin));
    vlayout->addWidget(dynamic_cast<QWidget*>(navigatePlugin));
    layout->addLayout(vlayout,2);
    layout->addWidget(dynamic_cast<QWidget*>(multiViewPlugin),8);
    layout->addWidget(dynamic_cast<QWidget*>(volumeVisualizationPlugin), 3);
    layout->addWidget(dynamic_cast<QWidget*>(measurementPlugin), 3);

    mainLayout->addLayout(layout);
    mainLayout->addWidget(dynamic_cast<QWidget*>(QObjectFactory::Instance()->Create("StatusBarWidget")));

    setLayout(mainLayout);

    //调用IO接口读取数据
    IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    IQF_MitkIO* pIO = (IQF_MitkIO*)pMain->GetInterfacePtr(QF_MitkMain_IO);
    if (pDataManager&&pIO)
    {
        pIO->LoadFiles();
    }
}


widget::~widget()
{

}
