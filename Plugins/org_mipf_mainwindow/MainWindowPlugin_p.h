#include <ctkPluginActivator.h> 
#include <ctkPluginContext.h>

class QfServiceInstance;

class MainWindowPlugin : public QObject, public ctkPluginActivator
{
    Q_OBJECT
    Q_INTERFACES(ctkPluginActivator)
#ifdef HAVE_QT5
        Q_PLUGIN_METADATA(IID "org_ctk_example")
#endif

public:
    MainWindowPlugin();
    void start(ctkPluginContext *Context);
    void stop(ctkPluginContext *Context);

private:
    QfServiceInstance   *m_service;
};

