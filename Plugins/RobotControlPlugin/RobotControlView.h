#ifndef RobotControlView_h__
#define RobotControlView_h__

#include "PluginView.h"
#include <QWidget>
#include <QVector3D>
#include <../NDIPlugin/SphereFit.h>
#include "RobotControlImpl.h"

class QPushButton;
class QLineEdit;
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
    void SignalStop();
    void SignalBackToHome();
    void SignalStartDrag();
    void SignalStopDrag();
    void SignalMoveX(double step, double speed);
    void SignalMoveY(double step, double speed);
    void SignalMoveZ(double step, double speed);
    void SignalMoveToPosition(double x, double y, double z);
    void SignalMoveToExpression(SpFlangeExpression expression);

    void SignalApproachToNDIPosition(const QVector3D& ndiPosition, double acceptError ,double maxStep,double minStep,double relaxFactor, bool autoStop);
    void SignalNDIErrorEstimate(const QVector3D& ndiPosition);

    void SignalAutoCaculateOffset(const QVector3D& ndiPosition, double acceptError, double maxStep, double minStep, double relaxFactor);
    

    void SignalMoveToPositionWithOffset(QVector3D position,QVector3D offset);
protected slots:
    void SlotCurrentPositionChanged(QVector3D position);
    void SlotNDIErrorEstimated(double error);
    void SlotCurrentTExpressionChanged(QMatrix4x4 tExpression);
    void SlotCurrentPExpressionChanged(SpFlangeExpression pExpression);
    void SlotApproachFinished();
    void SlotAutoCaculateOffsetCompleted();
private:
    void RecordTargetPosition();
    void AddSpherePoint();
    void ApproachToTargetPosition();
    void CaculateOffset();
    void AutoCaculateOffset();

private:
    RobotControlImpl* m_RobotControlImpl;
    QThread *m_robotThread;
    QVector3D m_currentRobotPosition;
    QVector3D m_targetRobotPositionAppro;
    QVector3D m_targetNdiPosition;
    SphereFit m_sphere;
};

#endif // RobotControlView_h__