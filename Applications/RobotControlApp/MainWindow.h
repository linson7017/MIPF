#ifndef MainWindow_h__
#define MainWindow_h__

#include "UIs/Activity.h"
#include "iqf_observer.h"
#include <QObject>

#pragma once

class QLineEdit;
class QPushButton;
class RobotControl;

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

    QF::IQF_Main* m_pMain;

};

#endif // MainWindow_h__
