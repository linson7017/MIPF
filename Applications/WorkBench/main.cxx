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
    //Mitkע��
    QmitkRegisterClasses();


    /**************���򻷾���Qt�����ĳ�ʼ��*******************/
    //��ʼ�����򻷾�
    QString wk = qApp->applicationFilePath();
    app_env appenv(wk.toLocal8Bit().constData());
    appenv.setMainPtr(QF::QF_CreateMainObject(wk.toLocal8Bit().constData()));
    //ָ��qtframework����Ŀ¼��ûָ�����Զ�����Ϊ��ִ�г���·����ui_qtframework/app_name/Ŀ¼
    //app_env::setWorkDir("G:/Projects/20160111/bin/ui_qtframework/UITestApp");
    //��ʼ��qt����
    qt_context context(&qtapplication);
    //����qt�������
    qt_context::setApplicationCoding("UTF-8");
    //����qt����Ĭ������
    qt_context::setDefaultLanguage("Chinese");
    //����qt������
    qt_context::setApplicationStyle("Fusion");

    //���qt����������·��
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