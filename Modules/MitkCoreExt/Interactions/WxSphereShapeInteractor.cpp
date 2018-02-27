/*===================================================================

* \author Guo Chu
* \date Jan,2018

===================================================================*/

#include "WxSphereShapeInteractor.h"
#include "Rendering/WxBoundingShapeUtil.h"
#include "Rendering/WxSphereShapeVtkMapper2D.h"
#include "Rendering/WxSphereShapeVtkMapper3D.h"
//mitk
#include <mitkInteractionEventObserver.h>
#include <mitkInteractionKeyEvent.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkSurface.h>
//vtk
#include <vtkPointData.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkPolyData.h>
#include <vtkCamera.h>

// Properties to allow the user to interact with the base data
const char *selectedSphereColorPropertyName = "Sphere Shape.Selected Color";
const char *deselectedSphereColorPropertyName = "Sphere Shape.Deselected Color";
const char *activeSphereHandleIdPropertyName = "Sphere Shape.Active Handle ID";
const char *sphereShapePropertyName = "Sphere Shape";

namespace mitk
{
	class WxSphereShapeInteractor::Impl
	{
	public:
		Impl() : ScrollEnabled(false)
		{

			Point3D initialPoint;
			initialPoint.Fill(0.0);

			//ActiveHandle初始化
			ActiveHandle = Handle(initialPoint, 0, GetHandleIndices(0));
		}

		~Impl() {}
		bool ScrollEnabled;
		Point3D LastPickedWorldPoint;

		Handle ActiveHandle;
		Geometry3D::Pointer OriginalGeometry;
		std::map<us::ServiceReferenceU, mitk::EventConfig> DisplayInteractorConfigs;
	};
}

mitk::WxSphereShapeInteractor::WxSphereShapeInteractor() : 
	m_Impl(new Impl),
	m_pDataStorage(nullptr),
	m_pRenderer(nullptr),
	m_pSphereNode(nullptr),
	m_bInitFlag(false)
{
}

mitk::WxSphereShapeInteractor::~WxSphereShapeInteractor()
{
	delete m_Impl;
}

void mitk::WxSphereShapeInteractor::ConnectActionsAndFunctions()
{
	// **Conditions** that can be used in the state machine, to ensure that certain conditions are met, before actually
	// executing an action
	CONNECT_CONDITION("isHoveringOverObject", CheckOverObject);
	CONNECT_CONDITION("isHoveringOverHandles", CheckOverHandles);

	// **Function** in the statemachine patterns also referred to as **Actions**
	CONNECT_FUNCTION("selectObject", SelectObject);
	CONNECT_FUNCTION("deselectObject", DeselectObject);
	CONNECT_FUNCTION("selectHandle", SelectHandle);	
	CONNECT_FUNCTION("deselectHandles", DeselectHandles);
	CONNECT_FUNCTION("initInteraction", InitInteraction);
	CONNECT_FUNCTION("translateObject", TranslateObject);	
 	CONNECT_FUNCTION("scaleObject", ScaleObject);
}

void mitk::WxSphereShapeInteractor::DataNodeChanged()
{
	mitk::DataNode::Pointer newInputNode = this->GetDataNode();
	if (newInputNode == nullptr)
	{
		if (m_pSphereNode.IsNotNull())
		{
			DataInteractor::EnableInteraction(false);
			mitk::RenderingManager::GetInstance()->RequestUpdateAll();

			mitk::ColorProperty::Pointer color = (mitk::ColorProperty::New(0.8, 0.8, 0.8));
			if (color.IsNotNull())
			{
				m_pSphereNode->GetPropertyList()->SetProperty("color", color);
			}
			m_pSphereNode->SetProperty("layer", mitk::IntProperty::New(99));
			m_pSphereNode->SetProperty(sphereShapePropertyName, mitk::BoolProperty::New(false));
			m_pSphereNode->GetPropertyList()->DeleteProperty(activeSphereHandleIdPropertyName);
		}
	}
	else
	{
		if (m_pSphereNode.IsNotNull())
		{
			DataInteractor::EnableInteraction(true);

			mitk::ColorProperty::Pointer color = (mitk::ColorProperty::New(1.0, 1.0, 1.0));
			if (color.IsNotNull())
			{
				m_pSphereNode->GetPropertyList()->SetProperty("color", color);
			}

			m_pSphereNode->SetProperty(sphereShapePropertyName, mitk::BoolProperty::New(true));
			m_pSphereNode->AddProperty(activeSphereHandleIdPropertyName, mitk::IntProperty::New(-1));
			m_pSphereNode->SetBoolProperty("fixedLayer", mitk::BoolProperty::New(true));

			mitk::RenderingManager::GetInstance()->RequestUpdateAll();
		}
	}
}

void mitk::WxSphereShapeInteractor::SetDataStorage(mitk::DataStorage* dataStorage)
{
	m_pDataStorage = dataStorage;
}

void mitk::WxSphereShapeInteractor::SetRenderer(vtkRenderer* renderer)
{
	m_pRenderer = renderer;
}

bool mitk::WxSphereShapeInteractor::InitMembers(InteractionEvent *interactionEvent)
{
	InteractionPositionEvent *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
	if (positionEvent == nullptr)
		return false;

	m_Impl->LastPickedWorldPoint = positionEvent->GetPositionInWorld();

	return true;
}

void mitk::WxSphereShapeInteractor::Undo(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
	Undo();
}

void mitk::WxSphereShapeInteractor::Redo(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
	Redo();
}

void mitk::WxSphereShapeInteractor::Undo()
{
	UndoEvent.Send();
}

void mitk::WxSphereShapeInteractor::Redo()
{
	RedoEvent.Send();
}

vtkPolyData* mitk::WxSphereShapeInteractor::GetSphere()
{
	if (m_pSphereNode.IsNull())
	{
		return nullptr;
	}
	Surface::Pointer sphereSurface = dynamic_cast<Surface *>(m_pSphereNode->GetData());
	if (sphereSurface.IsNotNull())
	{
		return sphereSurface->GetVtkPolyData();
	}
	return nullptr;
}

void mitk::WxSphereShapeInteractor::HandlePositionChanged(const InteractionEvent *interactionEvent, Point3D &center)
{
	Surface::Pointer sphereSurface = dynamic_cast<Surface *>(m_pSphereNode->GetData());
	if(sphereSurface.IsNull())
	{
		return;
	}
	mitk::BaseGeometry::Pointer sphereGeometry = sphereSurface->GetGeometry();
	std::vector<Point3D> cornerPoints = GetCornerPoints(sphereGeometry, false);
	// set activehandle position
	Point3D pointLeft = CalcAvgPoint(cornerPoints[5], cornerPoints[6]);
	m_Impl->ActiveHandle.SetPosition(pointLeft);
	// calculate center based on half way of the distance between two opposing cornerpoints
	center = CalcAvgPoint(cornerPoints[7], cornerPoints[0]);	
}

bool mitk::WxSphereShapeInteractor::CheckOverObject(const InteractionEvent *interactionEvent)
{
	const InteractionPositionEvent *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
	if (positionEvent == nullptr)
		return false;

	Surface::Pointer sphereSurface = dynamic_cast<Surface *>(m_pSphereNode->GetData());
	if (sphereSurface.IsNull())
	{
		return false;
	}
	mitk::BaseGeometry::Pointer sphereGeometry = sphereSurface->GetGeometry();
	Point3D sphereCenter = sphereGeometry->GetCenter();

	Point2D displayCenterPoint;
	interactionEvent->GetSender()->WorldToDisplay(sphereCenter, displayCenterPoint);

	Point2D currentDisplayPosition = positionEvent->GetPointerPositionOnScreen();
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
	double sphereR = extent[0] / 2;

	Vector3D upNormal;
	positionEvent->GetSender()->GetVtkRenderer()->GetActiveCamera()->GetViewUp(upNormal[0], upNormal[1], upNormal[2]);
	upNormal.Normalize();
	Point3D pointR;
	pointR = sphereCenter + sphereR * upNormal;
	Point2D pointR2D;
	interactionEvent->GetSender()->WorldToDisplay(pointR, pointR2D);
	double sphereR2D = displayCenterPoint.EuclideanDistanceTo(pointR2D);
	bool isInside =
		currentDisplayPosition.EuclideanDistanceTo(displayCenterPoint) < sphereR2D;

	return isInside;
}

bool mitk::WxSphereShapeInteractor::CheckOverHandles(const InteractionEvent *interactionEvent)
{
	Point3D sphereCenter;
	HandlePositionChanged(interactionEvent, sphereCenter);
	const InteractionPositionEvent *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
	if (positionEvent == nullptr)
		return false;

	Surface::Pointer sphereSurface = dynamic_cast<Surface *>(m_pSphereNode->GetData());
	if (sphereSurface.IsNull())
	{
		return false;
	}
	mitk::BaseGeometry::Pointer sphereGeometry = sphereSurface->GetGeometry();

	Point2D displayCenterPoint;
	interactionEvent->GetSender()->WorldToDisplay(sphereCenter, displayCenterPoint);
	double scale = interactionEvent->GetSender()->GetScaleFactorMMPerDisplayUnit(); // GetDisplaySizeInMM
	mitk::DoubleProperty::Pointer handleSizeProperty =
		dynamic_cast<mitk::DoubleProperty *>(m_pSphereNode->GetProperty("Sphere Shape.Handle Size Factor"));

	ScalarType initialHandleSize;
	if (handleSizeProperty != nullptr)
		initialHandleSize = handleSizeProperty->GetValue();
	else
		initialHandleSize = 1.0 / 40.0;

	mitk::Point2D displaysize = interactionEvent->GetSender()->GetDisplaySizeInMM();
	ScalarType handlesize = ((displaysize[0] + displaysize[1]) / 2.0) * initialHandleSize;

	Point2D centerpoint;
	interactionEvent->GetSender()->WorldToDisplay(m_Impl->ActiveHandle.GetPosition(), centerpoint);
	Point2D currentDisplayPosition = positionEvent->GetPointerPositionOnScreen();

	if ((currentDisplayPosition.EuclideanDistanceTo(centerpoint) < (handlesize / scale)) &&
		(currentDisplayPosition.EuclideanDistanceTo(displayCenterPoint) >
		(handlesize / scale))) // check if mouse is hovering over center point
	{
		m_Impl->ActiveHandle.SetActive(true);
		m_pSphereNode->GetPropertyList()->SetProperty(activeSphereHandleIdPropertyName,
			mitk::IntProperty::New(0));
		m_pSphereNode->GetData()->Modified();
		interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();
		return true;
	}
	else
	{
		m_Impl->ActiveHandle.SetActive(false);
	}
	m_pSphereNode->GetPropertyList()->SetProperty(activeSphereHandleIdPropertyName, mitk::IntProperty::New(-1));

	return false;
}

void mitk::WxSphereShapeInteractor::SelectObject(StateMachineAction *, InteractionEvent *)
{
	if (m_pSphereNode.IsNull())
		return;

	mitk::ColorProperty::Pointer selectedColor =
		dynamic_cast<mitk::ColorProperty *>(m_pSphereNode->GetProperty(selectedSphereColorPropertyName));
	if (selectedColor.IsNotNull())
	{
		m_pSphereNode->GetPropertyList()->SetProperty("color", selectedColor);
	}
	m_pSphereNode->UpdateOutputInformation(); // Geometry is up-to-date
	m_pSphereNode->GetData()->Modified();
	mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::WxSphereShapeInteractor::DeselectObject(StateMachineAction *, InteractionEvent *interactionEvent)
{
	if (m_pSphereNode.IsNull())
		return;

	mitk::ColorProperty::Pointer deselectedColor =
		dynamic_cast<mitk::ColorProperty *>(m_pSphereNode->GetProperty(deselectedSphereColorPropertyName));
	if (deselectedColor.IsNotNull())
	{
		m_pSphereNode->GetPropertyList()->SetProperty("color", deselectedColor);
	}

	m_pSphereNode->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
	m_pSphereNode->GetData()->Modified();
	interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();
}

void mitk::WxSphereShapeInteractor::SelectHandle(StateMachineAction *, InteractionEvent *interactionEvent)
{
	if (m_pSphereNode.IsNull())
		return;

	mitk::ColorProperty::Pointer selectedColor =
		dynamic_cast<mitk::ColorProperty *>(m_pSphereNode->GetProperty(deselectedSphereColorPropertyName));
	if (selectedColor.IsNotNull())
	{
		m_pSphereNode->GetPropertyList()->SetProperty("color", selectedColor);
	}
	m_pSphereNode->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
	m_pSphereNode->GetData()->Modified();
	interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();
	return;
}

void mitk::WxSphereShapeInteractor::DeselectHandles(StateMachineAction *, InteractionEvent *interactionEvent)
{
	if (m_pSphereNode.IsNull())
		return;

	m_pSphereNode->GetPropertyList()->SetProperty(activeSphereHandleIdPropertyName, mitk::IntProperty::New(-1));
	m_pSphereNode->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
	m_pSphereNode->GetData()->Modified();
	interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();
}

void mitk::WxSphereShapeInteractor::SetDataNode(DataNode *node)
{
	DataInteractor::SetDataNode(node); // calls DataNodeChanged internally
	Init();
}

void mitk::WxSphereShapeInteractor::Init()
{
	if (m_pDataStorage && (!m_bInitFlag))
	{
		m_bInitFlag = true;

		mitk::BaseGeometry::Pointer geometry = static_cast<mitk::BaseGeometry*>(this->GetDataNode()->GetData()->GetGeometry());
		if (geometry.IsNull())
		{
			return;
		}
		double center[3];
		center[0] = geometry->GetCenter()[0];
		center[1] = geometry->GetCenter()[1];
		center[2] = geometry->GetCenter()[2];
		vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
		sphere->SetCenter(center);
		mitk::Point3D BBmin = geometry->GetIndexToWorldTransform()->TransformPoint(geometry->GetBoundingBox()->GetMinimum());
		mitk::Point3D BBmax = geometry->GetIndexToWorldTransform()->TransformPoint(geometry->GetBoundingBox()->GetMaximum());
		double r = (BBmax[0] - BBmin[0]) / 2;
		sphere->SetRadius(r);
		sphere->SetPhiResolution(30);
		sphere->SetThetaResolution(30);
		sphere->Update();
 		mitk::Surface::Pointer surface = mitk::Surface::New();
 		surface->SetVtkPolyData(sphere->GetOutput());
		m_pSphereNode = mitk::DataNode::New();
		m_pSphereNode->SetData(surface);

		auto wxMapper3D = mitk::WxSphereShapeVtkMapper3D::New();
		m_pSphereNode->SetMapper(mitk::BaseRenderer::Standard3D, wxMapper3D);
		auto wxMapper2D = mitk::WxSphereShapeVtkMapper2D::New();
		m_pSphereNode->SetMapper(mitk::BaseRenderer::Standard2D, wxMapper2D);

		m_pSphereNode->SetProperty("helper object", mitk::BoolProperty::New(true));
		m_pSphereNode->SetProperty("name", mitk::StringProperty::New("Sphere"));
		m_pSphereNode->SetProperty("color", mitk::ColorProperty::New(1.0, 1.0, 1.0));
		m_pSphereNode->SetProperty("opacity", mitk::FloatProperty::New(0.3));
		m_pSphereNode->SetProperty("layer", mitk::IntProperty::New(99));
		m_pSphereNode->AddProperty("handle size factor", mitk::DoubleProperty::New(1.0 / 40.0));
		m_pSphereNode->SetBoolProperty("pickable", true);

		// add color properties
		mitk::ColorProperty::Pointer selectedColor =
			dynamic_cast<mitk::ColorProperty *>(m_pSphereNode->GetProperty(selectedSphereColorPropertyName));
		mitk::ColorProperty::Pointer deselectedColor =
			dynamic_cast<mitk::ColorProperty *>(m_pSphereNode->GetProperty(deselectedSphereColorPropertyName));

		if (selectedColor.IsNull())
			m_pSphereNode->AddProperty(selectedSphereColorPropertyName, mitk::ColorProperty::New(0.0, 0.0, 1.0));

		if (deselectedColor.IsNull())
			m_pSphereNode->AddProperty(deselectedSphereColorPropertyName, mitk::ColorProperty::New(1.0, 1.0, 1.0));

		mitk::ColorProperty::Pointer initialColor =
			dynamic_cast<mitk::ColorProperty *>(m_pSphereNode->GetProperty(deselectedSphereColorPropertyName));
		if (initialColor.IsNotNull())
		{
			m_pSphereNode->SetColor(initialColor->GetColor());
		}

		m_pSphereNode->SetProperty(sphereShapePropertyName, mitk::BoolProperty::New(true));
		m_pSphereNode->AddProperty(activeSphereHandleIdPropertyName, mitk::IntProperty::New(-1));
		m_pSphereNode->SetBoolProperty("fixedLayer", mitk::BoolProperty::New(true));

		m_pDataStorage->Add(m_pSphereNode);
		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}
}

void mitk::WxSphereShapeInteractor::InitInteraction(StateMachineAction *, InteractionEvent *interactionEvent)
{
	InitMembers(interactionEvent);
}

void mitk::WxSphereShapeInteractor::TranslateObject(StateMachineAction *, InteractionEvent *interactionEvent)
{
	InteractionPositionEvent *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
	if (positionEvent == nullptr)
		return;

	Surface::Pointer sphereSurface = dynamic_cast<Surface *>(m_pSphereNode->GetData());
	if (sphereSurface.IsNull())
	{
		return;
	}
	mitk::BaseGeometry::Pointer sphereGeometry = sphereSurface->GetGeometry();

	Point3D sphereCenter = sphereGeometry->GetCenter();
	Point3D currentWorldPosition = positionEvent->GetPositionInWorld();
	Vector3D interactionMove;

	interactionMove[0] = currentWorldPosition[0] - m_Impl->LastPickedWorldPoint[0];
	interactionMove[1] = currentWorldPosition[1] - m_Impl->LastPickedWorldPoint[1];
	interactionMove[2] = currentWorldPosition[2] - m_Impl->LastPickedWorldPoint[2];

	if ((interactionMove[0] + interactionMove[1] + interactionMove[2]) !=
		0.0) // only update current position if a movement occured
	{
		m_Impl->LastPickedWorldPoint = currentWorldPosition;

		auto newSphere = vtkSmartPointer<vtkSphereSource>::New();
		mitk::Point3D newCenter;
		newCenter[0] = sphereCenter[0] + interactionMove[0];
		newCenter[1] = sphereCenter[1] + interactionMove[1];
		newCenter[2] = sphereCenter[2] + interactionMove[2];
		newSphere->SetCenter(newCenter[0], newCenter[1], newCenter[2]);
		std::vector<Point3D> cornerPoints = GetCornerPoints(sphereGeometry, false);
		double r = floor(((cornerPoints[4])[0] - (cornerPoints[0])[0]) / 2) + 1;
		newSphere->SetRadius(r);
		newSphere->SetPhiResolution(30);
		newSphere->SetThetaResolution(30);
		newSphere->Update();
		sphereSurface->SetVtkPolyData(newSphere->GetOutput());

		m_pSphereNode->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
		m_pSphereNode->GetData()->Modified();
		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
// 		//改变mitkCrosshair当前焦点
// 		const BaseRenderer::Pointer sender = interactionEvent->GetSender();
// 		auto renWindows = sender->GetRenderingManager()->GetAllRegisteredRenderWindows();
// 		for (auto renWin : renWindows)
// 		{
// 			BaseRenderer::GetInstance(renWin)->GetSliceNavigationController()->SelectSliceByPoint(newCenter);
// 		}
	}
}

void mitk::WxSphereShapeInteractor::ScaleObject(StateMachineAction *, InteractionEvent *interactionEvent)
{
	InteractionPositionEvent *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
	if (positionEvent == nullptr)
		return;

	Surface::Pointer sphereSurface = dynamic_cast<Surface *>(m_pSphereNode->GetData());
	if (sphereSurface.IsNull())
	{
		return;
	}
	mitk::BaseGeometry::Pointer sphereGeometry = sphereSurface->GetGeometry();
	Point3D sphereCenter = sphereGeometry->GetCenter();
	Point3D handlePickedPoint = m_Impl->ActiveHandle.GetPosition();
	Point3D currentWorldPosition = positionEvent->GetPositionInWorld();
	std::vector<Point3D> cornerPoints = GetCornerPoints(sphereGeometry, false);
	double rOld = ((cornerPoints[4])[0] - (cornerPoints[0])[0]) / 2;

	Vector3D interactionMove;
	interactionMove[0] = currentWorldPosition[0] - m_Impl->LastPickedWorldPoint[0];
	interactionMove[1] = currentWorldPosition[1] - m_Impl->LastPickedWorldPoint[1];
	interactionMove[2] = currentWorldPosition[2] - m_Impl->LastPickedWorldPoint[2];	

	Vector3D faceNormal;
	faceNormal[0] = handlePickedPoint[0] - sphereCenter[0];
	faceNormal[1] = handlePickedPoint[1] - sphereCenter[1];
	faceNormal[2] = handlePickedPoint[2] - sphereCenter[2];
	Vector3D faceShiftVector = ((faceNormal * interactionMove) / (faceNormal.GetNorm() * faceNormal.GetNorm())) * faceNormal;
	double faceShift = (faceNormal * interactionMove) / faceNormal.GetNorm();
	double rNew = rOld + faceShift;
	if ((std::abs(rNew - rOld) > 1) && (rNew >= 1))
	{
		
		auto newSphere = vtkSmartPointer<vtkSphereSource>::New();
		newSphere->SetCenter(sphereCenter[0], sphereCenter[1], sphereCenter[2]);
		newSphere->SetRadius(rNew);
		newSphere->SetPhiResolution(30);
		newSphere->SetThetaResolution(30);
		newSphere->Update();
		sphereSurface->SetVtkPolyData(newSphere->GetOutput());

		mitk::BaseGeometry::Pointer sphereGeometryNew = sphereSurface->GetGeometry();
		std::vector<Point3D> cornerPointsNew = GetCornerPoints(sphereGeometry, false);
		m_Impl->LastPickedWorldPoint = cornerPointsNew[5];

		m_pSphereNode->UpdateOutputInformation(); // Geometry is up-to-date
		m_pSphereNode->GetData()->Modified();
		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}
}
