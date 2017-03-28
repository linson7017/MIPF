#ifndef PluginLoader_h__
#define PluginLoader_h__

#include <QtPlugin>  

enum PLUGIN_LOAD_RESULT
{
    LOAD_SUCCESS,
    LOAD_FAILED,
    UNKNOW_EXCEPTION
};

#define CAST_SERVICE(ServiceClass,ServiceInstance) qobject_cast<ServiceClass*>(ServiceInstance)

class PluginLoader
{
public:
    virtual void AddPluginSearchPath(const char* path)=0;
    virtual PLUGIN_LOAD_RESULT LoadAllPlugin(const char* pluginsPath) = 0;
    virtual PLUGIN_LOAD_RESULT InstallPlugin(const char* pluginSybolName) = 0;
    virtual QObject* GetService(const char* serviceName) = 0;
};

Q_DECLARE_INTERFACE(PluginLoader, "PluginLoader/1.0");
#endif // PluginLoader_h__