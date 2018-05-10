#include <QApplication>

//qfmain
#include "iqf_main.h"

//qtxml
#include "Utils/PluginFactory.h"
#include "UIs/QF_Plugin.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

#include "qf_log.h"

int main(int argc, char *argv[])
{
    QApplication qtapplication(argc, argv);

    //Create IQF_Main object, meanwhile assign application entry and library path 
    QF::IQF_Main* pMain = QF::QF_CreateMainObject(qApp->applicationFilePath().toLocal8Bit().constData(),
        qApp->applicationDirPath().toLocal8Bit().constData(), false);
    //Load the launcher plugin
    pMain->Init("","init-plugins.cfg");
    //Setup launcher
    QF::QF_Plugin* launcher = dynamic_cast<QF::QF_Plugin*>(QF::PluginFactory::Instance()->Create("Launcher"));
    launcher->SetMainPtr(pMain);
    launcher->SetupResource();

    return qtapplication.exec();
}