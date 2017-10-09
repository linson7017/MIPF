#include "FreehandSurfaceCutInteractor.h"
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

#include <vtkFillHolesFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkPointData.h>
#include <vtkFeatureEdges.h>
#include <vtkStripper.h>

#include "Rendering/FreehandSurfaceCutMapper3D.h"

FreehandSurfaceCutInteractor::FreehandSurfaceCutInteractor():m_bDrawing(false), m_bInitFlag(false), m_pSurfaceData(nullptr), m_pCurveNode(nullptr) , m_pDataStorage(nullptr)
, m_currentSurfaceIndex(0), m_bInsideOut(false), m_pRenderer(nullptr)
{
}


FreehandSurfaceCutInteractor::~FreehandSurfaceCutInteractor()
{
}

void FreehandSurfaceCutInteractor::SetDataNode(mitk::DataNode *dataNode)
{
    mitk::DataInteractor::SetDataNode(dataNode);
    Init();
}

void FreehandSurfaceCutInteractor::SetRenderer(vtkRenderer* renderer)
{
    m_pRenderer = renderer;
}

void FreehandSurfaceCutInteractor::Start()
{
   if (m_pRenderer)
   {
       m_pRenderer->GetActiveCamera()->ParallelProjectionOn();
       mitk::RenderingManager::GetInstance()->RequestUpdateAll();
   }
}

void FreehandSurfaceCutInteractor::End()
{
    if (m_pRenderer)
    {
        m_pRenderer->GetActiveCamera()->ParallelProjectionOff();
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void FreehandSurfaceCutInteractor::Init()
{

    m_pSurfaceData = dynamic_cast<mitk::Surface*>(GetDataNode()->GetData());
    if (m_pSurfaceData&&m_pDataStorage)
    {
        m_bInitFlag = true;
        m_pCurveNode = mitk::DataNode::New();
        m_pCurveNode->SetColor(0.0, 1.0, 0.0);
        m_pCurveNode->SetProperty("helper object", mitk::BoolProperty::New(true));
        m_pCurveNode->SetFloatProperty("line width",2.0);


        mitk::FreehandSurfaceCutMapper3D::Pointer mapper = mitk::FreehandSurfaceCutMapper3D::New();
        m_pCurveNode->SetMapper(mitk::BaseRenderer::Standard3D, mapper);
        m_pCurveNode->SetStringProperty("3d mapper type", "freehand surface cut");

        mitk::Surface::Pointer curve = mitk::Surface::New();
        m_pCurveData = vtkSmartPointer<vtkPolyData>::New();
        curve->SetVtkPolyData(m_pCurveData);
        m_pCurveNode->SetData(curve);
        m_pDataStorage->Add(m_pCurveNode);

        m_pCurvePoints = vtkSmartPointer<vtkPoints>::New();

        m_vSurface.clear();

        vtkSmartPointer<vtkPolyData> originSurfaceData = vtkSmartPointer<vtkPolyData>::New();
        originSurfaceData->DeepCopy(m_pSurfaceData->GetVtkPolyData());
        m_vSurface.push_back(originSurfaceData);
    }
}

void FreehandSurfaceCutInteractor::ConnectActionsAndFunctions()
{
    // connect the action and condition names of the state machine pattern with function within
    // this DataInteractor
    CONNECT_FUNCTION("initMove", InitMove);
    CONNECT_FUNCTION("draw", Draw);
    CONNECT_FUNCTION("modify", Modify);
    CONNECT_FUNCTION("finished", Finished);
    CONNECT_FUNCTION("undo", Undo);
    CONNECT_FUNCTION("redo", Redo);

}

void FreehandSurfaceCutInteractor::InitMove(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    mitk::InteractionPositionEvent *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
    if (positionEvent == NULL)
        return;
    m_LastPoint = positionEvent->GetPositionInWorld();
    m_bDrawing = true;
    m_pCurvePoints->Reset();
    m_pCurveData->Reset();
}

void FreehandSurfaceCutInteractor::Draw(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
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

void FreehandSurfaceCutInteractor::RefreshCurve()
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

void FreehandSurfaceCutInteractor::Modify(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    if (m_bDrawing&&m_bInitFlag)
    {
        mitk::InteractionPositionEvent *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
        if (positionEvent != NULL)
        {
            mitk::Point3D newPoint, resultPoint;
            newPoint = positionEvent->GetPositionInWorld();
            mitk::Vector3D dirVector = newPoint - m_LastPoint;

        }
    }
}

void FreehandSurfaceCutInteractor::ProjectPointOnPlane(const mitk::Point3D& input, vtkCamera* camera, mitk::Point3D& output)
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
    center = center + normal *( range[0]+1.0);

    double distance =  (input - center) * normal;
    output = input - normal * distance;
}

void FreehandSurfaceCutInteractor::Finished(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    if (!m_bInitFlag)
    {
        return;
    }
    m_bDrawing = false;

   if (m_pCurvePoints->GetNumberOfPoints()<3)
   {
       return;
   }

   mitk::InteractionPositionEvent *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
    if (!positionEvent)
    {
        return;
    }

    auto clipFunction = vtkSmartPointer<vtkImplicitSelectionLoop>::New();
    clipFunction->SetLoop(m_pCurvePoints.GetPointer());

    double eyeNormal[] = {0.0,0.0,1.0};
    positionEvent->GetSender()->GetVtkRenderer()->GetActiveCamera()->GetDirectionOfProjection(eyeNormal);
    clipFunction->SetNormal(eyeNormal);
   // clipFunction->AutomaticNormalGenerationOn();

    m_pSurfaceData = dynamic_cast<mitk::Surface*>(GetDataNode()->GetData());
    vtkSmartPointer<vtkClipPolyData> clipper =
        vtkSmartPointer<vtkClipPolyData>::New();
    clipper->SetInputData(m_pSurfaceData->GetVtkPolyData());
    clipper->SetClipFunction(clipFunction);
    clipper->SetInsideOut(m_bInsideOut);

    clipper->Update();


    m_pSurfaceData->GetVtkPolyData()->DeepCopy(clipper->GetOutput());
    
    
    m_vSurface.erase(m_vSurface.begin() + m_currentSurfaceIndex+1, m_vSurface.end());
    m_vSurface.push_back(clipper->GetOutput());
    m_currentSurfaceIndex++;

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

}

void FreehandSurfaceCutInteractor::Undo(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    Undo();
}

void  FreehandSurfaceCutInteractor::Redo(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    Redo();
}


void FreehandSurfaceCutInteractor::SetInsideOut(bool flag)
{
    m_bInsideOut = flag;
}

void  FreehandSurfaceCutInteractor::Undo()
{
    if (m_currentSurfaceIndex>0)
    {
        m_currentSurfaceIndex--;
        RefreshCurrentSurface();
    }
}

void  FreehandSurfaceCutInteractor::Redo()
{
    if (m_currentSurfaceIndex<m_vSurface.size()-1)
    {
        m_currentSurfaceIndex++;
        RefreshCurrentSurface();
    }
}

void FreehandSurfaceCutInteractor::RefreshCurrentSurface()
{
    if (m_currentSurfaceIndex >= 0 && m_currentSurfaceIndex < m_vSurface.size())
    {
        m_pSurfaceData->GetVtkPolyData()->DeepCopy(m_vSurface.at(m_currentSurfaceIndex));
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void  FreehandSurfaceCutInteractor::Reset()
{
    if (m_vSurface.size()>0)
    {
        m_pSurfaceData->GetVtkPolyData()->DeepCopy(m_vSurface.at(0));
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void FreehandSurfaceCutInteractor::Finished()
{
    if (m_bInitFlag)
    {
        m_vSurface.erase(m_vSurface.begin() + 1, m_vSurface.end());
        
    }
    else
    {
        m_vSurface.clear();
    }
    m_currentSurfaceIndex = 0;
    m_pCurveData->Reset();
    m_pCurvePoints->Reset();

    End();

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}