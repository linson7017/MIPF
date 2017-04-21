#include "RobotControlImpl.h"

#include <QCoreApplication>
#include <QFile>
#include <QtMath>
#include <QTime>

#include "hardwareController/robot_loader.h"
#include "hardwareController/force_sensor_loader.h"
#include "hardwareController/laser_range_finder_loader.h"


//qRegisterMetaType<SpFlangeExpression>("SpFlangeExpression")

extern bool global_robot_stop;
extern bool global_approach_finished;
extern QVector3D global_ndi_pos;
extern QMatrix4x4 global_registration_matrix;
extern bool global_position_sync;
extern QMatrix4x4 global_t_expression;

extern QMatrix4x4 global_temp_t_expression;

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
    EmitRobotMoved();

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
    EmitRobotMoved();
}

void RobotControlImpl::SlotMoveX(double step, double speed)
{
    global_robot_stop = false;
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
    global_robot_stop = true;
    EmitRobotMoved();
}
void RobotControlImpl::SlotMoveY(double step, double speed)
{
    global_robot_stop = false;
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
    EmitRobotMoved();
}
void RobotControlImpl::SlotMoveZ(double step, double speed)
{
    global_robot_stop = false;
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
    EmitRobotMoved();
    
}

void RobotControlImpl::SlotMoveToPosition(double x, double y, double z)
{
   // global_robot_stop = false;
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
    EmitRobotMoved();
}

void RobotControlImpl::SlotMoveToExpression(SpFlangeExpression expression)
{
   // global_robot_stop = false;
    if (m_robotController.isNull())
    {
        return;
    }
    m_robotController->MoveToPosition(expression, 1);
    while (m_robotController->IsBusy())
    {

    }
    m_robotController->Pause();
    global_robot_stop = true;
    EmitRobotMoved();
}

QVector3D RobotControlImpl::CurrentPosition()
{
    return m_robotController->GetFlangePosition().GetPosition();
}

QMatrix4x4 RobotControlImpl::CurrentTExpression()
{
    QMatrix4x4 m;
    PExpressionToTExpression(m_robotController->GetFlangePosition(), m);
    return m;
}

void RobotControlImpl::TExpressionToPExpression(const QMatrix4x4& tExpression, SpFlangeExpression& pExpression)
{
    
    if (qAbs(tExpression(3,1))!=1)
    {
        double theta1 = -qAsin(tExpression(2, 0));
        double theta2 = qDegreesToRadians(180.0) - theta1;
        double fin1 = qAtan2(tExpression(2, 1) / qCos(theta1), tExpression(2, 2) / qCos(theta1));
        double fin2 = qAtan2(tExpression(2, 1) / qCos(theta2), tExpression(2, 2) / qCos(theta2));
        double sfin1 = qAtan2(tExpression(1, 0) / qCos(theta1), tExpression(0, 0) / qCos(theta1));
        double sfin2 = qAtan2(tExpression(1, 0) / qCos(theta2), tExpression(0, 0) / qCos(theta2));
    }
    else
    {
        double sfin = 0;
        if (tExpression(2,0) == -1)
        {
            double theta = qDegreesToRadians(180.0) / 2;
            double fin = sfin + qAtan2(tExpression(0, 1), tExpression(0, 2));
        }
        else
        {
            double theta = -qDegreesToRadians(180.0) / 2;
            double fin = -sfin + qAtan2(-tExpression(0, 1), -tExpression(0, 2));
        }
    }
}

void RobotControlImpl::PExpressionToTExpression(const SpFlangeExpression &pExpression,
    QMatrix4x4 &tExpression/*,
    QStringList &strExpression*/)
{
    double x = pExpression.GetX();
    double y = pExpression.GetY();
    double z = pExpression.GetZ();
    double r = qDegreesToRadians(pExpression.GetA());
    double b = qDegreesToRadians(pExpression.GetB());
    double a = qDegreesToRadians(pExpression.GetC());

    QVector4D rows[3];

    rows[0] = QVector4D(qCos(a)*qCos(b),
        qCos(a)*qSin(b)*qSin(r) - qSin(a)*qCos(r),
        qCos(a)*qSin(b)*qCos(r) + qSin(a)*qSin(r),
        x);

    rows[1] = QVector4D(qSin(a)*qCos(b),
        qSin(a)*qSin(b)*qSin(r) + qCos(a)*qCos(r),
        qSin(a)*qSin(b)*qCos(r) - qCos(a)*qSin(r),
        y);

    rows[2] = QVector4D(-qSin(b),
        qCos(b)*qSin(r),
        qCos(b)*qCos(r),
        z);

    for (int i = 0; i < 3; i++)
    {
        tExpression.setRow(i, rows[i]);
    }

    //QStringList pointList;
    //pointList.append(QString::number(tExpression(0, 3), 'f', 5));
    //pointList.append(QString::number(tExpression(1, 3), 'f', 5));
    //pointList.append(QString::number(tExpression(2, 3), 'f', 5));
    //pointList.append(QString::number(tExpression(0, 1), 'f', 5));
    //pointList.append(QString::number(tExpression(1, 1), 'f', 5));
    //pointList.append(QString::number(tExpression(2, 1), 'f', 5));
    //pointList.append(QString::number(tExpression(0, 2), 'f', 5));
    //pointList.append(QString::number(tExpression(1, 2), 'f', 5));
    //pointList.append(QString::number(tExpression(2, 2), 'f', 5));
    ////TODO:提常量
    //strExpression = pointList;
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

double RobotControlImpl::Approach(const QVector3D& ndiPosition, double initStep, double minStep, double relaxFactor, int axis)
{
    double step = initStep;
    double metric = INT_MAX;
    bool greater = true;
    while (step > minStep)
    {
        global_position_sync = false;
        double delta = (greater ? -1 : 1)*step;
        switch (axis)
        {
        case 0:
            SlotMoveX(delta, 2);
        	break;
        case 1:
            SlotMoveY(delta, 2);
            break;
        case 2:
            SlotMoveZ(delta, 2);
            break;
        default:
            break;
        }
        while (!global_position_sync) {}

       // qDebug() << "Current NDI Position: " << global_ndi_pos;
        double cmetric = qAbs(ndiPosition.distanceToPoint(global_ndi_pos));

        if (cmetric > metric)
        {
            greater = !greater;
            step *= relaxFactor;
            /*if (step > 10*minStep)
            {
                step *= relaxFactor;
            }
            else
            {
                step = qAbs(step - minStep);
            }*/
        }
        metric = cmetric;
        //qDebug() << "Step: " << step;
    }
    return metric;
}

void RobotControlImpl::SlotApproachToNDIPosition(const QVector3D& ndiPosition, double acceptError, double maxStep, double minStep, double relaxFactor)
{
    qDebug() << "SlotApproachToNDIPosition";
    qDebug() << "Target NDI Position: " << ndiPosition;

    double moveSpeed = 1.0;
    bool greater = true;
    double relax = relaxFactor;
    int times = 0;
    double metric = INT_MAX;
    double p1, p2, p3;
    double step = qAbs(ndiPosition.distanceToPoint(global_ndi_pos));

    while (metric>acceptError)
    {
        double cmetric = 0.0;
        if (metric > 1.0)
        {
            qDebug() << "Approach X!";
            cmetric = Approach(ndiPosition, qAbs(ndiPosition.distanceToPoint(global_ndi_pos)), minStep, relaxFactor, 0);

            qDebug() << "Approach Y!";
            cmetric = Approach(ndiPosition, cmetric, minStep, relaxFactor, 1);

            qDebug() << "Approach Z!";
            cmetric = Approach(ndiPosition, cmetric, minStep, relaxFactor, 2);
        }
        else
        {
            qDebug() << "Approach X!";
            cmetric = Approach(ndiPosition, minStep * 10, minStep*0.1, relaxFactor, 0);

            qDebug() << "Approach Y!";
            cmetric = Approach(ndiPosition, minStep * 10, minStep*0.1, relaxFactor, 1);

            qDebug() << "Approach Z!";
            cmetric = Approach(ndiPosition, minStep * 10, minStep*0.1, relaxFactor, 2);
        }

            metric = cmetric;

            qDebug() << "Current Metric: " << cmetric;

        
    }

    qDebug() << "Approach Target NDI Position Finished!";
    qDebug() << "Target: " << ndiPosition;
    qDebug() << "Current: " << global_ndi_pos;
    qDebug() << "Distance: " << ndiPosition.distanceToPoint(global_ndi_pos);
    m_robotController->Pause();


}


void RobotControlImpl::SlotAutoCaculateOffset(const QVector3D& ndiPosition, double acceptError, double maxStep, double minStep, double relaxFactor)
{
    SpFlangeExpression initExpression = m_robotController->GetFlangePosition();
    for (int i = -2; i <= 2; i++)
    {
        SpFlangeExpression expression = initExpression;
        expression.SetA(initExpression.GetA() + 20.0*i);
        qDebug() << "Move To Expression: " << expression.GetExpression();
        SlotMoveToExpression(expression);
        while (!global_position_sync) {}

        SlotApproachToNDIPosition(ndiPosition, acceptError,maxStep,minStep,relaxFactor);

        emit SignalApproachFinished();
    }

    for (int j = -2; j <= 2; j++)
    {
        SpFlangeExpression expression = initExpression;
        expression.SetB(initExpression.GetB() + 20.0*j);
        qDebug() << "Move To Expression: " << expression.GetExpression();
        SlotMoveToExpression(expression);
        while (!global_position_sync) {}

        SlotApproachToNDIPosition(ndiPosition, acceptError, maxStep, minStep, relaxFactor);

        emit SignalApproachFinished();
    }

    for (int k = -2; k <= 2; k++)
    {
        SpFlangeExpression expression = initExpression;
        expression.SetC(initExpression.GetC() + 20.0*k);
        qDebug() << "Move To Expression: " << expression.GetExpression();
        SlotMoveToExpression(expression);
        while (!global_position_sync) {}

        SlotApproachToNDIPosition(ndiPosition, acceptError, maxStep, minStep, relaxFactor);

        emit SignalApproachFinished();
    }

    emit SignalAutoCaculateOffsetCompleted();
}


void RobotControlImpl::EmitRobotMoved()
{
    emit CurrentTExpressionChanged(CurrentTExpression());
    emit CurrentPExpressionChanged(m_robotController->GetFlangePosition());
    emit CurrentPositionChanged(CurrentPosition());
}

void RobotControlImpl::SlotMoveToPositionWithOffset(QVector3D position, QVector3D offset)
{
    SpFlangeExpression expression = m_robotController->GetFlangePosition();
    expression.SetPosition(position);
    QMatrix4x4 tExpression;
    PExpressionToTExpression(expression, tExpression);
    
    double x = position.x() - (tExpression(0, 0)*offset.x() + tExpression(0, 1)*offset.y() + tExpression(0, 2)*offset.z());
    double y = position.y() - (tExpression(1, 0)*offset.x() + tExpression(1, 1)*offset.y() + tExpression(1, 2)*offset.z());
    double z = position.z() - (tExpression(2, 0)*offset.x() + tExpression(2, 1)*offset.y() + tExpression(2, 2)*offset.z());

    qDebug() << "MoveToPositionWithOffset1：" << QVector3D(x, y, z);
    /*double modelOffsetX = tExpression(0, 0)*offset.x() + tExpression(0, 1)*offset.y() + tExpression(0, 2)*offset.z();
    double modelOffsetY = tExpression(1, 0)*offset.x() + tExpression(1, 1)*offset.y() + tExpression(1, 2)*offset.z();
    double modelOffsetZ = tExpression(2, 0)*offset.x() + tExpression(2, 1)*offset.y() + tExpression(2, 2)*offset.z();

    expression.SetPosition(position + QVector3D(modelOffsetX, modelOffsetY,modelOffsetZ));
    SlotMoveToExpression(expression);*/

    double tx = (global_temp_t_expression(0, 0) - tExpression(0, 0))*offset.x() +
        (global_temp_t_expression(0, 1) - tExpression(0, 1))*offset.y() +
        (global_temp_t_expression(0, 2) - tExpression(0, 2))*offset.z() + global_temp_t_expression(0, 3);
    double ty = (global_temp_t_expression(1, 0) - tExpression(1, 0))*offset.x() +
        (global_temp_t_expression(1, 1) - tExpression(1, 1))*offset.y() +
        (global_temp_t_expression(1, 2) - tExpression(1, 2))*offset.z() + global_temp_t_expression(1, 3);
    double tz = (global_temp_t_expression(2, 0) - tExpression(2, 0))*offset.x() +
        (global_temp_t_expression(2, 1) - tExpression(2, 1))*offset.y() +
        (global_temp_t_expression(2, 2) - tExpression(2, 2))*offset.z() + global_temp_t_expression(2, 3);

    qDebug() << "MoveToPositionWithOffset2：" << QVector3D(tx,ty,tz);

    expression.SetPosition(QVector3D(tx, ty, tz));
    SlotMoveToExpression(expression);
}