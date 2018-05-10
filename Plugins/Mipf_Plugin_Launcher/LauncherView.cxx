#include "LauncherView.h"
#include "iqf_main.h"

#include <iostream>
//qtxml
#include "Res/R.h"
#include "Common/app_env.h"
#include "Common/qt_context.h"

//qt
#include <QVariant>
#include <QApplication>
#include <QSettings>

#include "MainWindow.h"
#include "SplashWindow.h"

#include "tclap/CmdLine.h"


LauncherView::LauncherView() 
{
 
}

WndHandle LauncherView::GetPluginHandle()
{
    return m_pMainWindow;
}

void LauncherView::SetupResource()
{
    QString wk = qApp->applicationFilePath();
    QVariant v;
    app_env appenv(wk.toLocal8Bit().constData());
    appenv.setMainPtr(m_pMain);
    qApp->setProperty("MainPtr", QVariant::fromValue(appenv.getMainPtr()));
    //初始化qt环境
    qt_context context(qApp);
    //设置qt程序编码
    qt_context::setApplicationCoding("UTF-8");

    std::vector<std::string> arguments;
    foreach (QString argStr,qApp->arguments())
    {
        arguments.push_back(argStr.toLocal8Bit().constData());
    }
    std::string configFilename = "config.ini";
    if (arguments.size()>1)
    {
        try {
            TCLAP::CmdLine cmd("Command description message", ' ', "0.9");
            TCLAP::ValueArg<std::string> configArg("c", "ConifgFileName", "The config file to start application.", false, "config.ini", "string");
            cmd.add(configArg);
            cmd.parse(arguments);
            configFilename = configArg.getValue();
        }
        catch (TCLAP::ArgException &e)  // catch any exceptions
        {
            std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        }
    }
    QString configFilePath = QString(m_pMain->GetConfigPath()) + QString("/%1").arg(configFilename.c_str());
    std::cout << "The config file of application is: " << configFilename << std::endl;
    if (!QFile(configFilePath.toLocal8Bit().constData()).exists())
    {
        std::cout << "Config file " << configFilePath.toLocal8Bit().constData() << " does not exist ! The application will use the default settings !" << std::endl;
    }
    QSettings set(configFilePath, QSettings::IniFormat);
    set.beginGroup("config");
    std::string startXML = set.value("Start-XML-File", "main.xml").toString().toStdString();
    std::string style = set.value("Default-Application-Style", "fusion").toString().toStdString();
    std::string splashImage = set.value("Splash-Image", "").toString().toStdString();
    std::string componentsFile = set.value("Components-Config-File", "components.cfg").toString().toStdString();
    std::string pluginsFile = set.value("Plugins-Config-File", "plugins.cfg").toString().toStdString();
    set.endGroup();
    //设置qt程序默认语言
    // qt_context::setDefaultLanguage("Chinese");
    //设置qt程序风格
    qt_context::setApplicationStyle(style.c_str());

#define  SHOW_SPASH
    if (splashImage.empty())
    {
#undef SHOW_SPASH
    }
#ifdef _DEBUG
#undef SHOW_SPASH
#endif


#ifdef SHOW_SPASH
    SplashWindow splashWindow(m_pMain, R::Instance()->getImageResourceUrl(splashImage.c_str()).c_str());
    splashWindow.show();
#endif // _DEBUG

    m_pMain->Init(componentsFile.c_str(), pluginsFile.c_str());

#ifdef  SHOW_SPASH
    m_pMainWindow = new MainWindow(startXML.c_str(), &splashWindow);
#else
    m_pMainWindow = new MainWindow(startXML.c_str());
#endif
    m_pMainWindow->setShowMode(Activity::MAXIMIZED);
    m_pMainWindow->active();

#ifdef SHOW_SPASH
    splashWindow.close();
#endif
}

void LauncherView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "") == 0)
    {
        //do what you want for the message
    }
}