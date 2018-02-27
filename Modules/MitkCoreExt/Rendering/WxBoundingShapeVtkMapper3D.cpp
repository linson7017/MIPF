/*===================================================================

* \author Guo Chu
* \date Jan,2018

===================================================================*/

#include "WxBoundingShapeVtkMapper3D.h"
#include "WxBoundingShapeUtil.h"
#include <mitkBaseProperty.h>
#include <vtkAppendPolyData.h>
#include <vtkCamera.h>
#include <vtkCubeSource.h>
#include <vtkDataSetMapper.h>
#include <vtkMath.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkSphereSource.h>
#include <vtkTransformFilter.h>
#include <mitkSurface.h>

namespace mitk
{
  class WxBoundingShapeVtkMapper3D::Impl
  {
    class LocalStorage : public Mapper::BaseLocalStorage
    {
    public:
      LocalStorage();
      ~LocalStorage();

      LocalStorage(const LocalStorage &) = delete;
      LocalStorage &operator=(const LocalStorage &) = delete;

      std::vector<vtkSmartPointer<vtkSphereSource>> Handles;
      vtkSmartPointer<vtkActor> Actor;
      vtkSmartPointer<vtkActor> HandleActor;
      vtkSmartPointer<vtkActor> SelectedHandleActor;
      vtkSmartPointer<vtkPropAssembly> PropAssembly;
    };

  public:
    Impl() : DistanceFromCam(1.0)
    {
      Point3D initialPoint;
      initialPoint.Fill(0);

      for (int i = 0; i < 6; ++i)
        HandlePropertyList.push_back(Handle(initialPoint, i, GetHandleIndices(i)));
    }

    double DistanceFromCam;
    std::vector<Handle> HandlePropertyList;
    mitk::LocalStorageHandler<LocalStorage> LocalStorageHandler;
  };
}

mitk::WxBoundingShapeVtkMapper3D::Impl::LocalStorage::LocalStorage()
  : Actor(vtkSmartPointer<vtkActor>::New()),
    HandleActor(vtkSmartPointer<vtkActor>::New()),
    SelectedHandleActor(vtkSmartPointer<vtkActor>::New()),
    PropAssembly(vtkSmartPointer<vtkPropAssembly>::New())
{
  for (int i = 0; i < 6; i++)
    Handles.push_back(vtkSmartPointer<vtkSphereSource>::New());
}

mitk::WxBoundingShapeVtkMapper3D::Impl::LocalStorage::~LocalStorage()
{
}

void mitk::WxBoundingShapeVtkMapper3D::SetDefaultProperties(DataNode *node, BaseRenderer *renderer, bool overwrite)
{
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

mitk::WxBoundingShapeVtkMapper3D::WxBoundingShapeVtkMapper3D() : m_Impl(new Impl)
{
}

mitk::WxBoundingShapeVtkMapper3D::~WxBoundingShapeVtkMapper3D()
{
  delete m_Impl;
}

void mitk::WxBoundingShapeVtkMapper3D::ApplyColorAndOpacityProperties(BaseRenderer*, vtkActor*)
{
}

void mitk::WxBoundingShapeVtkMapper3D::GenerateDataForRenderer(BaseRenderer *renderer)
{
  auto dataNode = this->GetDataNode();

  if (dataNode == nullptr)
    return;

  vtkCamera *camera = renderer->GetVtkRenderer()->GetActiveCamera();

  auto localStorage = m_Impl->LocalStorageHandler.GetLocalStorage(renderer);
  bool needGenerateData = localStorage->GetLastGenerateDataTime() < dataNode->GetMTime();
  double distance = camera->GetDistance();

  if (std::abs(distance - m_Impl->DistanceFromCam) > mitk::eps)
  {
    m_Impl->DistanceFromCam = distance;
    needGenerateData = true;
  }

  if (needGenerateData)
  {
    bool isVisible = true;
    dataNode->GetVisibility(isVisible, renderer);

    if (!isVisible)
    {
      localStorage->Actor->VisibilityOff();
      return;
    }

		mitk::Surface *surfaceData = dynamic_cast<mitk::Surface *>(dataNode->GetData());
		if (surfaceData == nullptr)
			return;

		mitk::BaseGeometry::Pointer geometry = surfaceData->GetGeometry();

    // calculate cornerpoints from geometry
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

		auto cube = vtkSmartPointer<vtkCubeSource>::New();
		cube->SetCenter(center[0], center[1], center[2]);
		cube->SetBounds(cornerPoints[0][0], cornerPoints[7][0], cornerPoints[0][1], cornerPoints[7][1], cornerPoints[0][2], cornerPoints[7][2]);
		cube->Update();

    vtkSmartPointer<vtkPolyData> polydata = cube->GetOutput();
    if (polydata == nullptr)
    {
      localStorage->Actor->VisibilityOff();
      return;
    }

    mitk::DoubleProperty::Pointer handleSizeProperty =
      dynamic_cast<mitk::DoubleProperty *>(this->GetDataNode()->GetProperty("Bounding Shape.Handle Size Factor"));

    ScalarType initialHandleSize;
    if (handleSizeProperty != nullptr)
      initialHandleSize = handleSizeProperty->GetValue();
    else
      initialHandleSize = 1.0 / 40.0;

    double handlesize =
      ((camera->GetDistance() * std::tan(vtkMath::RadiansFromDegrees(camera->GetViewAngle()))) / 2.0) *
      initialHandleSize;

    if (localStorage->PropAssembly->GetParts()->IsItemPresent(localStorage->HandleActor))
      localStorage->PropAssembly->RemovePart(localStorage->HandleActor);
    if (localStorage->PropAssembly->GetParts()->IsItemPresent(localStorage->Actor))
      localStorage->PropAssembly->RemovePart(localStorage->Actor);

    auto selectedhandlemapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    auto appendPoly = vtkSmartPointer<vtkAppendPolyData>::New();

    mitk::IntProperty::Pointer activeHandleId =
      dynamic_cast<mitk::IntProperty *>(dataNode->GetProperty("Bounding Shape.Active Handle ID"));

    int i = 0;
    for (auto &handle : localStorage->Handles)
    {
      Point3D handlecenter = m_Impl->HandlePropertyList[i].GetPosition();
      handle->SetCenter(handlecenter[0], handlecenter[1], handlecenter[2]);
      handle->SetRadius(handlesize);
      handle->Update();
      if (activeHandleId == nullptr)
      {
        appendPoly->AddInputConnection(handle->GetOutputPort());
      }
      else
      {
        if (activeHandleId->GetValue() != m_Impl->HandlePropertyList[i].GetIndex())
        {
          appendPoly->AddInputConnection(handle->GetOutputPort());
        }
        else
        {
          selectedhandlemapper->SetInputData(handle->GetOutput());
          localStorage->SelectedHandleActor->SetMapper(selectedhandlemapper);
          localStorage->SelectedHandleActor->GetProperty()->SetColor(0, 0, 1);
          localStorage->SelectedHandleActor->GetMapper()->SetInputDataObject(handle->GetOutput());
          localStorage->PropAssembly->AddPart(localStorage->SelectedHandleActor);
        }
      }
      i++;
    }
    appendPoly->Update();

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polydata);

    auto handlemapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    handlemapper->SetInputData(appendPoly->GetOutput());

    localStorage->Actor->SetMapper(mapper);
    localStorage->Actor->GetMapper()->SetInputDataObject(polydata);
		mitk::FloatProperty::Pointer cubeOpacity = dynamic_cast<mitk::FloatProperty *>(dataNode->GetProperty("opacity"));
		if (cubeOpacity != nullptr)
		{
			localStorage->Actor->GetProperty()->SetOpacity(cubeOpacity->GetValue());
		}
		else
		{
			localStorage->Actor->GetProperty()->SetOpacity(0.3);
		}    

    mitk::ColorProperty::Pointer selectedColor = dynamic_cast<mitk::ColorProperty *>(dataNode->GetProperty("color"));
    if (selectedColor != nullptr)
    {
      mitk::Color color = selectedColor->GetColor();
      localStorage->Actor->GetProperty()->SetColor(color[0], color[1], color[2]);
    }

    localStorage->HandleActor->SetMapper(handlemapper);
    if (activeHandleId == nullptr)
    {
      localStorage->HandleActor->GetProperty()->SetColor(1, 1, 1);
    }
    else
    {
      localStorage->HandleActor->GetProperty()->SetColor(0, 1, 0);
    }

    localStorage->HandleActor->GetMapper()->SetInputDataObject(appendPoly->GetOutput());

    this->ApplyColorAndOpacityProperties(renderer, localStorage->Actor);
    this->ApplyColorAndOpacityProperties(renderer, localStorage->HandleActor);
    this->ApplyColorAndOpacityProperties(renderer, localStorage->SelectedHandleActor);

    localStorage->Actor->VisibilityOn();
    localStorage->HandleActor->VisibilityOn();
    localStorage->SelectedHandleActor->VisibilityOn();

    localStorage->PropAssembly->AddPart(localStorage->Actor);
    localStorage->PropAssembly->AddPart(localStorage->HandleActor);
    localStorage->PropAssembly->VisibilityOn();

    localStorage->UpdateGenerateDataTime();
  }
}
vtkProp *mitk::WxBoundingShapeVtkMapper3D::GetVtkProp(BaseRenderer *renderer)
{
  return m_Impl->LocalStorageHandler.GetLocalStorage(renderer)->PropAssembly;
}
