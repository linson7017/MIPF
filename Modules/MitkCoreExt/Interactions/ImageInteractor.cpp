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

#include <vtkMath.h>


ImageInteractor::ImageInteractor() :m_bDragging(false), m_originMatrix(NULL), m_bInitFlag(false)
{
    m_transform = vtkSmartPointer<vtkTransform>::New();
    m_transform->Identity();
}


ImageInteractor::~ImageInteractor()
{
}

vtkMatrix4x4* ImageInteractor::GetTransform()
{
    return m_transform->GetMatrix();
}

void ImageInteractor::SetTransform(vtkMatrix4x4* data)
{
    m_transform->GetMatrix()->DeepCopy(data);
    TransformChangedEvent.Send(m_transform->GetMatrix());
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
        m_originMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    }
    m_originMatrix->DeepCopy(GetDataNode()->GetData()->GetGeometry()->GetVtkMatrix());
    m_originCenter = GetDataNode()->GetData()->GetGeometry()->GetCenter();
    m_bInitFlag = true;
}

void ImageInteractor::RefreshDataGeometry()
{
    auto rm = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkMatrix4x4::Multiply4x4(m_transform->GetMatrix(), m_originMatrix, rm);
    if (GetDataNode() != NULL)
    {
        GetDataNode()->GetData()->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(rm);
        GetDataNode()->Modified();
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }

    Modified();
}

void ImageInteractor::Reset()
{
    m_transform->Identity();
    TransformChangedEvent.Send(m_transform->GetMatrix());
    RefreshDataGeometry();
}

void ImageInteractor::Translate(const vtkVector3d& translate)
{
    auto  invertMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkMatrix4x4::Invert(m_transform->GetMatrix(), invertMatrix);
    invertMatrix->SetElement(0, 3, 0.0);
    invertMatrix->SetElement(1, 3, 0.0);
    invertMatrix->SetElement(2, 3, 0.0);
    invertMatrix->SetElement(3, 3, 1.0);
    invertMatrix->SetElement(3, 0, 0.0);
    invertMatrix->SetElement(3, 1, 0.0);
    invertMatrix->SetElement(3, 2, 0.0);

    double ft[4] = { translate[0], translate[1] ,translate[2] ,1.0 };
    double* delta = invertMatrix->MultiplyDoublePoint(ft);

    m_transform->Translate(delta[0], delta[1], delta[2]);
    TransformChangedEvent.Send(m_transform->GetMatrix());
    RefreshDataGeometry();
}

void ImageInteractor::Rotate(double angle, const vtkVector3d& normal)
{
    m_transform->Translate(m_originCenter[0], m_originCenter[1], m_originCenter[2]);
    m_transform->RotateWXYZ(angle, normal.GetData());
    m_transform->Translate(-m_originCenter[0], -m_originCenter[1], -m_originCenter[2]);
    TransformChangedEvent.Send(m_transform->GetMatrix());
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
            Translate(vtkVector3d(dirVector.GetElement(0), dirVector.GetElement(1), dirVector.GetElement(2)));

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

            mitk::Vector3D normal = positionEvent->GetSender()->GetCurrentWorldGeometry2D()->GetNormal();
            mitk::Point3D imageCenter = GetDataNode()->GetData()->GetGeometry()->GetCenter();
            vtkVector3d start((m_LastPoint - imageCenter).GetDataPointer());
            vtkVector3d end((newPoint - imageCenter).GetDataPointer());
            float angle = vtkMath::DegreesFromRadians(acos(start.Normalized().Dot(end.Normalized())));
            Rotate(angle,vtkVector3d(start.Cross(end)));
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
