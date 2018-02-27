/*===================================================================

* \author Guo Chu
* \date Jan,2018

===================================================================*/

#include "WxSphereShapeVtkMapper2D.h"
#include <vtkSphereSource.h>
#include "WxBoundingShapeUtil.h"
#include <vtkProperty2D.h>
#include <mitkSurface.h>
#include <vtkMath.h>
#include <vtkStripper.h>

static vtkSmartPointer<vtkSphereSource> CreateHandle()
{
	auto handle = vtkSmartPointer<vtkSphereSource>::New();

	handle->SetPhiResolution(8);
	handle->SetThetaResolution(16);

	return handle;
}

namespace mitk
{
	class WxSphereShapeVtkMapper2D::Impl
	{
	public:
		Impl()
		{
			Point3D initialPoint;
			initialPoint.Fill(0);

			ActiveHandle = Handle(initialPoint, 0, GetHandleIndices(0));
		}
		Handle ActiveHandle;
		mitk::LocalStorageHandler<LocalStorage> LocalStorageHandler;
	};
}

mitk::WxSphereShapeVtkMapper2D::LocalStorage::LocalStorage()
	: m_Actor(vtkSmartPointer<vtkActor>::New()),
	m_SelectedHandleActor(vtkSmartPointer<vtkActor2D>::New()),
	m_Mapper(vtkSmartPointer<vtkPolyDataMapper>::New()),
	m_SelectedHandleMapper(vtkSmartPointer<vtkPolyDataMapper2D>::New()),
	m_Cutter(vtkSmartPointer<vtkCutter>::New()),
	m_CuttingPlane(vtkSmartPointer<vtkPlane>::New()),
	m_LastSliceNumber(0),
	m_PropAssembly(vtkSmartPointer<vtkPropAssembly>::New()),
	m_ZoomFactor(1.0)
{
	m_Actor->SetMapper(m_Mapper);
	m_Actor->VisibilityOn();

	m_SelectedHandleActor->VisibilityOn();
	m_SelectedHandleActor->SetMapper(m_SelectedHandleMapper);

	vtkCoordinate *tcoord = vtkCoordinate::New();
	tcoord->SetCoordinateSystemToWorld();
	m_SelectedHandleMapper->SetTransformCoordinate(tcoord);
	tcoord->Delete();

	m_Cutter->SetCutFunction(m_CuttingPlane);

	m_HandleSphere = CreateHandle();

	m_PropAssembly->AddPart(m_Actor);
	m_PropAssembly->AddPart(m_SelectedHandleActor);
	m_PropAssembly->VisibilityOn();
}

bool mitk::WxSphereShapeVtkMapper2D::LocalStorage::IsUpdateRequired(mitk::BaseRenderer *renderer,
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

mitk::WxSphereShapeVtkMapper2D::LocalStorage::~LocalStorage()
{
}

void mitk::WxSphereShapeVtkMapper2D::SetDefaultProperties(DataNode *node, BaseRenderer *renderer, bool overwrite)
{
	Superclass::SetDefaultProperties(node, renderer, overwrite);
}

mitk::WxSphereShapeVtkMapper2D::WxSphereShapeVtkMapper2D() : m_Impl(new Impl)
{
}

mitk::WxSphereShapeVtkMapper2D::~WxSphereShapeVtkMapper2D()
{
	delete m_Impl;
}

vtkProp *mitk::WxSphereShapeVtkMapper2D::GetVtkProp(BaseRenderer *renderer)
{
	return m_Impl->LocalStorageHandler.GetLocalStorage(renderer)->m_PropAssembly;
}

void mitk::WxSphereShapeVtkMapper2D::ApplyColorAndOpacityProperties(BaseRenderer *, vtkActor *)
{
}

void mitk::WxSphereShapeVtkMapper2D::GenerateDataForRenderer(BaseRenderer *renderer)
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
		Surface::Pointer sphereSurface = static_cast<Surface *>(node->GetData());
		if (sphereSurface == nullptr)
			return;

		mitk::BaseGeometry::Pointer sphereGeometry = sphereSurface->GetGeometry();

		// calculate cornerpoints and extent from geometry with visualization offset
		std::vector<Point3D> cornerPoints = GetCornerPoints(sphereGeometry, false);
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

		mitk::Point3D sphereCenter = sphereGeometry->GetCenter();

		Point3D pointLeft = CalcAvgPoint(cornerPoints[5], cornerPoints[6]);
		m_Impl->ActiveHandle.SetPosition(pointLeft);

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

		auto sphere = vtkSphereSource::New();
		sphere->SetCenter(sphereCenter[0], sphereCenter[1], sphereCenter[2]);
		sphere->SetRadius(extent[0] / 2);
		sphere->SetPhiResolution(30);
		sphere->SetThetaResolution(30);
		sphere->Update();
		vtkSmartPointer<vtkPolyData> polydata = sphere->GetOutput();
		if (polydata == nullptr || (polydata->GetNumberOfPoints() < 1))
		{
			localStorage->m_Actor->VisibilityOff();
			localStorage->m_SelectedHandleActor->VisibilityOff();
			return;
		}

		// estimate current image plane to decide whether the sphere is visible or not
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

		localStorage->m_CuttingPlane->SetOrigin(origin);
		localStorage->m_CuttingPlane->SetNormal(normal);

		// add sphere polydata to local storage
		localStorage->m_Cutter->SetInputData(polydata);
		localStorage->m_Cutter->SetGenerateCutScalars(1);
		localStorage->m_Cutter->Update();

		if (localStorage->m_PropAssembly->GetParts()->IsItemPresent(localStorage->m_Actor))
			localStorage->m_PropAssembly->RemovePart(localStorage->m_Actor);
		if (localStorage->m_PropAssembly->GetParts()->IsItemPresent(localStorage->m_SelectedHandleActor))
			localStorage->m_PropAssembly->RemovePart(localStorage->m_SelectedHandleActor);

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

			// add handles and their assigned properties to the local storage
			mitk::IntProperty::Pointer activeHandleId =
				dynamic_cast<mitk::IntProperty *>(node->GetProperty("Sphere Shape.Active Handle ID"));

			bool visible = false;

			Point3D handleCenter = m_Impl->ActiveHandle.GetPosition();

			localStorage->m_HandleSphere->SetRadius(handleSize);
			localStorage->m_HandleSphere->SetCenter(handleCenter[0], handleCenter[1], handleCenter[2]);

			vtkMath::Normalize(normal);
			double angle = vtkMath::DegreesFromRadians(acos(vtkMath::Dot(normal, result0)));
			double angle1 = vtkMath::DegreesFromRadians(acos(vtkMath::Dot(normal, result1)));
			double angle2 = vtkMath::DegreesFromRadians(acos(vtkMath::Dot(normal, result2)));

			// show handles only if the corresponding face is aligned to the render window
			if (/*(((std::abs(angle - 0) < 0.001) || (std::abs(angle - 180) < 0.001))) ||*/
				(((std::abs(angle1 - 0) < 0.001) || (std::abs(angle1 - 180) < 0.001))) ||
				(((std::abs(angle2 - 0) < 0.001) || (std::abs(angle2 - 180) < 0.001))))
			{
				localStorage->m_HandleSphere->Update();
				vtkSmartPointer<vtkCutter> handleCutter = vtkSmartPointer<vtkCutter>::New();
				handleCutter->SetInputData(localStorage->m_HandleSphere->GetOutput());
				handleCutter->SetCutFunction(localStorage->m_CuttingPlane);
				handleCutter->SetGenerateCutScalars(1);
				handleCutter->Update();	

				//Èç¹ûÇÐµ½Handle
				if (handleCutter->GetOutput()->GetNumberOfPoints() > 0)
				{					
					localStorage->m_SelectedHandleMapper->SetInputData(localStorage->m_HandleSphere->GetOutput());
					localStorage->m_SelectedHandleActor->VisibilityOn();
					if (activeHandleId != nullptr)
					{

						if (activeHandleId->GetValue() == 0)
						{
							localStorage->m_SelectedHandleActor->GetProperty()->SetColor(0, 0, 1);
						}
						else
						{
							localStorage->m_SelectedHandleActor->GetProperty()->SetColor(0, 1, 0);
						}
					}
					else
					{
						localStorage->m_SelectedHandleActor->GetProperty()->SetColor(1, 1, 1);
					}
					visible = true;
				}
				else
				{
					localStorage->m_SelectedHandleActor->VisibilityOff();
				}
				
			}

			if (!visible)
			{
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
			
			mitk::FloatProperty::Pointer sphereOpacity = dynamic_cast<mitk::FloatProperty *>(node->GetProperty("opacity"));
			if (sphereOpacity != nullptr)
			{
				localStorage->m_Actor->GetProperty()->SetOpacity(sphereOpacity->GetValue());
			}

			// add parts to the overall storage
			localStorage->m_PropAssembly->AddPart(localStorage->m_Actor);
			localStorage->m_PropAssembly->AddPart(localStorage->m_SelectedHandleActor);			
			localStorage->m_PropAssembly->VisibilityOn();
			localStorage->m_Actor->VisibilityOn();
		}
		else
		{
			localStorage->m_PropAssembly->VisibilityOff();
			localStorage->m_Actor->VisibilityOff();
			localStorage->m_SelectedHandleActor->VisibilityOff();
			localStorage->UpdateGenerateDataTime();
		}
		localStorage->UpdateGenerateDataTime();
	}
}