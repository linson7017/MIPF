#include "MainWindowPlugin_p.h"
#include <iostream>
#include <Utils/Log.h>
#include "MainWindow.h"
#include "QmitkRegisterClasses.h"

MainWindowPlugin::MainWindowPlugin() :m_service(0)
{

}

void MainWindowPlugin::start(ctkPluginContext *Context)
{
    std::cout << "Start Main window!" << std::endl;
   // m_service = new QfServiceInstance;
   //Mitk×¢²á
    QmitkRegisterClasses();
    MainWindow mainWidget("main.xml");
    mainWidget.setShowMode(Activity::MAXIMIZED);
    mainWidget.active();
}

void MainWindowPlugin::stop(ctkPluginContext *Context)
{
    if (m_service)
    {
        delete m_service;
        m_service = 0;
    }
}

#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
#include <QtPlugin> 
Q_EXPORT_PLUGIN2(org_commontk_log, ctkLogPlugin)
#endif

