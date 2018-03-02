#include "MainWindow.h"
#include <QApplication>
#include <QStyleFactory>


//qtframework
#include <Common/app_env.h>
#include <Common/qt_context.h>
#include <Res/R.h>


//qfmain
#include "iqf_main.h"
#include "iqf_component.h"
#include "Utils/QObjectFactory.h"


#include <QSettings>


#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

int main(int argc, char *argv[])
{
    QApplication qtapplication(argc, argv);

    /**************程序环境和Qt环境的初始化*******************/
    //初始化程序环境
    QString wk = qApp->applicationFilePath();
    QVariant v;

    app_env appenv(wk.toLocal8Bit().constData());

    appenv.setMainPtr(QF::QF_CreateMainObject(wk.toLocal8Bit().constData(), qApp->applicationDirPath().toLocal8Bit().constData()));
    qApp->setProperty("MainPtr", QVariant::fromValue(appenv.getMainPtr()));
    //初始化qt环境
    qt_context context(&qtapplication);
    //设置qt程序编码
    qt_context::setApplicationCoding("UTF-8");


    //添加qt插件库的搜索路径
#if defined _WIN32 || defined WIN32 || defined __NT__ || defined __WIN32__
    qt_context::addLibraryPath(wk.append("plugins/win32").toLocal8Bit().constData());
#else
    qt_context::addLibraryPath(wk.append("/plugins/linux").toLocal8Bit().constData());
#endif   

#ifdef QFCONFIG_DEBUG
    //复制 qfconfig 目录
    QString cmd = QString("xcopy %1 %2 /S /y /Q")
        .arg("S:\\Codes\\MIPF\\bin\\qfconfig\\WorkBench")
        .arg("S:\\Codes\\MIPF-build\\bin\\qfconfig\\WorkBench");
    system(cmd.toLocal8Bit().constData());
#endif

    std::string path = appenv.getConfigResDir();
    QSettings set(QString(static_cast<QF::IQF_Main*>(appenv.getMainPtr())->GetConfigPath()).append("/config.ini"), QSettings::IniFormat);
    set.beginGroup("config");
    std::string startXML = set.value("Start-XML-File", "main.xml").toString().toStdString();
    std::string style = set.value("Default-Application-Style", "fusion").toString().toStdString();
    set.endGroup();
    //设置qt程序默认语言
    // qt_context::setDefaultLanguage("Chinese");
    //设置qt程序风格
    qt_context::setApplicationStyle(style.c_str());

    MainWindow mainWidget(startXML.c_str());
    mainWidget.setShowMode(Activity::MAXIMIZED);
    mainWidget.active();

    return qtapplication.exec();
}