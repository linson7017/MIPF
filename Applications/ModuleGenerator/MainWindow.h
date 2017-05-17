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
    public slots:
    void ModuleNameChanged(const QString& text);

private:
    void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
    void Generate();
    void GenerateCMakeList();
    void GenerateSourceFiles();

    void CreateFile(QString fileName, const QString fileContent);
    void CheckAndCreateDirectory(const QString path);
    QF::IQF_Main* m_pMain;

    QString m_CMakeListText;
    QString m_ProjectName;
    QString m_ModuleName;
    QString m_ComName;
    QString m_CommandName;
    QString m_MessageName;
    QString m_Dir;
    bool m_UseITK;
    bool m_UseVTK;
    bool m_UseMitkWidgets;
    bool m_UseMitkWidgetsExt;

    bool m_UseCom;
    bool m_UseCommand;
    bool m_UseMessage;
};

#endif // MainWindow_h__
