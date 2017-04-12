#include "RobotControlImpl.h"

#include <QCoreApplication>
#include <QFile>
#include <QtMath>
#include <QTime>

#include "hardwareController/robot_loader.h"
#include "hardwareController/force_sensor_loader.h"
#include "hardwareController/laser_range_finder_loader.h"


extern bool global_robot_stop;
extern QVector3D global_ndi_pos;
extern QMatrix4x4 global_registration_matrix;

void RobotControlImpl::SlotInit()
{
    if (m_robotController.isNull())
    {
        SpRobotLoader robotLoader;
        m_robotController = robotLoader.Load();
        if (m_robotController.isNull())
        {
            //TODO:发送失败消息
            //ret.SetErrorCode(ERROR_HARDWARE_FAILED);
        }
    }

    SpResult ret;
    ret = m_robotController->Init();
    m_robotController->BackInitPosition();

    while (m_robotController->IsBusy())
    {

    }

    m_robotController->Stop();

    if (m_forceSensorController.isNull())
    {
        SpForceSensorLoader forceSensorLoader;
        m_forceSensorController = forceSensorLoader.Load();
        if (m_forceSensorController.isNull())
        {
            //TODO:errorcode需要改成正确的
            ret.SetErrorCode(ERROR_HARDWARE_FAILED);
        }
    }


    ret = m_forceSensorController->Init();
    QSharedPointer<SpTool> tool = QSharedPointer<SpTool>(new SpTool);
    ret = tool->Load("laserTool");
    if (m_robotController.isNull())
    {
        ret.SetErrorCode(ERROR_HARDWARE_FAILED);
    }
    else
    {
        m_robotController->SetTool(tool);
    }

    /*if (m_laserRangeFinderController.isNull())
    {
        SpLaserRangeFinderLoader loader;
        m_laserRangeFinderController = loader.Load();
    }

    if (!m_laserRangeFinderController.isNull())
    {
        ret = m_laserRangeFinderController->Init();
    }*/
}

void RobotControlImpl::SlotStartDrag()
{
    if (m_robotController.isNull())
    {
        return;
    }
    if (1)
    {
        m_isDragging = true;

        SpResult ret;
        m_robotController->UnlockRadial();
        ret = m_robotController->StartMoveWithForce();

        while (m_isDragging)
        {
            ret = m_robotController->WaitMotion();
            if (ret.IsSuccess())
            {
                QSharedPointer<SpForceSonserParam> param = m_forceSensorController->GetParam();
                if (param->Valid())
                {
                    ret = m_robotController->MoveWithForce(param);
                }
            }
            else
            {
                m_isDragging = false;
            }

            QCoreApplication::processEvents();
        }
        ret = m_robotController->StopMoveWithForce();
    }
}

void RobotControlImpl::SlotStopDrag()
{
    m_isDragging = false;
    m_robotController->Pause();
    emit CurrentPositionChanged(CurrentPosition());
}

void RobotControlImpl::SlotMoveX(double step, double speed)
{
    qDebug() << "Slot MoveX: "<<step;
    if (m_robotController.isNull())
    {
        return;
    }
    SpFlangeExpression expression = m_robotController->GetFlangePosition();
    qDebug() << "Current Position: " << expression.GetPosition();
    expression.SetX(expression.GetX() + step);
    m_robotController->MoveToPosition(expression, speed);
    qDebug() << "Move X: "<< CurrentPosition();
    while (m_robotController->IsBusy())
    {

    }
    emit CurrentPositionChanged(CurrentPosition());
}
void RobotControlImpl::SlotMoveY(double step, double speed)
{
    if (m_robotController.isNull())
    {
        return;
    }
    SpFlangeExpression expression = m_robotController->GetFlangePosition();
    expression.SetY(expression.GetY() + step);
    m_robotController->MoveToPosition(expression,speed);
    while (m_robotController->IsBusy())
    {

    }
    m_robotController->Pause();
    global_robot_stop = true;
    emit CurrentPositionChanged(CurrentPosition());
}
void RobotControlImpl::SlotMoveZ(double step, double speed)
{
    if (m_robotController.isNull())
    {
        return;
    }
    SpFlangeExpression expression = m_robotController->GetFlangePosition();
    expression.SetZ(expression.GetZ() + step);
    m_robotController->MoveToPosition(expression, speed);
    while (m_robotController->IsBusy())
    {

    }
    m_robotController->Pause();
    global_robot_stop = true;
    emit CurrentPositionChanged(CurrentPosition());
    
}

void RobotControlImpl::SlotMoveToPosition(double x, double y, double z)
{
    if (m_robotController.isNull())
    {
        return;
    }
    SpFlangeExpression expression = m_robotController->GetFlangePosition();
    expression.SetPosition(QVector3D(x, y, z));
    m_robotController->MoveToPosition(expression, 1);
    while (m_robotController->IsBusy())
    {

    }
    m_robotController->Pause();
    global_robot_stop = true;
    emit CurrentPositionChanged(CurrentPosition());
}

QVector3D RobotControlImpl::CurrentPosition()
{
    return m_robotController->GetFlangePosition().GetPosition();
}

void RobotControlImpl::SlotNDIErrorEstimate(const QVector3D& ndiPosition)
{
    SpFlangeExpression expression = m_robotController->GetFlangePosition();
    expression.SetPosition(ndiPosition);
    m_robotController->MoveToPosition(expression, 1);
    while (m_robotController->IsBusy())
    {

    }
    double offset = 20;
    //X
    double error = 0;
    int times = 0;
    double step = 30;
    double relax = 0.7;
    int num = 10;
    for (int i=0;i<num;i++)
    {    
        SlotMoveX(step, 2);
        SlotMoveX(-step, 2);
        error += global_ndi_pos.distanceToPoint(ndiPosition)*global_ndi_pos.distanceToPoint(ndiPosition);
        times++;
        SlotMoveX(-step, 2);
        SlotMoveX(step, 2);
        error += global_ndi_pos.distanceToPoint(ndiPosition)*global_ndi_pos.distanceToPoint(ndiPosition);
        times++;
        step = step*relax;
    }
    step = 30;
    for (int i = 0; i < num; i++)
    {   
        SlotMoveY(20, 2);
        SlotMoveY(-20, 2);
        error += global_ndi_pos.distanceToPoint(ndiPosition)*global_ndi_pos.distanceToPoint(ndiPosition);
        times++;
        SlotMoveY(-20, 2);
        SlotMoveY(20, 2);
        error += global_ndi_pos.distanceToPoint(ndiPosition)*global_ndi_pos.distanceToPoint(ndiPosition);
        times++;
        step = step*relax;
    }
    step = 30;
    for (int i = 0; i < num; i++)
    {
        SlotMoveZ(20, 2);
        SlotMoveZ(-20, 2);
        error += global_ndi_pos.distanceToPoint(ndiPosition)*global_ndi_pos.distanceToPoint(ndiPosition);
        times++;
        SlotMoveZ(-20, 2);
        SlotMoveZ(20, 2);
        error += global_ndi_pos.distanceToPoint(ndiPosition)*global_ndi_pos.distanceToPoint(ndiPosition);
        times++;
        step = step*relax;
    }
    double variance = qSqrt((double)error / times);
    emit SignalNDIErrorEstimated(variance);
}


void RobotControlImpl::SlotApproachToNDIPosition(const QVector3D& ndiPosition, const QVector3D& initPosition, double maxStep, double minStep, double relaxFactor)
{
    qDebug() << "SlotApproachToNDIPosition";
    qDebug() << "Target NDI Position: " << ndiPosition;
    SpFlangeExpression expression = m_robotController->GetFlangePosition();
    /*expression.SetPosition(initPosition);
    m_robotController->MoveToPosition(expression,1);
    while (m_robotController->IsBusy())
    {

    }*/
    double moveSpeed = 2.0;
    bool greater = true;
    double step = maxStep;
    double relax = relaxFactor;
    int times = 0;
    double tolerance = 0.1;
    double metric = INT_MAX;
    //approch X
    qDebug() << "Approach X!";
    while (step>minStep)
    {     
        expression = m_robotController->GetFlangePosition();
        double currentX = expression.GetX() + (greater?-1:1)*step;
        expression.SetX(currentX);

        
        m_robotController->MoveToPosition(expression, moveSpeed);
        while (m_robotController->IsBusy())
        {

        }
        emit CurrentPositionChanged(CurrentPosition());
       // m_robotController->Pause();
        
        qDebug() << "Current NDI Position: " << global_ndi_pos;
        double cmetric = qAbs(ndiPosition.distanceToPoint(global_ndi_pos));
        qDebug() << "Current Metric: " << cmetric;
        
        if (cmetric>metric)
        {
            greater = !greater;
            step = step*relax;
        }
        metric = cmetric;
        times++;
        qDebug() << "Times: " << times;
        qDebug() << "Step: " << step;
    }

    //approch Y
    greater = true;
    step = maxStep;
    relax = relaxFactor;
    times = 0;
    tolerance = 0.1;
    metric = INT_MAX;
    qDebug() << "Approach Y!";
    while (step > minStep)
    {
        
        expression = m_robotController->GetFlangePosition();
        double currentY = expression.GetY() + (greater ? -1 : 1)*step;
        expression.SetY(currentY);

        m_robotController->MoveToPosition(expression, moveSpeed);
        while (m_robotController->IsBusy())
        {

        }
        emit CurrentPositionChanged(CurrentPosition());
        qDebug() << "Current NDI Position: " << global_ndi_pos;
        double cmetric = qAbs(ndiPosition.distanceToPoint(global_ndi_pos));
        qDebug() << "Current Metric: " << cmetric;
        if (cmetric > metric)
        {
            greater = !greater;
            step = step*relax;
        }
        metric = cmetric;
        times++;
    }

    //approch Z
    greater = true;
    step = maxStep;
    relax = relaxFactor;
    times = 0;
    tolerance = 0.1;
    metric = INT_MAX;
    qDebug() << "Approach Z!";
    while (step > minStep)
    {      
        expression = m_robotController->GetFlangePosition();
        double currentZ = expression.GetZ() + (greater ? -1 : 1)*step;
        expression.SetZ(currentZ);

        m_robotController->MoveToPosition(expression, moveSpeed);
        while (m_robotController->IsBusy())
        {

        }
        emit CurrentPositionChanged(CurrentPosition());
        qDebug() << "Current NDI Position: " << global_ndi_pos;
        double cmetric = qAbs(ndiPosition.distanceToPoint(global_ndi_pos));
        qDebug() << "Current Metric: " << cmetric;
        if (cmetric > metric)
        {
            greater = !greater;
            step = step*relax;
        }
        metric = cmetric;
        times++;
    }

    qDebug() << "Approach Target NDI Position Finished!";
    qDebug() << "Target: " << ndiPosition;
    qDebug() << "Current: " << global_ndi_pos;
    qDebug() << "Distance: " << ndiPosition.distanceToPoint(global_ndi_pos);
    m_robotController->Pause();
}