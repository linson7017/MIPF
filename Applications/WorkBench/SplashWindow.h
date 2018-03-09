/********************************************************************
	FileName:    SplashWindow.h
	Author:        Ling Song
	Date:           Month 3 ; Year 2018
	Purpose:	     
*********************************************************************/
#ifndef SplashWindow_h__
#define SplashWindow_h__

#include <QSplashScreen>
#include "iqf_observer.h"

namespace QF {
    class IQF_Main;
}
class QLabel;

class SplashWindow:public QWidget, public QF::IQF_Observer
{
public:
    SplashWindow(QF::IQF_Main* pMain,const char* backgrouUrl);
    ~SplashWindow();
    void showMessage(const char* message);
protected:
    void Update(const char* szMessage, int iValue = 0, void* pValue = 0);

    virtual void paintEvent(QPaintEvent* paintEvent);
    virtual void resizeEvent(QResizeEvent* resizeEvent);
    QRegion createMaskRegion(const QImage & image, bool posMask = true);
private:
    QF::IQF_Main* m_pMain;
    QLabel* m_pMessageLabel;
    QImage m_backgroundImage;
};
#endif // SplashWindow_h__