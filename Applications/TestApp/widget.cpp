#include "widget.h"

#include <QtWidgets>
#include "Utils/QObjectFactory.h"
#include "UIs/QF_Plugin.h"
#include "MitkMain/IQF_MitkDataManager.h"
#include "MitkMain/IQF_MitkIO.h"
#include "iqf_main.h"

widget::widget(QF::IQF_Main* pMain)
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    QHBoxLayout* layout = new QHBoxLayout;
    QVBoxLayout* vlayout = new QVBoxLayout;

    QF::QF_Plugin* multiViewPlugin = dynamic_cast<QF::QF_Plugin*>(QObjectFactory::Instance()->Create("MultiViewsWidget"));
    QF::QF_Plugin* dataManagerPlugin = dynamic_cast<QF::QF_Plugin*>(QObjectFactory::Instance()->Create("DataManagerWidget"));  
    QF::QF_Plugin* navigatePlugin = dynamic_cast<QF::QF_Plugin*>(QObjectFactory::Instance()->Create("ImageNavigatorWidget"));
    QF::QF_Plugin* volumeVisualizationPlugin = dynamic_cast<QF::QF_Plugin*>(QObjectFactory::Instance()->Create("VolumeVisualizationWidget"));
    QF::QF_Plugin* measurementPlugin = dynamic_cast<QF::QF_Plugin*>(QObjectFactory::Instance()->Create("MeasurementWidget"));

    
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

    vlayout->addWidget(dynamic_cast<QWidget*>(dataManagerPlugin));
    vlayout->addWidget(dynamic_cast<QWidget*>(navigatePlugin));
    layout->addLayout(vlayout,2);
    layout->addWidget(dynamic_cast<QWidget*>(multiViewPlugin),8);
    layout->addWidget(dynamic_cast<QWidget*>(volumeVisualizationPlugin), 3);
    layout->addWidget(dynamic_cast<QWidget*>(measurementPlugin), 3);

    mainLayout->addLayout(layout);
    mainLayout->addWidget(dynamic_cast<QWidget*>(QObjectFactory::Instance()->Create("StatusBarWidget")));

    setLayout(mainLayout);

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
