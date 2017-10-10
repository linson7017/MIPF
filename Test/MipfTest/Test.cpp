#include <QApplication>

#include "widget.h"
#include <QVariant>

//qtframework
#include <Common/app_env.h>
#include <Common/qt_context.h>
#include <Res/R.h>

//qfmain
#include "iqf_main.h"

int main(int argc, char *argv[])
{
    QApplication qtapplication(argc, argv);

    /**************���򻷾���Qt�����ĳ�ʼ��*******************/
    //��ʼ�����򻷾�
    QString wk = qApp->applicationFilePath();
    app_env appenv(wk.toLocal8Bit().constData());
    appenv.setMainPtr(QF::QF_CreateMainObject(wk.toLocal8Bit().constData()));
    qApp->setProperty("MainPtr", QVariant::fromValue(appenv.getMainPtr()));
    R::Instance();
    qt_context context(&qtapplication);
    
    

    widget mainWidget((QF::IQF_Main*)appenv.getMainPtr());
    mainWidget.showMaximized();

    return qtapplication.exec();
}