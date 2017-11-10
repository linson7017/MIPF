#include "ImageInteractor.h"
#include "mitkInternalEvent.h"
#include "mitkMouseMoveEvent.h"
#include "mitkRenderingManager.h"
#include "mitkEventStateMachine.h"
//
#include "mitkBaseRenderer.h"
#include "mitkDispatcher.h"
#include <mitkPropertyList.h>
#include <mitkCameraController.h>


ImageInteractor::ImageInteractor() :m_bDragging(false), m_originMatrix(NULL), m_bInitFlag(false)
{
    m_transformMatrix.setToIdentity();
}


ImageInteractor::~ImageInteractor()
{
}

void ImageInteractor::SetDataNode(mitk::DataNode *dataNode)
{
    mitk::DataInteractor::SetDataNode(dataNode);
    Init();
}

void ImageInteractor::Init()
{
    if (!m_originMatrix)
    {
        m_originMatrix = vtkMatrix4x4::New();
    }
    m_originMatrix->DeepCopy(GetDataNode()->GetData()->GetGeometry()->GetVtkMatrix());
    m_originCenter = GetDataNode()->GetData()->GetGeometry()->GetCenter();
    m_bInitFlag = true;
}

void ImageInteractor::SetTransformMatrix(const QMatrix4x4& matrix)
{
    m_transformMatrix = matrix;
}

QMatrix4x4 ImageInteractor::GetTransformMatrix()
{
    return m_transformMatrix;
}

void ImageInteractor::RefreshDataGeometry()
{
    vtkMatrix4x4* transform = vtkMatrix4x4::New();
    vtkMatrix4x4* rm = vtkMatrix4x4::New();
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            transform->SetElement(i, j, m_transformMatrix(i, j));
        }
    }
    vtkMatrix4x4::Multiply4x4(transform, m_originMatrix, rm);
    if (GetDataNode() != NULL)
    {
        GetDataNode()->GetData()->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(rm);
        GetDataNode()->Modified();
        //RequestRenderWindowUpdate();
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }

    Modified();
}

void ImageInteractor::Reset()
{
    m_transformMatrix.setToIdentity();
    RefreshDataGeometry();
}

void ImageInteractor::Translate(const QVector3D& translate)
{
    QMatrix4x4 invertMatrix = m_transformMatrix;
    invertMatrix = invertMatrix.inverted();
    invertMatrix(0, 3) = 0.0;
    invertMatrix(1, 3) = 0.0;
    invertMatrix(2, 3) = 0.0;
    invertMatrix(3, 3) = 1.0;
    invertMatrix(3, 0) = 0.0;
    invertMatrix(3, 1) = 0.0;
    invertMatrix(3, 2) = 0.0;
    QVector3D delta = invertMatrix*translate;
    m_transformMatrix.translate(delta);
    RefreshDataGeometry();
}

void ImageInteractor::Rotate(double angle, const QVector3D& normal)
{
    m_transformMatrix.translate(m_originCenter[0], m_originCenter[1], m_originCenter[2]);
    m_transformMatrix.rotate(angle, normal);
    m_transformMatrix.translate(-m_originCenter[0], -m_originCenter[1], -m_originCenter[2]);
    RefreshDataGeometry();
}

void ImageInteractor::ConnectActionsAndFunctions()
{
    // connect the action and condition names of the state machine pattern with function within
    // this DataInteractor
    CONNECT_FUNCTION("selectImage", SelectImage);
    CONNECT_FUNCTION("initMove", InitMove);
    CONNECT_FUNCTION("moveImage", MoveImage);
    CONNECT_FUNCTION("rotateImage", RotateImage);
    CONNECT_FUNCTION("finishMovement", FinishMove);
}

void ImageInteractor::SelectImage(mitk::StateMachineAction *, mitk::InteractionEvent *)
{

}

void ImageInteractor::InitMove(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    mitk::InteractionPositionEvent *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
    if (positionEvent == NULL)
        return;
    m_LastPoint = positionEvent->GetPositionInWorld();
    m_bDragging = true;
}

void ImageInteractor::MoveImage(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    if (m_bDragging&&m_bInitFlag)
    {
        mitk::InteractionPositionEvent *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
        if (positionEvent != NULL)
        {
            mitk::Point3D newPoint, resultPoint;
            newPoint = positionEvent->GetPositionInWorld();
            mitk::Vector3D dirVector = newPoint - m_LastPoint;
            Translate(QVector3D(dirVector.GetElement(0), dirVector.GetElement(1), dirVector.GetElement(2)));

            m_LastPoint = newPoint;
        }
    }
}

void ImageInteractor::RotateImage(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    if (m_bDragging&&m_bInitFlag)
    {
        mitk::InteractionPositionEvent *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
        if (positionEvent != NULL)
        {
            mitk::Point3D newPoint, resultPoint;
            newPoint = positionEvent->GetPositionInWorld();
            mitk::Vector3D dirVector = newPoint - m_LastPoint;

            // m_SumVec = m_SumVec + dirVector;
            double angle = dirVector.Length;


            mitk::Vector3D normal = positionEvent->GetSender()->GetCurrentWorldGeometry2D()->GetNormal();
            QVector3D qStart(m_LastPoint.GetElement(0), m_LastPoint.GetElement(1), m_LastPoint.GetElement(2));
            QVector3D qEnd(newPoint.GetElement(0), newPoint.GetElement(1), newPoint.GetElement(2));
            QVector3D qNormal(normal.GetElement(0), normal.GetElement(1), normal.GetElement(2));
            qNormal.normalize();

            QVector3D qDirection = QVector3D::crossProduct(qStart, qEnd);
            float direction = QVector3D::dotProduct(qDirection.normalized(), qNormal.normalized());
            direction /= fabs(direction);
            Rotate(direction*angle, qNormal);


            m_LastPoint = newPoint;

        }
    }
}

void ImageInteractor::FinishMove(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    if (!m_bInitFlag)
    {
        return;
    }
    m_bDragging = false;
}
