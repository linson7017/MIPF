#ifndef MainWindow_h__
#define MainWindow_h__

#include "UIs/Activity.h"
#include "iqf_observer.h"

#pragma once

namespace QF {
    class IQF_Main;
}

class MultiViews;
class IQF_MitkDataManager;

class MainWindow : public Activity, public QF::IQF_Observer
{
    Q_OBJECT
public:
    MainWindow(const char* xmlfile);
    ~MainWindow();


private:
    void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
    //Functions
    void OpenDicom();
    void OpenMetaImage();

    void SetupWidgets(const char* xmlfile);

    QF::IQF_Main* m_pMain;
    MultiViews* m_pMultiViews;
    IQF_MitkDataManager* m_pMitkDataManager;
};

#endif // MainWindow_h__
