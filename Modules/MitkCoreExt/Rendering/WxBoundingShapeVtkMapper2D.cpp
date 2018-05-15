/*===================================================================

* \author Guo Chu
* \date Jan,2018

===================================================================*/

#include "WxBoundingShapeUtil.h"
#include <mitkBaseProperty.h>
#include "WxBoundingShapeVtkMapper2D.h"

#include <vtkActor2D.h>
#include <vtkAppendPolyData.h>
#include <vtkCoordinate.h>
#include <vtkCubeSource.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkSphereSource.h>
#include <vtkStripper.h>
#include <vtkTransformFilter.h>
#include <vtkTransformPolyDataFilter.h>
#include <mitkSurface.h>

static vtkSmartPointer<vtkSphereSource> CreateHandle()
{
  auto handle = vtkSmartPointer<vtkSphereSource>::New();

  handle->SetPhiResolution(8);
  handle->SetThetaResolution(16);

  return handle;
}

namespace mitk
{
  class WxBoundingShapeVtkMapper2D::Impl
  {
  public:
    Impl()
    {
      Point3D initialPoint;
      initialPoint.Fill(0);

      for (int i = 0; i < 6; ++i)
        HandlePropertyList.push_back(Handle(initialPoint, i, GetHandleIndices(i)));
    }

    std::vector<Handle> HandlePropertyList;
    mitk::LocalStorageHandler<LocalStorage> LocalStorageHandler;
  };
}

mitk::WxBoundingShapeVtkMapper2D::LocalStorage::LocalStorage()
  : m_Actor(vtkSmartPointer<vtkActor>::New()),
  m_HandleActor(vtkSmartPointer<vtkActor2D>::New()),
  m_SelectedHandleActor(vtkSmartPointer<vtkActor2D>::New()),
  m_Mapper(vtkSmartPointer<vtkPolyDataMapper>::New()),
  m_HandleMapper(vtkSmartPointer<vtkPolyDataMapper2D>::New()),
  m_SelectedHandleMapper(vtkSmartPointer<vtkPolyDataMapper2D>::New()),
  m_Cutter(vtkSmartPointer<vtkCutter>::New()),
  m_CuttingPlane(vtkSmartPointer<vtkPlane>::New()),
  m_LastSliceNumber(0),
  m_PropAssembly(vtkSmartPointer<vtkPropAssembly>::New()),
  m_ZoomFactor(1.0)
{
  m_Actor->SetMapper(m_Mapper);
  m_Actor->VisibilityOn();

  m_HandleActor->SetMapper(m_HandleMapper);
  m_HandleActor->VisibilityOn();

  m_SelectedHandleActor->VisibilityOn();
  m_SelectedHandleActor->SetMapper(m_SelectedHandleMapper);

  vtkCoordinate *tcoord = vtkCoordinate::New();
  tcoord->SetCoordinateSystemToWorld();
  m_SelectedHandleMapper->SetTransformCoordinate(tcoord);
  tcoord->Delete();

  m_Cutter->SetCutFunction(m_CuttingPlane);

  for (int i = 0; i < 6; ++i)
    m_Handles.push_back(CreateHandle());

  m_PropAssembly->AddPart(m_Actor);
  m_PropAssembly->AddPart(m_HandleActor);
  m_PropAssembly->VisibilityOn();
}

bool mitk::WxBoundingShapeVtkMapper2D::LocalStorage::IsUpdateRequired(mitk::BaseRenderer *renderer,
  mitk::Mapper *mapper,
  mitk::DataNode *dataNode)
{
  const mitk::PlaneGeometry *worldGeometry = renderer->GetCurrentWorldPlaneGeometry();

  if (m_LastGenerateDataTime < worldGeometry->GetMTime())
    return true;

  unsigned int sliceNumber = renderer->GetSlice();
 
  if (m_LastSliceNumber != sliceNumber)
    return true;

  if (mapper && m_LastGenerateDataTime < mapper->GetMTime())
    return true;

  if (dataNode)
  {
    if (m_LastGenerateDataTime < dataNode->GetMTime())
      return true;

    mitk::BaseData *data = dataNode->GetData();

    if (data && m_LastGenerateDataTime < data->GetMTime())
      return true;
  }

  return false;
}

mitk::WxBoundingShapeVtkMapper2D::LocalStorage::~LocalStorage()
{
}

void mitk::WxBoundingShapeVtkMapper2D::SetDefaultProperties(DataNode *node, BaseRenderer *renderer, bool overwrite)
{
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

mitk::WxBoundingShapeVtkMapper2D::WxBoundingShapeVtkMapper2D() : m_Impl(new Impl)
{
}

mitk::WxBoundingShapeVtkMapper2D::~WxBoundingShapeVtkMapper2D()
{
  delete m_Impl;
}

void mitk::WxBoundingShapeVtkMapper2D::GenerateDataForRenderer(BaseRenderer *renderer)
{
  const DataNode::Pointer node = GetDataNode();
  if (node == nullptr)
    return;

  LocalStorage *localStorage = m_Impl->LocalStorageHandler.GetLocalStorage(renderer);

  // either update if GeometryData was modified or if the zooming was performed
  bool needGenerateData = localStorage->IsUpdateRequired(
    renderer, this, GetDataNode()); // true; 

  double scale = renderer->GetScaleFactorMMPerDisplayUnit();

  if (std::abs(scale - localStorage->m_ZoomFactor) > 0.001)
  {
    localStorage->m_ZoomFactor = scale;
    needGenerateData = true;
  }

  if (needGenerateData)
  {
    bool visible = true;
    GetDataNode()->GetVisibility(visible, renderer, "visible");

    if (!visible)
    {
      localStorage->m_Actor->VisibilityOff();
      return;
    }

		mitk::Surface *surfaceData = dynamic_cast<mitk::Surface *>(node->GetData());
		if (surfaceData == nullptr)
			return;
		mitk::BaseGeometry::Pointer geometry = surfaceData->GetGeometry();

    // calculate cornerpoints and extent from geometry with visualization offset
    std::vector<Point3D> cornerPoints = GetCornerPoints(geometry,true);
    Point3D p0 = cornerPoints[0];
    Point3D p1 = cornerPoints[1];
    Point3D p2 = cornerPoints[2];
    Point3D p4 = cornerPoints[4];
    Point3D extent;
    extent[0] =
      sqrt((p0[0] - p4[0]) * (p0[0] - p4[0]) + (p0[1] - p4[1]) * (p0[1] - p4[1]) + (p0[2] - p4[2]) * (p0[2] - p4[2]));
    extent[1] =
      sqrt((p0[0] - p2[0]) * (p0[0] - p2[0]) + (p0[1] - p2[1]) * (p0[1] - p2[1]) + (p0[2] - p2[2]) * (p0[2] - p2[2]));
    extent[2] =
      sqrt((p0[0] - p1[0]) * (p0[0] - p1[0]) + (p0[1] - p1[1]) * (p0[1] - p1[1]) + (p0[2] - p1[2]) * (p0[2] - p1[2]));

    // calculate center based on half way of the distance between two opposing cornerpoints
    mitk::Point3D center = CalcAvgPoint(cornerPoints[7], cornerPoints[0]);

    if (m_Impl->HandlePropertyList.size() == 6)
    {
      // set handle positions
      Point3D pointLeft = CalcAvgPoint(cornerPoints[5], cornerPoints[6]);
      Point3D pointRight = CalcAvgPoint(cornerPoints[1], cornerPoints[2]);
      Point3D pointTop = CalcAvgPoint(cornerPoints[0], cornerPoints[6]);
      Point3D pointBottom = CalcAvgPoint(cornerPoints[7], cornerPoints[1]);
      Point3D pointFront = CalcAvgPoint(cornerPoints[2], cornerPoints[7]);
      Point3D pointBack = CalcAvgPoint(cornerPoints[4], cornerPoints[1]);

      m_Impl->HandlePropertyList[0].SetPosition(pointLeft);
      m_Impl->HandlePropertyList[1].SetPosition(pointRight);
      m_Impl->HandlePropertyList[2].SetPosition(pointTop);
      m_Impl->HandlePropertyList[3].SetPosition(pointBottom);
      m_Impl->HandlePropertyList[4].SetPosition(pointFront);
      m_Impl->HandlePropertyList[5].SetPosition(pointBack);
    }

    // caculate face normals
    double result0[3], result1[3], result2[3];
    double a[3], b[3];
    a[0] = (cornerPoints[5][0] - cornerPoints[6][0]);
    a[1] = (cornerPoints[5][1] - cornerPoints[6][1]);
    a[2] = (cornerPoints[5][2] - cornerPoints[6][2]);

    b[0] = (cornerPoints[5][0] - cornerPoints[4][0]);
    b[1] = (cornerPoints[5][1] - cornerPoints[4][1]);
    b[2] = (cornerPoints[5][2] - cornerPoints[4][2]);

    vtkMath::Cross(a, b, result0);

    a[0] = (cornerPoints[0][0] - cornerPoints[6][0]);
    a[1] = (cornerPoints[0][1] - cornerPoints[6][1]);
    a[2] = (cornerPoints[0][2] - cornerPoints[6][2]);

    b[0] = (cornerPoints[0][0] - cornerPoints[2][0]);
    b[1] = (cornerPoints[0][1] - cornerPoints[2][1]);
    b[2] = (cornerPoints[0][2] - cornerPoints[2][2]);

    vtkMath::Cross(a, b, result1);

    a[0] = (cornerPoints[2][0] - cornerPoints[7][0]);
    a[1] = (cornerPoints[2][1] - cornerPoints[7][1]);
    a[2] = (cornerPoints[2][2] - cornerPoints[7][2]);

    b[0] = (cornerPoints[2][0] - cornerPoints[6][0]);
    b[1] = (cornerPoints[2][1] - cornerPoints[6][1]);
    b[2] = (cornerPoints[2][2] - cornerPoints[6][2]);

    vtkMath::Cross(a, b, result2);

    vtkMath::Normalize(result0);
    vtkMath::Normalize(result1);
    vtkMath::Normalize(result2);

		auto cube = vtkSmartPointer<vtkCubeSource>::New();
		cube->SetCenter(center[0], center[1], center[2]);
		cube->SetBounds(cornerPoints[0][0], cornerPoints[7][0], cornerPoints[0][1], cornerPoints[7][1], cornerPoints[0][2], cornerPoints[7][2]);
		cube->Update();

    vtkSmartPointer<vtkPolyData> polydata = cube->GetOutput();
    if (polydata == nullptr || (polydata->GetNumberOfPoints() < 1))
    {
      localStorage->m_Actor->VisibilityOff();
      localStorage->m_HandleActor->VisibilityOff();
      localStorage->m_SelectedHandleActor->VisibilityOff();
      return;
    }

    // estimate current image plane to decide whether the cube is visible or not
    const PlaneGeometry *planeGeometry = renderer->GetCurrentWorldPlaneGeometry();
    if ((planeGeometry == nullptr) || (!planeGeometry->IsValid()) || (!planeGeometry->HasReferenceGeometry()))
      return;

    double origin[3];
    origin[0] = planeGeometry->GetOrigin()[0];
    origin[1] = planeGeometry->GetOrigin()[1];
    origin[2] = planeGeometry->GetOrigin()[2];

    double normal[3];
    normal[0] = planeGeometry->GetNormal()[0];
    normal[1] = planeGeometry->GetNormal()[1];
    normal[2] = planeGeometry->GetNormal()[2];

    //    QF_INFO << "normal1 " << normal[0] << " " << normal[1] << " " << normal[2];
    localStorage->m_CuttingPlane->SetOrigin(origin);
    localStorage->m_CuttingPlane->SetNormal(normal);

    // add cube polydata to local storage
    localStorage->m_Cutter->SetInputData(polydata);
    localStorage->m_Cutter->SetGenerateCutScalars(1);
    localStorage->m_Cutter->Update();

    if (localStorage->m_PropAssembly->GetParts()->IsItemPresent(localStorage->m_HandleActor))
      localStorage->m_PropAssembly->RemovePart(localStorage->m_HandleActor);
    if (localStorage->m_PropAssembly->GetParts()->IsItemPresent(localStorage->m_Actor))
      localStorage->m_PropAssembly->RemovePart(localStorage->m_Actor);

    vtkCoordinate *tcoord = vtkCoordinate::New();
    tcoord->SetCoordinateSystemToWorld();
    localStorage->m_HandleMapper->SetTransformCoordinate(tcoord);
    tcoord->Delete();

    if (localStorage->m_Cutter->GetOutput()->GetNumberOfPoints() > 0) // if plane is visible in the renderwindow
    {
      mitk::DoubleProperty::Pointer handleSizeProperty =
        dynamic_cast<mitk::DoubleProperty *>(this->GetDataNode()->GetProperty("Bounding Shape.Handle Size Factor"));

      ScalarType initialHandleSize;
      if (handleSizeProperty != nullptr)
        initialHandleSize = handleSizeProperty->GetValue();
      else
        initialHandleSize = 1.0 / 40.0;

      mitk::Point2D displaySize = renderer->GetDisplaySizeInMM();
      double handleSize = ((displaySize[0] + displaySize[1]) / 2.0) * initialHandleSize;

      auto appendPoly = vtkSmartPointer<vtkAppendPolyData>::New();
      unsigned int i = 0;

      // add handles and their assigned properties to the local storage
      mitk::IntProperty::Pointer activeHandleId =
        dynamic_cast<mitk::IntProperty *>(node->GetProperty("Bounding Shape.Active Handle ID"));

      bool visible = false;
      bool selected = false;
      for (auto handle : localStorage->m_Handles)
      {
        Point3D handleCenter = m_Impl->HandlePropertyList[i].GetPosition();

        handle->SetRadius(handleSize);
        handle->SetCenter(handleCenter[0], handleCenter[1], handleCenter[2]);

        vtkMath::Normalize(normal);
        double angle = vtkMath::DegreesFromRadians(acos(vtkMath::Dot(normal, result0)));
        double angle1 = vtkMath::DegreesFromRadians(acos(vtkMath::Dot(normal, result1)));
        double angle2 = vtkMath::DegreesFromRadians(acos(vtkMath::Dot(normal, result2)));

        // show handles only if the corresponding face is aligned to the render window
        if ((((std::abs(angle - 0) < 0.001) || (std::abs(angle - 180) < 0.001)) && i != 0 && i != 1) ||
          (((std::abs(angle1 - 0) < 0.001) || (std::abs(angle1 - 180) < 0.001)) && i != 2 && i != 3) ||
          (((std::abs(angle2 - 0) < 0.001) || (std::abs(angle2 - 180) < 0.001)) && i != 4 && i != 5))
        {
          if (activeHandleId == nullptr)
          {
            appendPoly->AddInputConnection(handle->GetOutputPort());
          }
          else
          {
            if ((activeHandleId->GetValue() != m_Impl->HandlePropertyList[i].GetIndex()))
            {
              appendPoly->AddInputConnection(handle->GetOutputPort());
            }
            else
            {
              handle->Update();
              localStorage->m_SelectedHandleMapper->SetInputData(handle->GetOutput());
							localStorage->m_SelectedHandleActor->GetProperty()->SetColor(0, 0, 1);
              localStorage->m_SelectedHandleActor->VisibilityOn();
              selected = true;
            }
          }
          visible = true;
        }

        i++;
      }

      if (visible)
      {
        appendPoly->Update();
      }
      else
      {
        localStorage->m_HandleActor->VisibilityOff();
        localStorage->m_SelectedHandleActor->VisibilityOff();
      }

      auto stripper = vtkSmartPointer<vtkStripper>::New();
      stripper->SetInputData(localStorage->m_Cutter->GetOutput());
      stripper->Update();

      auto cutPolyData = vtkSmartPointer<vtkPolyData>::New();
      cutPolyData->SetPoints(stripper->GetOutput()->GetPoints());
      cutPolyData->SetPolys(stripper->GetOutput()->GetLines());

      localStorage->m_Actor->GetMapper()->SetInputDataObject(cutPolyData);
      mitk::ColorProperty::Pointer selectedColor = dynamic_cast<mitk::ColorProperty *>(node->GetProperty("color"));
      if (selectedColor != nullptr)
      {
        mitk::Color color = selectedColor->GetColor();
        localStorage->m_Actor->GetProperty()->SetColor(color[0], color[1], color[2]);
      }

			mitk::FloatProperty::Pointer cubeOpacity = dynamic_cast<mitk::FloatProperty *>(node->GetProperty("opacity"));
			if (cubeOpacity != nullptr)
			{
				localStorage->m_Actor->GetProperty()->SetOpacity(cubeOpacity->GetValue());
			}
			else
			{
				localStorage->m_Actor->GetProperty()->SetOpacity(0.3);
			}

      if (activeHandleId != nullptr)
      {
        localStorage->m_HandleActor->GetProperty()->SetColor(0, 1, 0);
      }
      else
      {
        localStorage->m_HandleActor->GetProperty()->SetColor(1, 1, 1);
      }
      localStorage->m_HandleActor->GetMapper()->SetInputDataObject(appendPoly->GetOutput());

      // add parts to the overall storage
      localStorage->m_PropAssembly->AddPart(localStorage->m_Actor);
      localStorage->m_PropAssembly->AddPart(localStorage->m_HandleActor);
      if (selected)
      {
        localStorage->m_PropAssembly->AddPart(localStorage->m_SelectedHandleActor);
      }

      localStorage->m_PropAssembly->VisibilityOn();
      localStorage->m_Actor->VisibilityOn();
      localStorage->m_HandleActor->VisibilityOn();
    }
    else
    {
      localStorage->m_PropAssembly->VisibilityOff();
      localStorage->m_Actor->VisibilityOff();
      localStorage->m_HandleActor->VisibilityOff();
      localStorage->m_SelectedHandleActor->VisibilityOff();
      localStorage->UpdateGenerateDataTime();
    }
    localStorage->UpdateGenerateDataTime();
  }
}

vtkProp *mitk::WxBoundingShapeVtkMapper2D::GetVtkProp(BaseRenderer *renderer)
{
  return m_Impl->LocalStorageHandler.GetLocalStorage(renderer)->m_PropAssembly;
}

void mitk::WxBoundingShapeVtkMapper2D::ApplyColorAndOpacityProperties(BaseRenderer *, vtkActor *)
{
}
