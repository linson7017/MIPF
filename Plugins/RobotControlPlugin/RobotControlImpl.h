#ifndef RobotControlImpl_h__
#define RobotControlImpl_h__

#include <QObject>

#include "hardwareController/ilaser_range_finder_controller.h"
#include "hardwareController/irobot_controller.h"
#include "hardwareController/iforce_sensor_controller.h"

class RobotControlImpl:public QObject
{
    Q_OBJECT
public:
    RobotControlImpl() : m_isDragging(false), m_robotController(NULL), m_speed(1.0)
    {

    }
    ~RobotControlImpl()
    {

    }
    QVector3D CurrentPosition();
    QMatrix4x4 CurrentTExpression();


    void SetApproachFinished(bool finished) { m_isApproachFinished = finished; }
    bool GetApproachFinished() { return m_isApproachFinished; }

    void SetSpeed(double speed) { m_speed = speed; }
    double GetSpeed() { return m_speed; }

public slots:
    void SlotInit();
    void SlotStop();
    void SlotBackToHome();
    void SlotStartDrag();
    void SlotStopDrag();
    void SlotMoveX(double step, double speed);
    void SlotMoveY(double step, double speed);
    void SlotMoveZ(double step, double speed);
    void SlotMoveToPosition(double x, double y, double z);
    void SlotApproachToNDIPosition(const QVector3D& ndiPosition, double acceptError, double maxStep, double minStep, double relaxFactor,bool autoStop);
    void SlotNDIErrorEstimate(const QVector3D& ndiPosition);
    void SlotMoveToExpression(SpFlangeExpression expression);

    void SlotAutoCaculateOffset(const QVector3D& ndiPosition, double acceptError, double maxStep, double minStep, double relaxFactor);
    void SlotMoveToPositionWithOffset(QVector3D position, QVector3D offset);
signals:
    void CurrentPositionChanged(QVector3D position);
    void CurrentTExpressionChanged(QMatrix4x4 tExpression);
    void CurrentPExpressionChanged(SpFlangeExpression pExpression);

    void SignalNDIErrorEstimated(double error);
    void SignalApproachFinished();
    void SignalAutoCaculateOffsetCompleted();
private:
    double Approach(const QVector3D& ndiPosition,double acceptError,double initStep,double minStep,double relaxFactor,int axis=0);
    bool ApproachToNDIPosition(const QVector3D& ndiPosition, double acceptError, double maxStep, double minStep, double relaxFactor, bool autoStop);

    void PExpressionToTExpression(const SpFlangeExpression &pExpression,
        QMatrix4x4 &tExpression/*,
                               QStringList &strExpression*/);
    void TExpressionToPExpression(const QMatrix4x4& tExpression,SpFlangeExpression& pExpression);

    void EmitRobotMoved();

    QSharedPointer<ISpRobotController> m_robotController;
    QSharedPointer<ISpForceSensorController> m_forceSensorController;
    QSharedPointer<ISpLaserRangeFinderController> m_laserRangeFinderController;

    bool m_isDragging;

    SpFlangeExpression m_groupPositions[3];
    QMap<int, QStringList> m_validPoints[10];

    bool m_isApproachFinished;

    double m_speed;
};
#endif // RobotControlImpl_h__