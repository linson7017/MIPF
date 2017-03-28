#include "ctkPluginFrameworkFactory.h"
#include <CTKPluginFramework.h>
#include <ctkPluginException.h>
#include <ctkPluginContext.h>
#include <ctkPlugin.h>
#include <QApplication>
#include <QDebug>
#include "../../Plugins/org_ctk_example/IService.h"
#include "../../Plugins/org_mipf_baseapp/BaseApp.h"
#include <QPluginLoader>
#include "ctkPluginFrameworkLauncher.h"
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication qtapplication(argc, argv);

    ctkPluginFrameworkLauncher::addSearchPath("E:/codes/MIPF-build/bin/Release", true);
    ctkPluginFrameworkLauncher::addSearchPath("E:/codes/MITK/MITK-build/MITK-build/bin/Release", true);
    ctkPluginFrameworkLauncher::addSearchPath("E:/codes/MIPF-build/bin/plugins/Release", true);
    QStringList symbolicNames = ctkPluginFrameworkLauncher::getPluginSymbolicNames("E:/codes/MIPF-build/bin/plugins/Release");
    bool SUCCEEDED = false;
    try
    {

        ctkPluginFrameworkLauncher::start("org.mipf.qfinit");

    }
    catch (ctkPluginException &e)
    {
        //std::cout << "Error in " << pluginSybolName << " " << e.message().toStdString() << std::endl;
        const ctkException* e2 = e.cause();

        if (e2)
            std::cout << e2->message().toStdString() << std::endl;
        // return LOAD_FAILED;
    }

    catch (ctkRuntimeException &e)
    {
        // std::cout << "Error in " << pluginSybolName << " " << e.what() << std::endl;
        const ctkException* e2 = e.cause();

        if (e2)
            std::cout << e2->message().toStdString() << std::endl;
        // return LOAD_FAILED;
    }
    catch (...)
    {
        // std::cout << "Error in " << pluginSybolName << std::endl;
        // return UNKNOW_EXCEPTION;
    }
    ctkServiceReference reference = ctkPluginFrameworkLauncher::getPluginContext()->getServiceReference("org.mitk.views.datamanager");
    if (reference)
    {
      //  QWidget* datamanagerWidget = qobject_cast<QWidget*>(ctkPluginFrameworkLauncher::getPluginContext()->getService(reference));
    }




    //qt plugin
    BaseApp* baseapp;
    QPluginLoader  pluginLoader("E:/codes/MIPF-build/bin/Release/org_mipf_baseapp.dll");
    QObject *plugin = pluginLoader.instance();
    if (plugin)
    {
        qDebug()<< "Load Success!" ;
        baseapp = qobject_cast<BaseApp *>(plugin);
        baseapp->Init();
    }

 //   system("pause");

    ////ctk plugin
    //ctkPluginFrameworkFactory* ctkFrameWorkFactory = new ctkPluginFrameworkFactory;
    //QSharedPointer<ctkPluginFramework> framework = ctkFrameWorkFactory->getFramework();

    //try
    //{
    //    framework->init();

    //    framework->start();

    //    qDebug() << "[Info] ctkPluginFramework start ...";
    //}
    //catch (const ctkPluginException &Exception)
    //{
    //    qDebug() << QObject::tr("Failed to initialize the plug-in framework: ") << Exception.what();
    //    return 1;
    //}


    ////install plugin
    //ctkPluginContext* pluginContext = framework->getPluginContext();
    //QSharedPointer<ctkPlugin> Plugin = pluginContext->installPlugin(QUrl::fromLocalFile("E:/codes/MIPF-build/bin/plugins/Release/liborg_ctk_example.dll"));
    //Plugin->start(ctkPlugin::START_TRANSIENT);

    //ctkServiceReference reference = pluginContext->getServiceReference("Service");
    //IService *test = dynamic_cast<IService *>(pluginContext->getService(reference));
    //test->SetParameter(10);
    //qDebug() << test->GetParameter();

    return qtapplication.exec();
}