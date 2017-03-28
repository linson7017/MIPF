#include "QfInitPlugin_p.h"
#include "QfServiceInstance.h"
#include <iostream>
#include <Utils/Log.h>
#include <QtWidgets/QApplication>

QfInitPlugin::QfInitPlugin() :m_service(0)
{

}

void QfInitPlugin::start(ctkPluginContext *Context)
{
    std::cout << "QfInitPlugin start!" << std::endl;
    m_service = new QfServiceInstance;
    ctkServiceRegistration re = Context->registerService(QStringList("QfService"), m_service);
    if (re)
    {
        std::cout << "Service Registered Success!"<<std::endl;
        m_service->Init(qApp);
    }
    else
    {
        std::cout << "Service Registered Failed!" << std::endl;
    }
}

void QfInitPlugin::stop(ctkPluginContext *Context)
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
