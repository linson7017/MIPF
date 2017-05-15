#include "QmitkRegisterClasses.h"
#include "MainWindow.h"
#include <QApplication>
#include <itksys/SystemTools.hxx>


//qtframework
#include <Common/app_env.h>
#include <Common/qt_context.h>
#include <Res/R.h>


//qfmain
#include "iqf_main.h"
#include "iqf_component.h"


#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

int main(int argc, char *argv[])
{
    QApplication qtapplication(argc, argv);
    //Mitk注册
    QmitkRegisterClasses();


    /**************程序环境和Qt环境的初始化*******************/
    //初始化程序环境
    QString wk = qApp->applicationFilePath();
    app_env appenv(wk.toLocal8Bit().constData());
    appenv.setMainPtr(QF::QF_CreateMainObject(wk.toLocal8Bit().constData()));
    //指定qtframework工作目录，没指定则自动设置为可执行程序路径下ui_qtframework/app_name/目录
    //app_env::setWorkDir("G:/Projects/20160111/bin/ui_qtframework/UITestApp");
    //初始化qt环境
    qt_context context(&qtapplication);
    //设置qt程序编码
    qt_context::setApplicationCoding("UTF-8");
    //设置qt程序默认语言
    qt_context::setDefaultLanguage("Chinese");
    //设置qt程序风格
    qt_context::setApplicationStyle("Fusion");

    //添加qt插件库的搜索路径
#if defined _WIN32 || defined WIN32 || defined __NT__ || defined __WIN32__
    qt_context::addLibraryPath(wk.append("plugins/win32").toLocal8Bit().constData());
#else
    qt_context::addLibraryPath(wk.append("/plugins/linux").toLocal8Bit().constData());
#endif   

    MainWindow mainWidget("main.xml");
    mainWidget.setShowMode(Activity::MAXIMIZED);
    mainWidget.active();

    return qtapplication.exec();
}