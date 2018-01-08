#include "FreehandCutInteractor.h"
#include "mitkInternalEvent.h"
#include "mitkMouseMoveEvent.h"
#include "mitkRenderingManager.h"
#include "mitkEventStateMachine.h"
//
#include "mitkBaseRenderer.h"
#include "mitkDispatcher.h"
#include <mitkPropertyList.h>
#include <mitkCameraController.h>


//vtk
#include <vtkPolyLine.h>  
#include <vtkCylinderSource.h>

#include <vtkImplicitSelectionLoop.h>
#include <vtkClipPolyData.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

#include "Rendering/FreehandSurfaceCutMapper3D.h"

#include <limits>
#include <iostream>
#include <cmath>

#include "CutImplementation.h"

FreehandCutInteractor::FreehandCutInteractor() :m_bDrawing(false), m_bInitFlag(false), m_pCurveNode(nullptr), m_pDataStorage(nullptr), m_bModify(false)
{

}


FreehandCutInteractor::~FreehandCutInteractor()
{
    if (m_implementation!=nullptr)
    {
        m_implementation->Release();
        m_implementation = nullptr;
    }
}

void FreehandCutInteractor::SetDataNode(mitk::DataNode *dataNode)
{
    mitk::DataInteractor::SetDataNode(dataNode);
    Init();
}

void FreehandCutInteractor::SetRenderer(vtkRenderer* renderer)
{
    m_pRenderer = renderer;
}

void FreehandCutInteractor::Start()
{
    if (m_pRenderer)
    {
        m_pRenderer->GetActiveCamera()->ParallelProjectionOn();
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void FreehandCutInteractor::End()
{
    if (m_pRenderer)
    {
        m_pRenderer->GetActiveCamera()->ParallelProjectionOff();
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void FreehandCutInteractor::Init()
{
    if (m_implementation&&m_pDataStorage)
    {
        m_bInitFlag = true;
        m_pCurveNode = mitk::DataNode::New();
        m_pCurveNode->SetColor(0.0, 1.0, 0.0);
        m_pCurveNode->SetProperty("helper object", mitk::BoolProperty::New(true));
        m_pCurveNode->SetFloatProperty("line width", 2.0);


        mitk::FreehandSurfaceCutMapper3D::Pointer mapper = mitk::FreehandSurfaceCutMapper3D::New();
        m_pCurveNode->SetMapper(mitk::BaseRenderer::Standard3D, mapper);
        m_pCurveNode->SetStringProperty("3d mapper type", "freehand surface cut");

        mitk::Surface::Pointer curve = mitk::Surface::New();
        m_pCurveData = vtkSmartPointer<vtkPolyData>::New();
        curve->SetVtkPolyData(m_pCurveData);
        m_pCurveNode->SetData(curve);
        m_pDataStorage->Add(m_pCurveNode);

        m_pCurvePoints = vtkSmartPointer<vtkPoints>::New();
        m_pCurvePointsBeforeModify = vtkSmartPointer<vtkPoints>::New();

        m_implementation->SetDataNode(GetDataNode());
        m_implementation->Init();
    }
}

void FreehandCutInteractor::ConnectActionsAndFunctions()
{
    // connect the action and condition names of the state machine pattern with function within
    // this DataInteractor
    CONNECT_FUNCTION("initMove", InitMove);
    CONNECT_FUNCTION("draw", Draw);
    CONNECT_FUNCTION("modify", Modify);
    CONNECT_FUNCTION("finished", Finished);
    CONNECT_FUNCTION("undo", Undo);
    CONNECT_FUNCTION("redo", Redo);
    CONNECT_FUNCTION("initModify", InitModify);

}

void FreehandCutInteractor::InitMove(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    mitk::InteractionPositionEvent *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
    if (positionEvent == NULL)
        return;
    m_LastPoint = positionEvent->GetPositionInWorld();

    m_bDrawing = true;
    m_pCurvePoints->Reset();
    m_pCurveData->Reset();
}

void FreehandCutInteractor::InitModify(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    MITK_INFO << "Begin Modify";
    mitk::InteractionPositionEvent *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
    if (positionEvent == NULL)
        return;
    m_LastPoint = positionEvent->GetPositionInWorld();

    ProjectPointOnPlane(m_LastPoint, positionEvent->GetSender()->GetVtkRenderer()->GetActiveCamera(), m_LastPoint);
    double minDistance = DistanceBetweenPointAndPoints(m_LastPoint, m_pCurvePoints.GetPointer());
    if (minDistance <= 5)
    {
        m_bModify = true;
        Undo();
    }
    else
    {
        m_bModify = false;
    }
}

void FreehandCutInteractor::Modify(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    if (m_bModify&&m_bInitFlag)
    {
        m_pCurveNode->SetColor(1, 0, 0);
        mitk::InteractionPositionEvent *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
        if (positionEvent != NULL)
        {
            mitk::Point3D newPoint;
            newPoint = positionEvent->GetPositionInWorld();
            ProjectPointOnPlane(newPoint, positionEvent->GetSender()->GetVtkRenderer()->GetActiveCamera(), newPoint);
            mitk::Vector3D dirVector = newPoint - m_LastPoint;
            double maxDistance = dirVector.GetVnlVector().two_norm();
            dirVector.Normalize();
            for (int i = 0; i < m_pCurvePointsBeforeModify.GetPointer()->GetNumberOfPoints(); ++i)
            {
                mitk::Point3D tempPoint = m_pCurvePointsBeforeModify.GetPointer()->GetPoint(i);

                double DistanceNeedToMove = maxDistance * exp(-m_vDistanceBetweenPointsAndPoint[i] * m_vDistanceBetweenPointsAndPoint[i] / 555);
                tempPoint = tempPoint + dirVector * DistanceNeedToMove;
                m_pCurvePoints->SetPoint(i, tempPoint[0], tempPoint[1], tempPoint[2]);
            }
            RefreshCurve();
            mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
        }
    }
}

void FreehandCutInteractor::Draw(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    if (m_bDrawing&&m_bInitFlag)
    {
        mitk::InteractionPositionEvent *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
        if (positionEvent != NULL)
        {

            mitk::Point3D newPoint = positionEvent->GetPositionInWorld();

            ProjectPointOnPlane(newPoint, positionEvent->GetSender()->GetVtkRenderer()->GetActiveCamera(), newPoint);

            m_pCurvePoints->InsertNextPoint(newPoint.GetDataPointer());
            RefreshCurve();

            mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());

        }
    }
}

void FreehandCutInteractor::RefreshCurve()
{
    vtkSmartPointer<vtkPolyLine> polyLine =
        vtkSmartPointer<vtkPolyLine>::New();
    polyLine->GetPointIds()->SetNumberOfIds(m_pCurvePoints->GetNumberOfPoints());
    for (unsigned int i = 0; i < m_pCurvePoints->GetNumberOfPoints(); i++)
    {
        polyLine->GetPointIds()->SetId(i, i);
    }
    vtkSmartPointer<vtkCellArray> cells =
        vtkSmartPointer<vtkCellArray>::New();
    cells->InsertNextCell(polyLine);

    m_pCurveData->SetPoints(m_pCurvePoints);
    m_pCurveData->SetLines(cells);

}

void FreehandCutInteractor::ProjectPointOnPlane(const mitk::Point3D& input, vtkCamera* camera, mitk::Point3D& output)
{
    double eyeNormal[] = { 0.0,0.0,1.0 };
    double focalCenter[] = { 0.0,0.0,0.0 };
    camera->GetDirectionOfProjection(eyeNormal);
    camera->GetPosition(focalCenter);
    double range[2];
    camera->GetClippingRange(range);

    mitk::Vector3D normal(eyeNormal);
    mitk::Point3D center(focalCenter);

    normal.Normalize();
    center = center + normal * (range[0] + 1.0);

    double distance = (input - center) * normal;
    output = input - normal * distance;
}

void FreehandCutInteractor::Finished(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    if (!m_bInitFlag)
    {
        return;
    }
    m_bDrawing = false;

    if (m_pCurvePoints->GetNumberOfPoints() < 3)
    {
        return;
    }

    m_implementation->Cut(m_pCurvePoints.Get(),interactionEvent);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    if (m_bModify)
    {
        m_pCurveNode->SetColor(0, 1, 0);
    }

}

void FreehandCutInteractor::Undo(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    Undo();
}

void  FreehandCutInteractor::Redo(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    Redo();
}


void FreehandCutInteractor::SetInsideOut(bool flag)
{
    m_implementation->InsideOut = flag;
}

void  FreehandCutInteractor::Undo()
{
    m_implementation->Undo();
}

void  FreehandCutInteractor::Redo()
{
    m_implementation->Redo();
}

void  FreehandCutInteractor::Reset()
{
    m_implementation->Reset();
}

void FreehandCutInteractor::Finished()
{
    m_implementation->Finished();
    m_pCurveData->Reset();
    m_pCurvePoints->Reset();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    End();

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

double FreehandCutInteractor::DistanceBetweenPointAndPoints(mitk::Point3D &point, vtkPoints *vtkpoints)
{
    double minDistance = std::numeric_limits<double>::max();
    mitk::Point3D m_NearestPoint;
    for (int i = 0; i < vtkpoints->GetNumberOfPoints(); ++i)
    {
        mitk::Point3D vtkpoint = vtkpoints->GetPoint(i);
        double newDistance = sqrt((point[0] - vtkpoint[0])*(point[0] - vtkpoint[0]) + (point[1] - vtkpoint[1])*(point[1] - vtkpoint[1]) + (point[2] - vtkpoint[2])*(point[2] - vtkpoint[2]));
        if (newDistance < minDistance)
        {
            minDistance = newDistance;
            m_NearestPoint = vtkpoint;
        }
    }
    //存储修改前各点到最近点的距离
    m_vDistanceBetweenPointsAndPoint.clear();
    for (int i = 0; i < vtkpoints->GetNumberOfPoints(); ++i)
    {
        mitk::Point3D vtkpoint = vtkpoints->GetPoint(i);
        double newDistance = sqrt((m_NearestPoint[0] - vtkpoint[0])*(m_NearestPoint[0] - vtkpoint[0]) + (m_NearestPoint[1] - vtkpoint[1])*(m_NearestPoint[1] - vtkpoint[1]) + (m_NearestPoint[2] - vtkpoint[2])*(m_NearestPoint[2] - vtkpoint[2]));
        m_vDistanceBetweenPointsAndPoint.push_back(newDistance);
    }
    //存储修改前曲线上的各个点的坐标
    m_pCurvePointsBeforeModify->Reset();
    for (int i = 0; i < vtkpoints->GetNumberOfPoints(); ++i)
    {
        m_pCurvePointsBeforeModify->InsertNextPoint(vtkpoints->GetPoint(i));
    }
    return minDistance;
}

