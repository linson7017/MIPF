#include "PluginLoaderInstance.h"
#include <Utils/Log.h>
#include <iostream>

#include <ctkPlugin.h>
#include <ctkPluginFrameworkFactory.h>
#include <ctkPluginContext.h>
#include <ctkPluginException.h>
#include <ctkPluginFrameworkLauncher.h>

PluginLoaderInstance::PluginLoaderInstance()
{
    
}

void PluginLoaderInstance::AddPluginSearchPath(const char* path)
{
    ctkPluginFrameworkLauncher::addSearchPath(path, true);
}

PLUGIN_LOAD_RESULT PluginLoaderInstance::LoadAllPlugin(const char* pluginsPath)
{
    return UNKNOW_EXCEPTION;
}

PLUGIN_LOAD_RESULT PluginLoaderInstance::InstallPlugin(const char* pluginSybolName)
{
    bool SUCCEEDED;
    try
    {
        SUCCEEDED = ctkPluginFrameworkLauncher::start(pluginSybolName);
    }

    catch (ctkPluginException &e)
    {
        std::cout << "Error in " << pluginSybolName << " " << e.message().toStdString() << std::endl;
        const ctkException* e2 = e.cause();

        if (e2)
            std::cout << e2->message().toStdString() << std::endl;
        return LOAD_FAILED;
    }

    catch (ctkRuntimeException &e)
    {
        std::cout << "Error in " << pluginSybolName << " " << e.what() << std::endl;
        const ctkException* e2 = e.cause();

        if (e2)
            std::cout << e2->message().toStdString() << std::endl;
        return LOAD_FAILED;
    }
    catch (...)
    {
        std::cout << "Error in " << pluginSybolName << std::endl;
        return UNKNOW_EXCEPTION;
    }


    return LOAD_SUCCESS;
}

QObject* PluginLoaderInstance::GetService(const char* serviceName)
{
    ctkServiceReference reference = ctkPluginFrameworkLauncher::getPluginContext()->getServiceReference(serviceName);
    if (!reference)
    {
        return NULL;
    }
    return ctkPluginFrameworkLauncher::getPluginContext()->getService(reference);
}


#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
Q_EXPORT_PLUGIN2(org_mipf_baseapp, BaseAppInstance)
#endif