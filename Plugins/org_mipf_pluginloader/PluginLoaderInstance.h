#include "PluginLoader.h"
#include <ctkPluginFramework.h>

class ctkPluginFrameworkFactory;
class PluginLoaderInstance :public QObject, public PluginLoader
{
    Q_OBJECT
    Q_INTERFACES(PluginLoader)

#if (QT_VERSION > QT_VERSION_CHECK(5,0,0))
        Q_PLUGIN_METADATA(IID "PluginLoaderInstance_ID")
#endif
public:
    PluginLoaderInstance();
    virtual void AddPluginSearchPath(const char* path);
    virtual PLUGIN_LOAD_RESULT LoadAllPlugin(const char* pluginsPath);
    virtual PLUGIN_LOAD_RESULT InstallPlugin(const char* pluginSybolName);
    virtual QObject* GetService(const char* serviceName);

private:
    QSharedPointer<ctkPluginFramework> m_ctkFrameWork;
    ctkPluginFrameworkFactory* m_ctkFrameWorkFactory;
};