#include "MainWindow.h"
#include "SplashWindow.h"
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

    /**************���򻷾���Qt�����ĳ�ʼ��*******************/
    //��ʼ�����򻷾�
    QString wk = qApp->applicationFilePath();
    QVariant v;

    app_env appenv(wk.toLocal8Bit().constData());

    QF::IQF_Main* pMain = QF::QF_CreateMainObject(wk.toLocal8Bit().constData(), qApp->applicationDirPath().toLocal8Bit().constData(), false);
    appenv.setMainPtr(pMain);
    qApp->setProperty("MainPtr", QVariant::fromValue(appenv.getMainPtr()));
    //��ʼ��qt����
    qt_context context(&qtapplication);
    //����qt�������
    qt_context::setApplicationCoding("UTF-8");


    //���qt����������·��
#if defined _WIN32 || defined WIN32 || defined __NT__ || defined __WIN32__
    qt_context::addLibraryPath(wk.append("plugins/win32").toLocal8Bit().constData());
#else
    qt_context::addLibraryPath(wk.append("/plugins/linux").toLocal8Bit().constData());
#endif   

#define QFCONFIG_DEBUG  
#ifdef QFCONFIG_DEBUG
    //���� qfconfig Ŀ¼
    QString cmd = QString("xcopy %1 %2 /S /y /Q")
        .arg("S:\\Codes\\MIPF\\bin\\qfconfig\\WorkBench")
        .arg("S:\\Codes\\MIPF-build\\bin\\qfconfig\\WorkBench");
    system(cmd.toLocal8Bit().constData());
#endif

    QSettings set(QString(pMain->GetConfigPath()).append("/config.ini"), QSettings::IniFormat);
    set.beginGroup("config");
    std::string startXML = set.value("Start-XML-File", "main.xml").toString().toStdString();
    std::string style = set.value("Default-Application-Style", "fusion").toString().toStdString();
    std::string splashImage = set.value("Splash-Image", "@image/splash.png").toString().toStdString();
    set.endGroup();
    //����qt����Ĭ������
    // qt_context::setDefaultLanguage("Chinese");
    //����qt������
    qt_context::setApplicationStyle(style.c_str());

    SplashWindow splashWindow(pMain, R::Instance()->getImageResourceUrl(splashImage.c_str()).c_str());
    splashWindow.show();
    pMain->Init();
    MainWindow mainWidget(startXML.c_str(), &splashWindow);
    mainWidget.setShowMode(Activity::MAXIMIZED);
    mainWidget.active();
    splashWindow.close();

    return qtapplication.exec();
}