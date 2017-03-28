#include "BaseAppInstance.h"
#include "../org_mipf_pluginloader/PluginLoader.h"
#include <QPluginLoader>
#include <QApplication.h>
//#include "MainWindow.h"

//services
#include "../org_mipf_qfinit/QfService.h"

void BaseAppInstance::Init() 
{
    std::cout << "Base App Init!" << std::endl; 
    PluginLoader* pPluginLoader;
    QPluginLoader  pluginLoader("E:/codes/MIPF-build/bin/Release/org_mipf_pluginloader.dll");
    QObject *plugin = pluginLoader.instance();
    if (plugin)
    {
        std::cout << "org_mipf_pluginloader Load Success" << std::endl;
        pPluginLoader = qobject_cast<PluginLoader *>(plugin);
        pPluginLoader->AddPluginSearchPath("E:/codes/MIPF-build/bin/plugins/Release");
        pPluginLoader->InstallPlugin("org.mipf.qfinit");
        pPluginLoader->InstallPlugin("org.mipf.mainwindow");
    }
    else
    {
        std::cout << "org_mipf_pluginloader Load Failed" << std::endl;
    }
}





#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
Q_EXPORT_PLUGIN2(org_mipf_baseapp, BaseAppInstance)
#endif