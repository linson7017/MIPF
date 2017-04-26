#include "RobotControlView.h"
#include "iqf_main.h"
#include "Res/R.h"

#include "RobotControlImpl.h"

#include "Utils/variant.h"

#include <QVector3D>

#include <QtWidgets>

bool global_force_stop = false;

bool global_robot_stop = false;
bool global_position_sync = false;
bool global_approach_finished = false;
QVector3D global_ndi_pos;
QMatrix4x4 global_registration_matrix;
QMatrix4x4 global_t_expression;
SpFlangeExpression global_p_expression;

QMatrix4x4 global_temp_t_expression;

RobotControlView::RobotControlView(QF::IQF_Main* pMain) :PluginView(pMain)
{
    qRegisterMetaType<SpFlangeExpression>("SpFlangeExpression");

    m_pMain->Attach(this);

    m_robotThread = new QThread;

    m_RobotControlImpl = new RobotControlImpl;
    m_RobotControlImpl->moveToThread(m_robotThread);
    connect(m_robotThread, &QThread::finished, m_robotThread, &QThread::deleteLater);

    
    connect(this, &RobotControlView::SignalInit, m_RobotControlImpl, &RobotControlImpl::SlotInit);
    connect(this, &RobotControlView::SignalStop, m_RobotControlImpl, &RobotControlImpl::SlotStop);
    connect(this, &RobotControlView::SignalBackToHome, m_RobotControlImpl, &RobotControlImpl::SlotBackToHome);
    connect(this, &RobotControlView::SignalStartDrag, m_RobotControlImpl, &RobotControlImpl::SlotStartDrag);
    connect(this, &RobotControlView::SignalStopDrag, m_RobotControlImpl, &RobotControlImpl::SlotStopDrag);
    connect(this, &RobotControlView::SignalMoveX, m_RobotControlImpl, &RobotControlImpl::SlotMoveX);
    connect(this, &RobotControlView::SignalMoveY, m_RobotControlImpl, &RobotControlImpl::SlotMoveY);
    connect(this, &RobotControlView::SignalMoveZ, m_RobotControlImpl, &RobotControlImpl::SlotMoveZ);
    connect(this, &RobotControlView::SignalMoveToPosition, m_RobotControlImpl, &RobotControlImpl::SlotMoveToPosition);
    connect(this, &RobotControlView::SignalMoveToExpression, m_RobotControlImpl, &RobotControlImpl::SlotMoveToExpression);

    connect(this, &RobotControlView::SignalApproachToNDIPosition, m_RobotControlImpl, &RobotControlImpl::SlotApproachToNDIPosition);
    connect(this, &RobotControlView::SignalAutoCaculateOffset, m_RobotControlImpl, &RobotControlImpl::SlotAutoCaculateOffset);
    connect(this, &RobotControlView::SignalMoveToPositionWithOffset, m_RobotControlImpl, &RobotControlImpl::SlotMoveToPositionWithOffset);

    connect(this, &RobotControlView::SignalNDIErrorEstimate, m_RobotControlImpl, &RobotControlImpl::SlotNDIErrorEstimate);

    connect(m_RobotControlImpl, &RobotControlImpl::CurrentPositionChanged, this, &RobotControlView::SlotCurrentPositionChanged);
    connect(m_RobotControlImpl, &RobotControlImpl::CurrentTExpressionChanged, this, &RobotControlView::SlotCurrentTExpressionChanged);
    connect(m_RobotControlImpl, &RobotControlImpl::CurrentPExpressionChanged, this, &RobotControlView::SlotCurrentPExpressionChanged);
    connect(m_RobotControlImpl, &RobotControlImpl::SignalNDIErrorEstimated, this, &RobotControlView::SlotNDIErrorEstimated);
    connect(m_RobotControlImpl, &RobotControlImpl::SignalApproachFinished, this, &RobotControlView::SlotApproachFinished);
    connect(m_RobotControlImpl, &RobotControlImpl::SignalAutoCaculateOffsetCompleted, this, &RobotControlView::SlotAutoCaculateOffsetCompleted);

    m_robotThread->start();
}

void RobotControlView::SlotCurrentPositionChanged(QVector3D position)
{
  //  qDebug() << "Current Position Changed:" << position;
    m_pMain->SendMessageQf("Robot.Move", 0, &position);
    m_currentRobotPosition = position;
}

void RobotControlView::SlotCurrentTExpressionChanged(QMatrix4x4 tExpression)
{
   // qDebug() << "Current T Expression Changed:" << tExpression;
    global_t_expression = tExpression;
}

void RobotControlView::SlotCurrentPExpressionChanged(SpFlangeExpression pExpression)
{
  //  qDebug() << "Current P Expression Changed:" << pExpression.GetExpression();
    global_p_expression = SpFlangeExpression(pExpression.GetX(),pExpression.GetY(), pExpression.GetZ()
        , pExpression.GetA(), pExpression.GetB(), pExpression.GetC());
}

void RobotControlView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "main.Init") == 0)
    {
        emit SignalInit();
    }
    else if (strcmp(szMessage, "main.Stop") == 0)
    {
        global_force_stop = true;
        emit SignalStop();
    }
    else if (strcmp(szMessage, "main.BackToHome") == 0)
    {
        emit SignalBackToHome();
    }
    else if (strcmp(szMessage, "main.StartDrag") == 0)
    {
        emit SignalStartDrag();
    }
    else if (strcmp(szMessage, "main.StopDrag") == 0)
    {
        emit SignalStopDrag();
    }
    else if (strcmp(szMessage, "main.MoveX") == 0)
    {
        VarientMap vmp = *(VarientMap*)pValue;
        variant v = variant::GetVariant(vmp, "text");
        QString step = v.getString();
        qDebug() << "Move X Step: " << step;
        emit SignalMoveX(step.toDouble(), 1);
    }
    else if (strcmp(szMessage, "main.MoveY") == 0)
    {
        VarientMap vmp = *(VarientMap*)pValue;
        variant v = variant::GetVariant(vmp, "text");
        QString step = v.getString();
        SignalMoveY(step.toDouble(), 1);
    }
    else if (strcmp(szMessage, "main.MoveZ") == 0)
    {
        VarientMap vmp = *(VarientMap*)pValue;
        variant v = variant::GetVariant(vmp, "text");
        QString step = v.getString();
        emit SignalMoveZ(step.toDouble(), 1);
    }
    else if (strcmp(szMessage, "main.MoveToPosition") == 0)
    {
        QVector3D* positionValue = (QVector3D*)(pValue);
        qDebug() << "Move To Position: " << *positionValue;
        emit SignalMoveToPosition(positionValue->x(),positionValue->y(),positionValue->z());
    }
    else if (strcmp(szMessage, "main.GetFlangePosition") == 0)
    {
        QLineEdit* lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.FlangePosition");
        if (lineEdit)
        {
            QVector3D position = m_RobotControlImpl->CurrentPosition();
            qDebug() << "Current Position: " << position;
            QString positionStr = QString("%1, %2, %3").arg(position.x()).arg(position.y()).arg(position.z());
            lineEdit->setText(positionStr);
        }
    }
    else if(strcmp(szMessage, "main.MattesRegistration") == 0)
    {
        SignalMoveX(20,1);
        while (global_robot_stop)
        {
            m_pMain->SendMessageQf("main.GetNeedlePosition", 0, NULL);
        }
        emit SignalMoveY(20,1);
        while (global_robot_stop)
        {
            m_pMain->SendMessageQf("main.GetNeedlePosition", 0, NULL);
        }  
        SignalMoveZ(20,1);
        while (global_robot_stop)
        {
            m_pMain->SendMessageQf("main.GetNeedlePosition", 0, NULL);
            m_pMain->SendMessageQf("main.Registrate", 0, NULL);
        }          
    }
    else if (strcmp(szMessage, "main.RegistrationMatrixCaculated") == 0)
    {
        QMatrix4x4* m = (QMatrix4x4*)pValue;
        global_registration_matrix = *m;
    }
    else if (strcmp(szMessage, "main.NDIPositionChanged") == 0)
    {
    
        QVector3D* m = (QVector3D*)pValue;
        global_ndi_pos = *m;
        QLineEdit* lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.CurrentNDIPosition");
        if (lineEdit)
        {
            QString text = QString("%1, %2 ,%3").arg(global_ndi_pos.x()).arg(global_ndi_pos.y()).arg(global_ndi_pos.z());
            lineEdit->setText(text);
        }
        lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.CurrentRobotPosition");
        if (lineEdit)
        {
            QString text = QString("%1, %2 ,%3").arg(m_currentRobotPosition.x()).arg(m_currentRobotPosition.y()).arg(m_currentRobotPosition.z());
            lineEdit->setText(text);
        }
        global_position_sync = true;
    }
    else if (strcmp(szMessage, "main.RecordTargetPosition") == 0)
    {
        RecordTargetPosition();
    }
    else if (strcmp(szMessage, "main.CaculateTargetRobotPosition") == 0)
    {
        QLineEdit* lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.CaculatedTargetRobotPosition");
        if (lineEdit)
        {
            QVector3D caculatedTargetRobotPosition = global_registration_matrix.inverted()*m_targetNdiPosition;
            QString text = QString("%1, %2 ,%3").arg(caculatedTargetRobotPosition.x()).arg(caculatedTargetRobotPosition.y()).arg(caculatedTargetRobotPosition.z());
            lineEdit->setText(text);
        }
    }
    else if (strcmp(szMessage, "main.ApproachToTargetPosition") == 0)
    {
        ApproachToTargetPosition();
    }
    else if (strcmp(szMessage, "main.NDIErrorEstimate") == 0)
    {
        emit SignalNDIErrorEstimate(m_targetNdiPosition);
    }
    else if (strcmp(szMessage, "main.AddSpherePoint") == 0)
    {
        AddSpherePoint();
        
    }
    else if (strcmp(szMessage, "main.CaculateOffset") == 0)
    {
        CaculateOffset();
    }
    else if (strcmp(szMessage, "main.AutoCaculateOffset") == 0)
    {
        AutoCaculateOffset();
    }
    else if (strcmp(szMessage, "main.RecordRobotPositionWithOffset") == 0)
    {
        QLineEdit* lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.NDICenterXInRobot");
        double offsetX = lineEdit->text().toDouble();
        lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.NDICenterYInRobot");
        double offsetY = lineEdit->text().toDouble();
        lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.NDICenterZInRobot");
        double offsetZ = lineEdit->text().toDouble();

        QMatrix4x4 tExpression   =  global_t_expression;

        global_temp_t_expression =  global_t_expression;

        double modelOffsetX = tExpression(0, 0)*offsetX + tExpression(0, 1)*offsetY + tExpression(0, 2)*offsetZ;
        double modelOffsetY = tExpression(1, 0)*offsetX + tExpression(1, 1)*offsetY + tExpression(1, 2)*offsetZ;
        double modelOffsetZ = tExpression(2, 0)*offsetX + tExpression(2, 1)*offsetY + tExpression(2, 2)*offsetZ;

        lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.RobotPointX");
        lineEdit->setText(QString("%1").arg(m_currentRobotPosition.x() + modelOffsetX));
        lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.RobotPointY");
        lineEdit->setText(QString("%1").arg(m_currentRobotPosition.y() + modelOffsetY));
        lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.RobotPointZ");
        lineEdit->setText(QString("%1").arg(m_currentRobotPosition.z() + modelOffsetZ));

    }
    else if (strcmp(szMessage, "main.MoveToRobotPositionAfterCalibrate") == 0)
    {
        QLineEdit* lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.RobotPointX");
        double x = lineEdit->text().toDouble();
        lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.RobotPointY");
        double y = lineEdit->text().toDouble();
        lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.RobotPointZ");
        double z = lineEdit->text().toDouble();

        lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.NDICenterXInRobot");
        double offsetX = lineEdit->text().toDouble();
        lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.NDICenterYInRobot");
        double offsetY = lineEdit->text().toDouble();
        lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.NDICenterZInRobot");
        double offsetZ = lineEdit->text().toDouble();

        emit SignalMoveToPositionWithOffset(QVector3D(x, y, z), QVector3D(offsetX, offsetY, offsetZ));
    }
}


void RobotControlView::AutoCaculateOffset()
{
    RecordTargetPosition();
    AddSpherePoint();

    QLineEdit* lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.MaxStep");
    double maxStep = lineEdit->text().toDouble();
    lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.MinStep");
    double minStep = lineEdit->text().toDouble();
    lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.RelaxFactor");
    double relaxFactor = lineEdit->text().toDouble();
    lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.AcceptError");
    double acceptError = lineEdit->text().toDouble();

    emit SignalAutoCaculateOffset(m_targetNdiPosition, acceptError, maxStep, minStep, relaxFactor);
}

void RobotControlView::RecordTargetPosition()
{
    qDebug() << "Record Target Position:" << global_ndi_pos;
    m_targetNdiPosition = global_ndi_pos;
    m_targetRobotPositionAppro = m_currentRobotPosition;
    QLineEdit* lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.TargetRobotPosition");
    if (lineEdit)
    {
        QString text = QString("%1, %2 ,%3").arg(m_currentRobotPosition.x()).arg(m_currentRobotPosition.y()).arg(m_currentRobotPosition.z());
        lineEdit->setText(text);
    }
}
void RobotControlView::AddSpherePoint()
{
    QListWidget* listWidget = (QListWidget*)m_pR->getObjectFromGlobalMap("main.RobotSpherePointList");
    if (listWidget)
    {
        double error = global_ndi_pos.distanceToPoint(m_targetNdiPosition);

        //m_sphere.InsertPoint(QVector3D(m_currentRobotPosition));
        QMatrix4x4 m = global_t_expression.inverted();
        m_sphere.InsertPoint(m.column(3).toVector3D());
        QString positionStr = QString("%1, %2, %3(error:%4)").arg(m.column(3).x()).arg(m.column(3).y()).arg(m.column(3).z()).arg(error);
        listWidget->addItem(positionStr);
        qDebug() << "Add Sphere Point:" << positionStr;
    }
}
void RobotControlView::ApproachToTargetPosition()
{
    QLineEdit* lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.MaxStep");
    double maxStep = lineEdit->text().toDouble();
    lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.MinStep");
    double minStep = lineEdit->text().toDouble();
    lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.RelaxFactor");
    double relaxFactor = lineEdit->text().toDouble();
    lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.AcceptError");
    double acceptError = lineEdit->text().toDouble();


    qDebug() << "Target NDI Position: " << m_targetNdiPosition;
    qDebug() << "Parameter: " << maxStep<<", "<< minStep<<", "<< relaxFactor;
    emit SignalApproachToNDIPosition(m_targetNdiPosition, acceptError, maxStep, minStep, relaxFactor,true);
}
void RobotControlView::CaculateOffset()
{
    QVector3D center;
    double radius;
    m_sphere.Fit(center, radius);
    QLineEdit* lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.NDICenterXInRobot");
    if (lineEdit)
    {
        lineEdit->setText(QString("%1").arg(center.x()));
    }
    lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.NDICenterYInRobot");
    if (lineEdit)
    {
        lineEdit->setText(QString("%1").arg(center.y()));
    }
    lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.NDICenterZInRobot");
    if (lineEdit)
    {
        lineEdit->setText(QString("%1").arg(center.z()));
    }
    lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.NDIRadius");
    if (lineEdit)
    {
        lineEdit->setText(QString("%1").arg(center.length()));
    }
    for (int i = 0; i < m_sphere.GetPointSize(); i++)
    {
        qDebug() << "Point " << i << " Distance To Center: " << m_sphere.GetPoint(i).distanceToPoint(center);
        qDebug() << "Offset " << i << " : " << center - m_sphere.GetPoint(i);
    }
}


void RobotControlView::SlotNDIErrorEstimated(double error)
{
    QLineEdit* lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.NDIError");
    if (lineEdit)
    {
        QString s;
        lineEdit->setText(s.setNum(error));
    }
}

void RobotControlView::SlotApproachFinished()
{
    AddSpherePoint();
   // global_approach_finished = true;
}

void RobotControlView::SlotAutoCaculateOffsetCompleted()
{
    CaculateOffset();
}