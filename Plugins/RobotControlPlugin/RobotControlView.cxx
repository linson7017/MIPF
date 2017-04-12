#include "RobotControlView.h"
#include "iqf_main.h"
#include "Res/R.h"

#include "RobotControlImpl.h"

#include "Utils/variant.h"

#include <QVector3D>

#include <QtWidgets>

bool global_robot_stop = false;
QVector3D global_ndi_pos;
QMatrix4x4 global_registration_matrix;

RobotControlView::RobotControlView(QF::IQF_Main* pMain) :PluginView(pMain)
{
    m_pMain->Attach(this);

    m_robotThread = new QThread;

    m_RobotControlImpl = new RobotControlImpl;
    m_RobotControlImpl->moveToThread(m_robotThread);
    connect(m_robotThread, &QThread::finished, m_robotThread, &QThread::deleteLater);

    
    connect(this, &RobotControlView::SignalInit, m_RobotControlImpl, &RobotControlImpl::SlotInit);
    connect(this, &RobotControlView::SignalStartDrag, m_RobotControlImpl, &RobotControlImpl::SlotStartDrag);
    connect(this, &RobotControlView::SignalStopDrag, m_RobotControlImpl, &RobotControlImpl::SlotStopDrag);
    connect(this, &RobotControlView::SignalMoveX, m_RobotControlImpl, &RobotControlImpl::SlotMoveX);
    connect(this, &RobotControlView::SignalMoveY, m_RobotControlImpl, &RobotControlImpl::SlotMoveY);
    connect(this, &RobotControlView::SignalMoveZ, m_RobotControlImpl, &RobotControlImpl::SlotMoveZ);
    connect(this, &RobotControlView::SignalMoveToPosition, m_RobotControlImpl, &RobotControlImpl::SlotMoveToPosition);
    connect(this, &RobotControlView::SignalApproachToNDIPosition, m_RobotControlImpl, &RobotControlImpl::SlotApproachToNDIPosition);
    connect(this, &RobotControlView::SignalNDIErrorEstimate, m_RobotControlImpl, &RobotControlImpl::SlotNDIErrorEstimate);

    connect(m_RobotControlImpl, &RobotControlImpl::CurrentPositionChanged, this, &RobotControlView::SlotCurrentPositionChanged);
    connect(m_RobotControlImpl, &RobotControlImpl::SignalNDIErrorEstimated, this, &RobotControlView::SlotNDIErrorEstimated);

    m_robotThread->start();
}

void RobotControlView::SlotCurrentPositionChanged(QVector3D position)
{
    qDebug() << "Current Position Changed:" << position;
    m_pMain->SendMessageQf("Robot.Move", 0, &position);
    m_currentRobotPosition = position;
}

void RobotControlView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "main.Init") == 0)
    {
        SignalInit();
    }
    else if (strcmp(szMessage, "main.StartDrag") == 0)
    {
        SignalStartDrag();
    }
    else if (strcmp(szMessage, "main.StopDrag") == 0)
    {
        SignalStopDrag();
    }
    else if (strcmp(szMessage, "main.MoveX") == 0)
    {
        VarientMap vmp = *(VarientMap*)pValue;
        variant v = variant::GetVariant(vmp, "text");
        QString step = v.getString();
        qDebug() << "Move X Step: " << step;
        SignalMoveX(step.toDouble(), 1);
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
        SignalMoveZ(step.toDouble(), 1);
    }
    else if (strcmp(szMessage, "main.MoveToPosition") == 0)
    {
        QVector3D* positionValue = (QVector3D*)(pValue);
        qDebug() << "Move To Position: " << *positionValue;
        SignalMoveToPosition(positionValue->x(),positionValue->y(),positionValue->z());
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
        SignalMoveY(20,1);
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
       // qDebug() << "NDI Position Changed: " << global_ndi_pos;
    }
    else if (strcmp(szMessage, "main.RecordTargetPosition") == 0)
    {
        m_targetNdiPosition = global_ndi_pos;
        m_targetRobotPositionAppro = m_currentRobotPosition;
    }
    else if (strcmp(szMessage, "main.ApproachToTargetPosition") == 0)
    {
        QLineEdit* lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.MaxStep");
        double maxStep = lineEdit->text().toDouble();
        lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.MinStep");
        double minStep = lineEdit->text().toDouble();
        lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.RelaxFactor");
        double relaxFactor = lineEdit->text().toDouble();


        qDebug() << "Target NDI Position: " << m_targetNdiPosition;
        emit SignalApproachToNDIPosition(m_targetNdiPosition, m_targetRobotPositionAppro, maxStep, minStep, relaxFactor);
    }
    else if (strcmp(szMessage, "main.NDIErrorEstimate") == 0)
    {
        emit SignalNDIErrorEstimate(m_targetNdiPosition);
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