#ifndef RobotControlView_h__
#define RobotControlView_h__

#include "PluginView.h"
#include <QWidget>
#include <QVector3D>

class QPushButton;
class QLineEdit;
class RobotControlImpl;
class QThread;


class RobotControlView :public QObject, public PluginView
{
    Q_OBJECT
public:
    RobotControlView(QF::IQF_Main* pMain);
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);

signals:
    void SignalInit();
    void SignalStartDrag();
    void SignalStopDrag();
    void SignalMoveX(double step, double speed);
    void SignalMoveY(double step, double speed);
    void SignalMoveZ(double step, double speed);
    void SignalMoveToPosition(double x, double y, double z);
    void SignalApproachToNDIPosition(const QVector3D& ndiPosition, const QVector3D& initPosition,double maxStep,double minStep,double relaxFactor);
    void SignalNDIErrorEstimate(const QVector3D& ndiPosition);
protected slots:
    void SlotCurrentPositionChanged(QVector3D position);
    void SlotNDIErrorEstimated(double error);
private:
    RobotControlImpl* m_RobotControlImpl;
    QThread *m_robotThread;
    QVector3D m_currentRobotPosition;
    QVector3D m_targetRobotPositionAppro;
    QVector3D m_targetNdiPosition;
};

#endif // RobotControlView_h__