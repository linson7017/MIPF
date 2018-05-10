#ifndef LauncherView_h__
#define LauncherView_h__

#include "UIs/QF_Plugin.h"

class MainWindow;

class LauncherView : public QF::QF_Plugin
{
public:
    LauncherView();
    ~LauncherView();
    virtual void InitResource() {};
    virtual void SetMainPtr(QF::IQF_Main* pMain) { m_pMain = pMain; };
    virtual void SetupResource();
    virtual WndHandle GetPluginHandle() ;
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);

private:
    QF::IQF_Main* m_pMain;
    MainWindow* m_pMainWindow;
};

#endif // LauncherView_h__