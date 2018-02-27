/*===================================================================

* \author Guo Chu
* \date Jan,2018

===================================================================*/

#include "Rendering/WxBoundingShapeUtil.h"
#include "WxBoundingShapeInteractor.h"
#include "Rendering/WxBoundingShapeVtkMapper2D.h"
#include "Rendering/WxBoundingShapeVtkMapper3D.h"
//mitk
#include <mitkDisplayInteractor.h>
#include <mitkInteractionConst.h>
#include <mitkInteractionEventObserver.h>
#include <mitkInteractionKeyEvent.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkSurface.h>
//vtk
#include <vtkCamera.h>
#include <vtkInteractorObserver.h>
#include <vtkInteractorStyle.h>
#include <vtkPointData.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkCubeSource.h>
#include <vtkRenderer.h>

// Properties to allow the user to interact with the base data
const char *selectedColorPropertyName = "Bounding Shape.Selected Color";
const char *deselectedColorPropertyName = "Bounding Shape.Deselected Color";
const char *activeHandleIdPropertyName = "Bounding Shape.Active Handle ID";
const char *boundingShapePropertyName = "Bounding Shape";

namespace mitk
{
  class WxBoundingShapeInteractor::Impl
  {
  public:
    Impl()
    {
      Point3D initialPoint;
      initialPoint.Fill(0.0);

      for (int i = 0; i < 6; ++i)
        Handles.push_back(Handle(initialPoint, i, GetHandleIndices(i)));
    }

    ~Impl() {}
    Point3D LastPickedWorldPoint;
    std::vector<Handle> Handles;
    Handle ActiveHandle;
  };
}

mitk::WxBoundingShapeInteractor::WxBoundingShapeInteractor() : 
	m_Impl(new Impl),	
	m_pDataStorage(nullptr),
	m_pRenderer(nullptr),
	m_pCubeNode(nullptr),
	m_bInitFlag(false)
{
}

mitk::WxBoundingShapeInteractor::~WxBoundingShapeInteractor()
{
  delete m_Impl;
}

void mitk::WxBoundingShapeInteractor::ConnectActionsAndFunctions()
{
  // **Conditions** that can be used in the state machine, to ensure that certain conditions are met, before actually
  // executing an action
  CONNECT_CONDITION("isHoveringOverObject", CheckOverObject);
  CONNECT_CONDITION("isHoveringOverHandles", CheckOverHandles);

  // **Function** in the statemachine patterns also referred to as **Actions**
  CONNECT_FUNCTION("selectObject", SelectObject);
  CONNECT_FUNCTION("deselectObject", DeselectObject);
  CONNECT_FUNCTION("deselectHandles", DeselectHandles);
  CONNECT_FUNCTION("selectHandle", SelectHandle);
	CONNECT_FUNCTION("initInteraction", InitInteraction);
	CONNECT_FUNCTION("translateObject", TranslateObject);
  CONNECT_FUNCTION("scaleObject", ScaleObject);
}

void mitk::WxBoundingShapeInteractor::DataNodeChanged()
{
  mitk::DataNode::Pointer newInputNode = this->GetDataNode();

	if (newInputNode == nullptr)
	{
		if (m_pCubeNode.IsNotNull())
		{
//			m_pCubeNode->SetVisibility(false);
			DataInteractor::EnableInteraction(false);
			mitk::RenderingManager::GetInstance()->RequestUpdateAll();

			mitk::ColorProperty::Pointer color = (mitk::ColorProperty::New(0.8, 0.8, 0.8));
			if (color.IsNotNull())
			{
				m_pCubeNode->GetPropertyList()->SetProperty("color", color);
			}
			m_pCubeNode->SetProperty("layer", mitk::IntProperty::New(99));
			m_pCubeNode->SetProperty(boundingShapePropertyName, mitk::BoolProperty::New(false));
			m_pCubeNode->GetPropertyList()->DeleteProperty(activeHandleIdPropertyName);
		}
	}
	else
	{
		if (m_pCubeNode.IsNotNull())
		{
//			m_pCubeNode->SetVisibility(true);
			DataInteractor::EnableInteraction(true);

			mitk::ColorProperty::Pointer color = (mitk::ColorProperty::New(1.0, 1.0, 1.0));
			if (color.IsNotNull())
			{
				m_pCubeNode->GetPropertyList()->SetProperty("color", color);
			}

			m_pCubeNode->SetProperty(boundingShapePropertyName, mitk::BoolProperty::New(true));
			m_pCubeNode->AddProperty(activeHandleIdPropertyName, mitk::IntProperty::New(-1));
			m_pCubeNode->SetBoolProperty("fixedLayer", mitk::BoolProperty::New(true));

			mitk::RenderingManager::GetInstance()->RequestUpdateAll();
		}
	}
}

void mitk::WxBoundingShapeInteractor::HandlePositionChanged(const InteractionEvent *interactionEvent, Point3D &center)
{
	Surface::Pointer cubeSurface = dynamic_cast<Surface *>(m_pCubeNode->GetData());
	if (cubeSurface.IsNull())
	{
		return;
	}
	mitk::BaseGeometry::Pointer cubeGeometry = cubeSurface->GetGeometry();

  std::vector<Point3D> cornerPoints = GetCornerPoints(cubeGeometry, true);
  if (m_Impl->Handles.size() == 6)
  {
    // set handle positions
    Point3D pointLeft = CalcAvgPoint(cornerPoints[5], cornerPoints[6]);
    Point3D pointRight = CalcAvgPoint(cornerPoints[1], cornerPoints[2]);
    Point3D pointTop = CalcAvgPoint(cornerPoints[0], cornerPoints[6]);
    Point3D pointBottom = CalcAvgPoint(cornerPoints[7], cornerPoints[1]);
    Point3D pointFront = CalcAvgPoint(cornerPoints[2], cornerPoints[7]);
    Point3D pointBack = CalcAvgPoint(cornerPoints[4], cornerPoints[1]);

    m_Impl->Handles[0].SetPosition(pointLeft);
    m_Impl->Handles[1].SetPosition(pointRight);
    m_Impl->Handles[2].SetPosition(pointTop);
    m_Impl->Handles[3].SetPosition(pointBottom);
    m_Impl->Handles[4].SetPosition(pointFront);
    m_Impl->Handles[5].SetPosition(pointBack);

    // calculate center based on half way of the distance between two opposing cornerpoints
    center = CalcAvgPoint(cornerPoints[7], cornerPoints[0]);
  }
}

void mitk::WxBoundingShapeInteractor::SetDataStorage(mitk::DataStorage* dataStorage)
{
	m_pDataStorage = dataStorage;
}

void mitk::WxBoundingShapeInteractor::SetRenderer(vtkRenderer* renderer)
{
	m_pRenderer = renderer;
}

void mitk::WxBoundingShapeInteractor::SetDataNode(DataNode *node)
{
  DataInteractor::SetDataNode(node); // calls DataNodeChanged internally
	Init();
}

void mitk::WxBoundingShapeInteractor::Init()
{
	if (m_pDataStorage && (!m_bInitFlag))
	{
		m_bInitFlag = true;

		mitk::BaseGeometry::Pointer Geometry = static_cast<mitk::BaseGeometry*>(this->GetDataNode()->GetData()->GetGeometry());
		if (Geometry.IsNull())
		{
			return;
		}
		auto cube = vtkSmartPointer<vtkCubeSource>::New();
		double center[3];
		center[0] = Geometry->GetCenter()[0];
		center[1] = Geometry->GetCenter()[1];
		center[2] = Geometry->GetCenter()[2];
		cube->SetCenter(center);
		mitk::Point3D BBmin = Geometry->GetIndexToWorldTransform()->TransformPoint(Geometry->GetBoundingBox()->GetMinimum());
		mitk::Point3D BBmax = Geometry->GetIndexToWorldTransform()->TransformPoint(Geometry->GetBoundingBox()->GetMaximum());
		cube->SetBounds(BBmin[0], BBmax[0], BBmin[1], BBmax[1], BBmin[2], BBmax[2]);
		cube->Update();

		mitk::Surface::Pointer surface = mitk::Surface::New();
		surface->SetVtkPolyData(cube->GetOutput());
		m_pCubeNode = mitk::DataNode::New();
		m_pCubeNode->SetData(surface);

		auto wxMapper2D = mitk::WxBoundingShapeVtkMapper2D::New();
		m_pCubeNode->SetMapper(mitk::BaseRenderer::Standard2D, wxMapper2D);
		auto wxMapper3D = mitk::WxBoundingShapeVtkMapper3D::New();
		m_pCubeNode->SetMapper(mitk::BaseRenderer::Standard3D, wxMapper3D);

		m_pCubeNode->SetProperty("helper object", mitk::BoolProperty::New(true));
		m_pCubeNode->SetProperty("name", mitk::StringProperty::New("Cube"));
		m_pCubeNode->SetProperty("color", mitk::ColorProperty::New(1.0, 1.0, 1.0));
		m_pCubeNode->SetProperty("opacity", mitk::FloatProperty::New(0.3));
		m_pCubeNode->SetProperty("layer", mitk::IntProperty::New(99));
		m_pCubeNode->AddProperty("handle size factor", mitk::DoubleProperty::New(1.0 / 40.0));
		m_pCubeNode->SetBoolProperty("pickable", true);

		// add color properties
		mitk::ColorProperty::Pointer selectedColor =
			dynamic_cast<mitk::ColorProperty *>(m_pCubeNode->GetProperty(selectedColorPropertyName));
		mitk::ColorProperty::Pointer deselectedColor =
			dynamic_cast<mitk::ColorProperty *>(m_pCubeNode->GetProperty(deselectedColorPropertyName));

		if (selectedColor.IsNull())
			m_pCubeNode->AddProperty(selectedColorPropertyName, mitk::ColorProperty::New(0.0, 0.0, 1.0));

		if (deselectedColor.IsNull())
			m_pCubeNode->AddProperty(deselectedColorPropertyName, mitk::ColorProperty::New(1.0, 1.0, 1.0));

		mitk::ColorProperty::Pointer initialColor =
			dynamic_cast<mitk::ColorProperty *>(m_pCubeNode->GetProperty(deselectedColorPropertyName));
		if (initialColor.IsNotNull())
		{
			m_pCubeNode->SetColor(initialColor->GetColor());
		}

		m_pCubeNode->SetProperty(boundingShapePropertyName, mitk::BoolProperty::New(true));
		m_pCubeNode->AddProperty(activeHandleIdPropertyName, mitk::IntProperty::New(-1));
		m_pCubeNode->SetBoolProperty("fixedLayer", mitk::BoolProperty::New(true));

		m_pDataStorage->Add(m_pCubeNode);
 		mitk::RenderingManager::GetInstance()->RequestUpdateAll();		
	}
}

vtkPolyData* mitk::WxBoundingShapeInteractor::GetCube()
{
	if (m_pCubeNode.IsNull())
	{
		return nullptr;
	}
	Surface::Pointer cubeSurface = dynamic_cast<Surface *>(m_pCubeNode->GetData());
	if (cubeSurface.IsNotNull())
	{
		return cubeSurface->GetVtkPolyData();
	}
	return nullptr;
}

void mitk::WxBoundingShapeInteractor::Undo(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
	Undo();
}

void mitk::WxBoundingShapeInteractor::Redo(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
	Redo();
}

void mitk::WxBoundingShapeInteractor::Undo()
{
	UndoEvent.Send();
}

void mitk::WxBoundingShapeInteractor::Redo()
{
	RedoEvent.Send();
}

bool mitk::WxBoundingShapeInteractor::CheckOverObject(const InteractionEvent *interactionEvent)
{
  const InteractionPositionEvent *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return false;

	Point3D currentWorldPosition = positionEvent->GetPositionInWorld();

	Surface::Pointer cubeSurface = dynamic_cast<Surface *>(m_pCubeNode->GetData());
	if (cubeSurface.IsNull())
	{
		return false;
	}

	mitk::BaseGeometry::Pointer cubeGeometry = cubeSurface->GetGeometry();
	std::vector<Point3D> cornerPoints = GetCornerPoints(cubeGeometry,true);
	Point3D p0 = cornerPoints[0];
	Point3D p7 = cornerPoints[7];

	bool isInside = 
		((currentWorldPosition[0] > p0[0]) && (currentWorldPosition[0] < p7[0])) &&
		((currentWorldPosition[1] > p0[1]) && (currentWorldPosition[1] < p7[1])) &&
		((currentWorldPosition[2] > p0[2]) && (currentWorldPosition[2] < p7[2]));							

  return isInside;
}

bool mitk::WxBoundingShapeInteractor::CheckOverHandles(const InteractionEvent *interactionEvent)
{
  Point3D boundingBoxCenter;
  HandlePositionChanged(interactionEvent, boundingBoxCenter);
  const InteractionPositionEvent *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return false;

	Surface::Pointer cubeSurface = dynamic_cast<Surface *>(m_pCubeNode->GetData());
	if (cubeSurface.IsNull())
	{
		return false;
	}
	mitk::BaseGeometry::Pointer cubeGeometry = cubeSurface->GetGeometry();

  std::vector<Point3D> cornerPoints = GetCornerPoints(cubeGeometry, true);

	Point2D displayCenterPoint;
  interactionEvent->GetSender()->WorldToDisplay(boundingBoxCenter, displayCenterPoint);
  double scale = interactionEvent->GetSender()->GetScaleFactorMMPerDisplayUnit(); // GetDisplaySizeInMM
  mitk::DoubleProperty::Pointer handleSizeProperty =
    dynamic_cast<mitk::DoubleProperty *>(m_pCubeNode->GetProperty("Bounding Shape.Handle Size Factor"));

  ScalarType initialHandleSize;
  if (handleSizeProperty != nullptr)
    initialHandleSize = handleSizeProperty->GetValue();
  else
    initialHandleSize = 1.0 / 40.0;

  mitk::Point2D displaysize = interactionEvent->GetSender()->GetDisplaySizeInMM();
  ScalarType handlesize = ((displaysize[0] + displaysize[1]) / 2.0) * initialHandleSize;
  unsigned int handleNum = 0;

  for (auto &handle : m_Impl->Handles)
  {
    Point2D centerpoint;
    interactionEvent->GetSender()->WorldToDisplay(handle.GetPosition(), centerpoint);
    Point2D currentDisplayPosition = positionEvent->GetPointerPositionOnScreen();

    if ((currentDisplayPosition.EuclideanDistanceTo(centerpoint) < (handlesize / scale)) &&
        (currentDisplayPosition.EuclideanDistanceTo(displayCenterPoint) >
         (handlesize / scale))) // check if mouse is hovering over center point
    {
      handle.SetActive(true);
      m_Impl->ActiveHandle = handle;
			m_pCubeNode->GetPropertyList()->SetProperty(activeHandleIdPropertyName,
                                                          mitk::IntProperty::New(handleNum++));
			m_pCubeNode->GetData()->Modified();
      interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();
      return true;
    }
    else
    {
      handleNum++;
      handle.SetActive(false);
    }
		m_pCubeNode->GetPropertyList()->SetProperty(activeHandleIdPropertyName, mitk::IntProperty::New(-1));
  }
  return false;
}

void mitk::WxBoundingShapeInteractor::SelectHandle(StateMachineAction *, InteractionEvent *interactionEvent)
{
	if (m_pCubeNode)
	{
		mitk::ColorProperty::Pointer selectedColor =
			dynamic_cast<mitk::ColorProperty *>(m_pCubeNode->GetProperty(deselectedColorPropertyName));
		if (selectedColor.IsNotNull())
		{
			m_pCubeNode->GetPropertyList()->SetProperty("color", selectedColor);
		}
		m_pCubeNode->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
		m_pCubeNode->GetData()->Modified();
		interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();
	}
}

void mitk::WxBoundingShapeInteractor::DeselectHandles(StateMachineAction *, InteractionEvent *interactionEvent)
{
	if (m_pCubeNode)
	{
		m_pCubeNode->GetPropertyList()->SetProperty(activeHandleIdPropertyName, mitk::IntProperty::New(-1));
		m_pCubeNode->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
		m_pCubeNode->GetData()->Modified();
		interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();
	}
}

void mitk::WxBoundingShapeInteractor::SelectObject(StateMachineAction *, InteractionEvent *)
{
	if (m_pCubeNode)
	{
		mitk::ColorProperty::Pointer selectedColor =
			dynamic_cast<mitk::ColorProperty *>(m_pCubeNode->GetProperty(selectedColorPropertyName));
		if (selectedColor.IsNotNull())
		{
			m_pCubeNode->GetPropertyList()->SetProperty("color", selectedColor);
		}
		m_pCubeNode->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
		m_pCubeNode->GetData()->Modified();
		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}
}

void mitk::WxBoundingShapeInteractor::DeselectObject(StateMachineAction *, InteractionEvent *interactionEvent)
{
	if (m_pCubeNode)
	{
		mitk::ColorProperty::Pointer deselectedColor =
			dynamic_cast<mitk::ColorProperty *>(m_pCubeNode->GetProperty(deselectedColorPropertyName));
		if (deselectedColor.IsNotNull())
		{
			m_pCubeNode->GetPropertyList()->SetProperty("color", deselectedColor);
		}

		m_pCubeNode->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
		m_pCubeNode->GetData()->Modified();
		interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();
	}
}

void mitk::WxBoundingShapeInteractor::InitInteraction(StateMachineAction *, InteractionEvent *interactionEvent)
{
  InitMembers(interactionEvent);
}

bool mitk::WxBoundingShapeInteractor::InitMembers(InteractionEvent *interactionEvent)
{
  InteractionPositionEvent *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return false;

  // get initial position coordinates
  m_Impl->LastPickedWorldPoint = positionEvent->GetPositionInWorld();

  return true;
}

void mitk::WxBoundingShapeInteractor::TranslateObject(StateMachineAction *, InteractionEvent *interactionEvent)
{
  InteractionPositionEvent *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return;

	Surface::Pointer cubeSurface = dynamic_cast<Surface *>(m_pCubeNode->GetData());
	if (cubeSurface.IsNull())
	{
		return;
	}
	mitk::BaseGeometry::Pointer cubeGeometry = cubeSurface->GetGeometry();

	Point3D cubeCenter = cubeGeometry->GetCenter();
	Point3D currentWorldPosition = positionEvent->GetPositionInWorld();
	Vector3D interactionMove;
	interactionMove[0] = currentWorldPosition[0] - m_Impl->LastPickedWorldPoint[0];
	interactionMove[1] = currentWorldPosition[1] - m_Impl->LastPickedWorldPoint[1];
	interactionMove[2] = currentWorldPosition[2] - m_Impl->LastPickedWorldPoint[2];

	if ((interactionMove[0] + interactionMove[1] + interactionMove[2]) !=
		0.0) // only update current position if a movement occured
	{
		m_Impl->LastPickedWorldPoint = currentWorldPosition;
		
		mitk::Point3D newCenter;
		newCenter[0] = cubeCenter[0] + interactionMove[0];
		newCenter[1] = cubeCenter[1] + interactionMove[1];
		newCenter[2] = cubeCenter[2] + interactionMove[2];
		auto newCube = vtkSmartPointer<vtkCubeSource>::New();
		newCube->SetCenter(newCenter[0], newCenter[1], newCenter[2]);
		std::vector<Point3D> cornerPoints = GetCornerPoints(cubeGeometry,false);
		newCube->SetBounds(cornerPoints[0][0] + interactionMove[0], cornerPoints[7][0] + interactionMove[0], cornerPoints[0][1] + interactionMove[1], cornerPoints[7][1] + interactionMove[1], cornerPoints[0][2] + interactionMove[2], cornerPoints[7][2] + interactionMove[2]);
		newCube->Update();
		cubeSurface->SetVtkPolyData(newCube->GetOutput());
		m_pCubeNode->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
		m_pCubeNode->GetData()->Modified();
		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}
}

void mitk::WxBoundingShapeInteractor::ScaleObject(StateMachineAction *, InteractionEvent *interactionEvent)
{
  InteractionPositionEvent *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return;

	Surface::Pointer cubeSurface = dynamic_cast<Surface *>(m_pCubeNode->GetData());
	if (cubeSurface.IsNull())
	{
		return;
	}
	mitk::BaseGeometry::Pointer cubeGeometry = cubeSurface->GetGeometry();

	Point3D handlePickedPoint = m_Impl->ActiveHandle.GetPosition();
	Point3D currentWorldPosition = positionEvent->GetPositionInWorld();
	Vector3D interactionMove;
	interactionMove[0] = currentWorldPosition[0] - m_Impl->LastPickedWorldPoint[0];
	interactionMove[1] = currentWorldPosition[1] - m_Impl->LastPickedWorldPoint[1];
	interactionMove[2] = currentWorldPosition[2] - m_Impl->LastPickedWorldPoint[2];

	std::vector<int> faces = m_Impl->ActiveHandle.GetFaceIndices();
	auto pointscontainer = mitk::BoundingBox::PointsContainer::New();

	std::vector<Point3D> cornerPoints = GetCornerPoints(cubeGeometry, true);
	unsigned int num = 0;
	for (auto point : cornerPoints)
	{
		pointscontainer->InsertElement(num++, point);
	}
	mitk::Point3D center = CalcAvgPoint(cornerPoints[7], cornerPoints[0]);

	Vector3D faceNormal;
	faceNormal[0] = handlePickedPoint[0] - center[0];
	faceNormal[1] = handlePickedPoint[1] - center[1];
	faceNormal[2] = handlePickedPoint[2] - center[2];
	Vector3D faceShift = ((faceNormal * interactionMove) / (faceNormal.GetNorm() * faceNormal.GetNorm())) * faceNormal;

	cornerPoints = GetCornerPoints(cubeGeometry, false);
	num = 0;
	for (auto point : cornerPoints)
	{
		pointscontainer->InsertElement(num++, point);
	}

	bool positionChangeThreshold = true;

	for (int numFaces = 0; numFaces < 8; numFaces++) // estimate the corresponding face and shift its assigned points
	{
		if ((numFaces != faces[0]) && (numFaces != faces[1]) && (numFaces != faces[2]) && (numFaces != faces[3]))
		{
			Point3D point = pointscontainer->GetElement(numFaces);
			point[0] += faceShift[0];
			point[1] += faceShift[1];
			point[2] += faceShift[2];			

			if (point == pointscontainer->GetElement(numFaces))
				positionChangeThreshold = false;
			else
				m_Impl->LastPickedWorldPoint = point;

			pointscontainer->InsertElement(numFaces, point);
		}
	}

	if (positionChangeThreshold)
	{
		mitk::Point3D newCenter = CalcAvgPoint(pointscontainer->ElementAt(7), pointscontainer->ElementAt(0));
		auto newCube = vtkSmartPointer<vtkCubeSource>::New();
		newCube->SetCenter(newCenter[0], newCenter[1], newCenter[2]);
		double minBound[3];
		minBound[0] = pointscontainer->ElementAt(0)[0];
		minBound[1] = pointscontainer->ElementAt(0)[1];
		minBound[2] = pointscontainer->ElementAt(0)[2];
		double maxBound[3];
		maxBound[0] = pointscontainer->ElementAt(7)[0];
		maxBound[1] = pointscontainer->ElementAt(7)[1];
		maxBound[2] = pointscontainer->ElementAt(7)[2];
		newCube->SetBounds(minBound[0], maxBound[0], minBound[1], maxBound[1], minBound[2], maxBound[2]);
		newCube->Update();
		cubeSurface->SetVtkPolyData(newCube->GetOutput());
		m_pCubeNode->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
		m_pCubeNode->GetData()->Modified();
		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}
}

