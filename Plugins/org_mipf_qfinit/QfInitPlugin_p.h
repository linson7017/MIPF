#ifndef QfInitPlugin_p_h__
#define QfInitPlugin_p_h__

#include <ctkPluginActivator.h> 
#include <ctkPluginContext.h>

class QfServiceInstance;

class QfInitPlugin : public QObject, public ctkPluginActivator
{
    Q_OBJECT
        Q_INTERFACES(ctkPluginActivator)
#ifdef HAVE_QT5
        Q_PLUGIN_METADATA(IID "org_ctk_example")
#endif

public:
    QfInitPlugin();
    void start(ctkPluginContext *Context);
    void stop(ctkPluginContext *Context);

private:
    QfServiceInstance   *m_service;
};
#endif // QfInitPlugin_p_h__