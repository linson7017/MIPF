/*===================================================================

* \author Guo Chu
* \date Jan,2018

===================================================================*/

#include "WxSphereShapeVtkMapper3D.h"
#include "WxBoundingShapeUtil.h"
#include <vtkCamera.h>
#include <mitkSurface.h>

namespace mitk
{
	class WxSphereShapeVtkMapper3D::Impl
	{
		class LocalStorage : public Mapper::BaseLocalStorage
		{
		public:
			LocalStorage();
			~LocalStorage();

			LocalStorage(const LocalStorage &) = delete;
			LocalStorage &operator=(const LocalStorage &) = delete;

			vtkSmartPointer<vtkSphereSource> HandleSource;
			vtkSmartPointer<vtkActor> SphereActor;
			vtkSmartPointer<vtkActor> HandleActor;
			vtkSmartPointer<vtkPropAssembly> PropAssembly;
		};

	public:
		Impl() : DistanceFromCam(1.0)
		{
			Point3D initialPoint;
			initialPoint.Fill(0);

			ActiveHandle = Handle(initialPoint, 0, GetHandleIndices(0));
		}

		double DistanceFromCam;
		Handle ActiveHandle;
		mitk::LocalStorageHandler<LocalStorage> LocalStorageHandler;
	};
}

mitk::WxSphereShapeVtkMapper3D::Impl::LocalStorage::LocalStorage()
	: SphereActor(vtkSmartPointer<vtkActor>::New()),
	HandleActor(vtkSmartPointer<vtkActor>::New()),
	PropAssembly(vtkSmartPointer<vtkPropAssembly>::New()),
	HandleSource(vtkSmartPointer<vtkSphereSource>::New())
{
}

mitk::WxSphereShapeVtkMapper3D::Impl::LocalStorage::~LocalStorage()
{
}

void mitk::WxSphereShapeVtkMapper3D::SetDefaultProperties(DataNode *node, BaseRenderer *renderer, bool overwrite)
{
	Superclass::SetDefaultProperties(node, renderer, overwrite);
}

mitk::WxSphereShapeVtkMapper3D::WxSphereShapeVtkMapper3D() : m_Impl(new Impl)
{
}

mitk::WxSphereShapeVtkMapper3D::~WxSphereShapeVtkMapper3D()
{
	delete m_Impl;
}

void mitk::WxSphereShapeVtkMapper3D::ApplyColorAndOpacityProperties(BaseRenderer*, vtkActor*)
{
}

vtkProp *mitk::WxSphereShapeVtkMapper3D::GetVtkProp(BaseRenderer *renderer)
{
	return m_Impl->LocalStorageHandler.GetLocalStorage(renderer)->PropAssembly;
}

void mitk::WxSphereShapeVtkMapper3D::GenerateDataForRenderer(BaseRenderer *renderer)
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
			localStorage->SphereActor->VisibilityOff();
			return;
		}

		// set the input-object at time t for the mapper
		mitk::Surface *surfaceData = dynamic_cast<mitk::Surface *>(dataNode->GetData());
		if (surfaceData == nullptr)
			return;

		mitk::BaseGeometry::Pointer geometry = surfaceData->GetGeometry();

		// calculate cornerpoints from geometry
		std::vector<Point3D> cornerPoints = GetCornerPoints(geometry, false);

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


		// set handle positions
		Point3D pointLeft = CalcAvgPoint(cornerPoints[5], cornerPoints[6]);
		m_Impl->ActiveHandle.SetPosition(pointLeft);

		auto sphere = vtkSphereSource::New();
		sphere->SetCenter(center[0], center[1], center[2]);
		sphere->SetRadius(extent[0] / 2);
		sphere->SetPhiResolution(30);
		sphere->SetThetaResolution(30);
		sphere->Update();
		vtkSmartPointer<vtkPolyData> polydata = sphere->GetOutput();
		if (polydata == nullptr)
		{
			localStorage->SphereActor->VisibilityOff();
			return;
		}

		mitk::DoubleProperty::Pointer handleSizeProperty =
			dynamic_cast<mitk::DoubleProperty *>(this->GetDataNode()->GetProperty("Sphere Shape.Handle Size Factor"));

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
		if (localStorage->PropAssembly->GetParts()->IsItemPresent(localStorage->SphereActor))
			localStorage->PropAssembly->RemovePart(localStorage->SphereActor);

		auto selectedhandlemapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		auto unselectedhandlemapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		vtkSmartPointer<vtkPolyData> unselectedhandlepolydata;

		mitk::IntProperty::Pointer activeHandleId =
			dynamic_cast<mitk::IntProperty *>(dataNode->GetProperty("Sphere Shape.Active Handle ID"));

		Point3D handlecenter = m_Impl->ActiveHandle.GetPosition();
		localStorage->HandleSource->SetCenter(handlecenter[0], handlecenter[1], handlecenter[2]);
		localStorage->HandleSource->SetRadius(handlesize);
		localStorage->HandleSource->Update();

		unselectedhandlepolydata = localStorage->HandleSource->GetOutput();
		unselectedhandlemapper->SetInputData(unselectedhandlepolydata);
		localStorage->HandleActor->SetMapper(unselectedhandlemapper);
		localStorage->HandleActor->GetMapper()->SetInputDataObject(localStorage->HandleSource->GetOutput());
		localStorage->PropAssembly->AddPart(localStorage->HandleActor);

		if (activeHandleId != nullptr)
		{
			if (activeHandleId->GetValue() == -1)
			{
				localStorage->HandleActor->GetProperty()->SetColor(0, 1, 0);
			}
			else
			{
				localStorage->HandleActor->GetProperty()->SetColor(0, 0, 1);
			}
		}
		else
		{
			localStorage->HandleActor->GetProperty()->SetColor(1, 1, 1);
		}

		auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInputData(polydata);		

		localStorage->SphereActor->SetMapper(mapper);
		localStorage->SphereActor->GetMapper()->SetInputDataObject(polydata);
		mitk::FloatProperty::Pointer sphereOpacity = dynamic_cast<mitk::FloatProperty *>(dataNode->GetProperty("opacity"));
		if (sphereOpacity != nullptr)
		{
			localStorage->SphereActor->GetProperty()->SetOpacity(sphereOpacity->GetValue());
		}		

		mitk::ColorProperty::Pointer selectedColor = dynamic_cast<mitk::ColorProperty *>(dataNode->GetProperty("color"));
		if (selectedColor != nullptr)
		{
			mitk::Color color = selectedColor->GetColor();
			localStorage->SphereActor->GetProperty()->SetColor(color[0], color[1], color[2]);
		}

// 		mitk::FloatProperty::Pointer sphereOpacity = dynamic_cast<mitk::FloatProperty *>(dataNode->GetProperty("Sphere Opacity"));
// 		if (sphereOpacity != nullptr)
// 		{
// 			float opacity = sphereOpacity->GetValue();
// 			localStorage->SphereActor->GetProperty()->SetOpacity(opacity);
// 		}

		this->ApplyColorAndOpacityProperties(renderer, localStorage->SphereActor);
//		this->ApplySphereShapeProperties(renderer, localStorage->SphereActor);

		this->ApplyColorAndOpacityProperties(renderer, localStorage->HandleActor);
//		this->ApplySphereShapeProperties(renderer, localStorage->HandleActor);

		localStorage->SphereActor->VisibilityOn();
		localStorage->HandleActor->VisibilityOn();

		localStorage->PropAssembly->AddPart(localStorage->SphereActor);
		localStorage->PropAssembly->VisibilityOn();

		localStorage->UpdateGenerateDataTime();
	}
}
