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
    RobotControlImpl() : m_isDragging(false), m_robotController(NULL)
    {

    }
    ~RobotControlImpl()
    {

    }
    QVector3D CurrentPosition();
public slots:
    void SlotInit();
    void SlotStartDrag();
    void SlotStopDrag();
    void SlotMoveX(double step, double speed);
    void SlotMoveY(double step, double speed);
    void SlotMoveZ(double step, double speed);
    void SlotMoveToPosition(double x, double y, double z);
    void SlotApproachToNDIPosition(const QVector3D& ndiPosition,const QVector3D& initPosition, double maxStep, double minStep, double relaxFactor);
    void SlotNDIErrorEstimate(const QVector3D& ndiPosition);
signals:
    void CurrentPositionChanged(QVector3D position);
    void SignalNDIErrorEstimated(double error);
private:

    QSharedPointer<ISpRobotController> m_robotController;
    QSharedPointer<ISpForceSensorController> m_forceSensorController;
    QSharedPointer<ISpLaserRangeFinderController> m_laserRangeFinderController;

    bool m_isDragging;

    SpFlangeExpression m_groupPositions[3];
    QMap<int, QStringList> m_validPoints[10];
};
#endif // RobotControlImpl_h__