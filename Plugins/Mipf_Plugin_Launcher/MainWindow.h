/********************************************************************
	FileName:    MainWindow.h
	Author:        Ling Song
	Date:           Month 3 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef MainWindow_h__
#define MainWindow_h__
#include "UIs/Activity.h"
#include "iqf_observer.h"

namespace QF {
    class IQF_Main;
}

class IQF_MitkDataManager;
class SplashWindow;

class MainWindow : public Activity, public QF::IQF_Observer
{
    Q_OBJECT
public:
    MainWindow(const char* xmlfile,SplashWindow* pSplashWindow=nullptr);
    ~MainWindow();
private:
    void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
    //Functions
    void OpenDicom();
    void OpenMetaImage();

    void SetupWidgets(const char* xmlfile);

    QF::IQF_Main* m_pMain;
    IQF_MitkDataManager* GetMitkDataManagerInterface();
    SplashWindow* m_pSplashWindow;
};

#endif // MainWindow_h__