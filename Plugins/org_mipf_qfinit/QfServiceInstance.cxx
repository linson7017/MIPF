#include "QfServiceInstance.h"
#include <QtWidgets/QApplication>

//qtframework
#include <Common/app_env.h>
#include <Common/qt_context.h>
#include <Res/R.h>
#include <iostream>

//qfmain
#include "iqf_main.h"
#include "iqf_component.h"

QfServiceInstance::QfServiceInstance()
{

}

bool QfServiceInstance::Init(QApplication* app)
{
    std::cout << "Qt Frame Work Init!" << std::endl;
    
    QString wk = app->applicationFilePath();
    app_env appenv(wk.toLocal8Bit().constData());
    appenv.setMainPtr(QF::QF_CreateMainObject());
    ((QF::IQF_Main*)appenv.getMainPtr())->RegisterResource(R::Instance());
    //ָ��qtframework����Ŀ¼��ûָ�����Զ�����Ϊ��ִ�г���·����ui_qtframework/app_name/Ŀ¼
    //app_env::setWorkDir("G:/Projects/ESonRad20160111/bin/ui_qtframework/UITestApp");
    //��ʼ��qt����
    qt_context context(app);
    //����qt�������
    qt_context::setApplicationCoding("UTF-8");
    //����qt����Ĭ������
    qt_context::setDefaultLanguage("Chinese");
    //����qt������
    qt_context::setApplicationStyle("Fusion");

    return true;
}