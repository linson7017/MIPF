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

#include "Rendering/FreehandSurfaceCutMapper3D.h"

#include <limits>
#include <iostream>
#include <cmath>

FreehandSurfaceCutInteractor::FreehandSurfaceCutInteractor() :m_bDrawing(false), m_bInitFlag(false), m_pSurfaceData(nullptr), m_pCurveNode(nullptr), m_pDataStorage(nullptr)
, m_currentSurfaceIndex(0), m_bInsideOut(false), m_bModify(false)
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
		m_pCurvePointsBeforeModify = vtkSmartPointer<vtkPoints>::New();

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
	CONNECT_FUNCTION("initModify", InitModify);
    
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

void FreehandSurfaceCutInteractor::InitModify(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
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

void FreehandSurfaceCutInteractor::Modify(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
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
		
		double DistanceNeedToMove = maxDistance * exp(-m_vDistanceBetweenPointsAndPoint[i]* m_vDistanceBetweenPointsAndPoint[i] / 555);
		tempPoint = tempPoint + dirVector*DistanceNeedToMove;
		m_pCurvePoints->SetPoint(i, tempPoint[0], tempPoint[1], tempPoint[2]);
	  }
	  RefreshCurve();
	  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
	}
  }
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
	if (m_bModify)
	{
	  m_pCurveNode->SetColor(0, 1, 0);
	}

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
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	End();

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

double FreehandSurfaceCutInteractor::DistanceBetweenPointAndPoints(mitk::Point3D &point, vtkPoints *vtkpoints)
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