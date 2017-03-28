#ifndef LoadPlugin_h__
#define LoadPlugin_h__

#include <ctkPluginActivator.h> 
#include <ctkPluginContext.h>

class ServiceOne;

class LoadPlugin : public QObject, public ctkPluginActivator
{
    Q_OBJECT
    Q_INTERFACES(ctkPluginActivator)

#ifdef HAVE_QT5
        Q_PLUGIN_METADATA(IID "org_ctk_example")
#endif

public:
    LoadPlugin();
    void start(ctkPluginContext *Context);
    void stop(ctkPluginContext *Context);

private:
    ServiceOne   *m_service;
};
#endif // LoadPlugin_h__