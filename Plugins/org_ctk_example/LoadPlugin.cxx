#include "LoadPlugin_p.h"

#include <QtPlugin> 
#include "ServiceOne.h"
#include "QDebug"

LoadPlugin::LoadPlugin():m_service(0)
{

}


void LoadPlugin::start(ctkPluginContext *Context)
{
    qDebug() << "Plugin Load Plugin Start!";
    m_service = new ServiceOne;
    ctkServiceRegistration re = Context->registerService(QStringList("Service"), m_service);
    if (re)
    {
        qDebug() << "Service Registered Success!";
    }
    else
    {
        qDebug() << "Service Registered Failed!";

    }
    
}

void LoadPlugin::stop(ctkPluginContext *Context)
{
    qDebug() << "Plugin Load Plugin Stop£¡";
    if (m_service)
    {
        delete m_service;
        m_service = 0;
    }
}

#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
Q_EXPORT_PLUGIN2(org_commontk_log, ctkLogPlugin)
#endif
