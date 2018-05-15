#include "FreehandVolumeCutInteractor.h"

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
#include <vtkLinearExtrusionFilter.h>
#include <vtkPolygon.h>
#include <vtkImageIterator.h>
#include <vtkImageData.h>
#include <vtkImageStencil.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkTransformPolyDataFilter.h>

#include "Rendering/FreehandSurfaceCutMapper3D.h"

#include <limits>
#include <iostream>
#include <cmath>

#include "qf_log.h"

FreehandVolumeCutInteractor::FreehandVolumeCutInteractor() :m_bDrawing(false), m_bInitFlag(false), m_pImageData(nullptr), m_pCurveNode(nullptr), m_pDataStorage(nullptr)
, m_currentImageIndex(0), m_bInsideOut(true), m_bModify(false)
{
}


FreehandVolumeCutInteractor::~FreehandVolumeCutInteractor()
{
}

void FreehandVolumeCutInteractor::SetDataNode(mitk::DataNode *dataNode)
{
    mitk::DataInteractor::SetDataNode(dataNode);
    Init();
}

void FreehandVolumeCutInteractor::SetRenderer(vtkRenderer* renderer)
{
    m_pRenderer = renderer;
}

void FreehandVolumeCutInteractor::Start()
{
    if (m_pRenderer)
    {
        m_pRenderer->GetActiveCamera()->ParallelProjectionOn();
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void FreehandVolumeCutInteractor::End()
{
    if (m_pRenderer)
    {
        m_pRenderer->GetActiveCamera()->ParallelProjectionOff();
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void FreehandVolumeCutInteractor::Init()
{

    m_pImageData = dynamic_cast<mitk::Image*>(GetDataNode()->GetData());
    if (m_pImageData&&m_pDataStorage)
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

        m_vImage.clear();

        vtkSmartPointer<vtkImageData> originSurfaceData = vtkSmartPointer<vtkImageData>::New();
        originSurfaceData->DeepCopy(m_pImageData->GetVtkImageData());
        m_vImage.push_back(originSurfaceData);
    }
}

void FreehandVolumeCutInteractor::ConnectActionsAndFunctions()
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

void FreehandVolumeCutInteractor::InitMove(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    mitk::InteractionPositionEvent *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
    if (positionEvent == NULL)
        return;
    m_LastPoint = positionEvent->GetPositionInWorld();

    m_bDrawing = true;
    m_pCurvePoints->Reset();
    m_pCurveData->Reset();
}

void FreehandVolumeCutInteractor::InitModify(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    QF_INFO << "Begin Modify";
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

void FreehandVolumeCutInteractor::Modify(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
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

void FreehandVolumeCutInteractor::Draw(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
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

void FreehandVolumeCutInteractor::RefreshCurve()
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

void FreehandVolumeCutInteractor::ProjectPointOnPlane(const mitk::Point3D& input, vtkCamera* camera, mitk::Point3D& output)
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

void FreehandVolumeCutInteractor::ImageStencilCut(mitk::Image* data, vtkPoints* points, mitk::InteractionEvent* interactionEvent)
{
    mitk::InteractionPositionEvent *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
    if (!positionEvent)
    {
        return;
    }

    vtkSmartPointer<vtkPolygon> polygon =
        vtkSmartPointer<vtkPolygon>::New();
    polygon->GetPointIds()->SetNumberOfIds(points->GetNumberOfPoints());
    for (int i = 0; i < points->GetNumberOfPoints(); i++)
    {
        polygon->GetPointIds()->SetId(i,i);
    }
    auto polygons = vtkSmartPointer<vtkCellArray>::New();
    polygons->InsertNextCell(polygon);
    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetPolys(polygons);

    vtkImageData* img = data->GetVtkImageData();
    double normal[3];
    positionEvent->GetSender()->GetVtkRenderer()->GetActiveCamera()->GetDirectionOfProjection(normal);

    vtkSmartPointer<vtkLinearExtrusionFilter> extrude =
        vtkSmartPointer<vtkLinearExtrusionFilter>::New();
    extrude->SetInputData(polyData);
    extrude->SetExtrusionTypeToVectorExtrusion();
    extrude->SetCapping(1);
    extrude->SetVector(normal);
    extrude->SetScaleFactor(800);

    //origin in vtkimagedata is often different from the origin in geometry
    mitk::Point3D imageOrigin;
    imageOrigin = data->GetGeometry()->GetOrigin();

    vtkSmartPointer<vtkPolyDataToImageStencil> pol2stenc =
        vtkSmartPointer<vtkPolyDataToImageStencil>::New();
    pol2stenc->SetInputConnection(extrude->GetOutputPort());
    pol2stenc->SetTolerance(0.0);
    pol2stenc->ReleaseDataFlagOn();
    pol2stenc->SetOutputOrigin(imageOrigin.GetDataPointer());
    pol2stenc->SetOutputSpacing(img->GetSpacing());
    pol2stenc->SetOutputWholeExtent(img->GetExtent());

    vtkSmartPointer<vtkImageStencil> imgstenc =
        vtkSmartPointer<vtkImageStencil>::New();
    imgstenc->SetInputData(img);
    imgstenc->SetStencilConnection(pol2stenc->GetOutputPort());
    imgstenc->SetReverseStencil(m_bInsideOut);
    imgstenc->SetBackgroundValue(data->GetScalarValueMin());
    imgstenc->Update();

    img->DeepCopy(imgstenc->GetOutput());

}

//much slower than image stencil cut
void FreehandVolumeCutInteractor::PolygonCut(mitk::Image* data, vtkPoints* points, mitk::InteractionEvent* interactionEvent)
{
    mitk::InteractionPositionEvent *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
    if (!positionEvent)
    {
        return;
    }

    vtkSmartPointer<vtkPolygon> polygon =
        vtkSmartPointer<vtkPolygon>::New();
    for (int i = 0; i < points->GetNumberOfPoints(); i++)
    {
        polygon->GetPoints()->InsertNextPoint(points->GetPoint(i));
    }

    vtkImageData* img = data->GetVtkImageData();
    


    int* dims = img->GetDimensions();
    double *ptr = static_cast<double *>(img->GetScalarPointer(0, 0, 0));
    mitk::Point3D index, coord,pCoord;
    double normal[3];
    positionEvent->GetSender()->GetVtkRenderer()->GetActiveCamera()->GetDirectionOfProjection(normal);
    for (int z = 0; z<dims[2]; z++)
    {
        for (int y = 0; y<dims[1]; y++)
        {
            for (int x = 0; x<dims[0]; x++)
            {
                index[0] = x;
                index[1] = y;
                index[2] = z;
                data->GetGeometry()->IndexToWorld(index, coord);
                ProjectPointOnPlane(coord, positionEvent->GetSender()->GetVtkRenderer()->GetActiveCamera(), pCoord);
                if (vtkPolygon::PointInPolygon(pCoord.GetDataPointer(),
                    polygon->GetPoints()->GetNumberOfPoints(),
                    static_cast<double*>(polygon->GetPoints()->GetData()->GetVoidPointer(0)),
                    polygon->GetPoints()->GetBounds(), normal
                    ))
                {
                    unsigned int *pixel = (unsigned int *)(img->GetScalarPointer(x,y,z));
                    *pixel = data->GetScalarValueMin();
                }
            }
        }
    }


}


void FreehandVolumeCutInteractor::Finished(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
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

   // auto cutImage = vtkSmartPointer<vtkImageData>::New();
    ImageStencilCut(m_pImageData, m_pCurvePoints, interactionEvent);
    m_pImageData->Modified();
   // cutImage->DeepCopy(m_pImageData->GetVtkImageData());

    //m_pImageData->GetVtkImageData()->DeepCopy(cutImage);


    auto cutImage = vtkSmartPointer<vtkImageData>::New();
    cutImage->DeepCopy(m_pImageData->GetVtkImageData());
    m_vImage.erase(m_vImage.begin() + m_currentImageIndex + 1, m_vImage.end());
    m_vImage.push_back(cutImage);
    m_currentImageIndex++;

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    if (m_bModify)
    {
        m_pCurveNode->SetColor(0, 1, 0);
    }

}

void FreehandVolumeCutInteractor::Undo(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    Undo();
}

void  FreehandVolumeCutInteractor::Redo(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    Redo();
}


void FreehandVolumeCutInteractor::SetInsideOut(bool flag)
{
    m_bInsideOut = flag;
}

void  FreehandVolumeCutInteractor::Undo()
{
    if (m_currentImageIndex>0)
    {
        m_currentImageIndex--;
        RefreshCurrentImage();
    }
}

void  FreehandVolumeCutInteractor::Redo()
{
    if (m_currentImageIndex<m_vImage.size() - 1)
    {
        m_currentImageIndex++;
        RefreshCurrentImage();
    }
}

void FreehandVolumeCutInteractor::RefreshCurrentImage()
{
    if (m_currentImageIndex >= 0 && m_currentImageIndex < m_vImage.size())
    {
        m_pImageData->GetVtkImageData()->DeepCopy(m_vImage.at(m_currentImageIndex));
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void  FreehandVolumeCutInteractor::Reset()
{
    if (m_vImage.size()>0)
    {
        m_pImageData->GetVtkImageData()->DeepCopy(m_vImage.at(0));
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void FreehandVolumeCutInteractor::Finished()
{
    if (m_bInitFlag)
    {
        m_vImage.erase(m_vImage.begin() + 1, m_vImage.end());

    }
    else
    {
        m_vImage.clear();
    }
    m_currentImageIndex = 0;
    m_pCurveData->Reset();
    m_pCurvePoints->Reset();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    End();

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

double FreehandVolumeCutInteractor::DistanceBetweenPointAndPoints(mitk::Point3D &point, vtkPoints *vtkpoints)
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