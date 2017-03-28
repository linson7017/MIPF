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
    //指定qtframework工作目录，没指定则自动设置为可执行程序路径下ui_qtframework/app_name/目录
    //app_env::setWorkDir("G:/Projects/ESonRad20160111/bin/ui_qtframework/UITestApp");
    //初始化qt环境
    qt_context context(app);
    //设置qt程序编码
    qt_context::setApplicationCoding("UTF-8");
    //设置qt程序默认语言
    qt_context::setDefaultLanguage("Chinese");
    //设置qt程序风格
    qt_context::setApplicationStyle("Fusion");

    return true;
}