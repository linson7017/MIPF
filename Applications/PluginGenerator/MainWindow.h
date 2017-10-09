#ifndef MainWindow_h__
#define MainWindow_h__

#include "UIs/Activity.h"
#include "iqf_observer.h"
#pragma once

namespace QF {
    class IQF_Main;
}

class MainWindow : public Activity, public QF::IQF_Observer
{
    Q_OBJECT
public:
    MainWindow(const char* xmlfile);
    ~MainWindow();


private:
    void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
    void Generate();
    void GenerateCMakeList();
    void GenerateSourceFiles();
    void AddToPluginCMakeList();

    void CreateFile(QString fileName, const QString fileContent);
    void CheckAndCreateDirectory(const QString path);
    QF::IQF_Main* m_pMain;

    QString m_CMakeListText;
    QString m_PluginName;
    QString m_ViewName;
    QString m_PluginDir;
    bool m_UseITK;
    bool m_UseVTK;
    bool m_UseMitkWidgets;
    bool m_UseMitkWidgetsExt;
    bool m_UseVMTK;


};

#endif // MainWindow_h__
