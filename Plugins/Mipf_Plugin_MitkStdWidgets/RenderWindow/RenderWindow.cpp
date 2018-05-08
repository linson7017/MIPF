#include "RenderWindow.h"

//qmitk
#include "QmitkRenderWindow.h"
#include "QmitkStepperAdapter.h"

//mitk
#include "mitkPlaneGeometryDataMapper2D.h"

//vtk
#include "vtkCornerAnnotation.h"
#include "vtkTextProperty.h"
#include "vtkMitkRectangleProp.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkMath.h"

#include "Rendering/OrientationMarkerVtkMapper3D.h"

#include <QWheelEvent>

#include "Res/R.h"



RenderWindow::RenderWindow()
{

}

RenderWindow::~RenderWindow()
{

}

void RenderWindow::CreateView()
{
    GetRenderer()->SetDataStorage(GetDataStorage());
   
    int size = m_attributes.size();

    if (HasAttribute("mapperID"))
    {
        std::string mapperIDStr = GetAttribute("mapperID");
        if (mapperIDStr.compare("Standard3D")==0)
        {
            GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);
        }
    }

    if (HasAttribute("direction"))
    {
        QString direction = GetAttribute("direction");
        if (direction.compare("Axial",Qt::CaseInsensitive)==0)
        {
            GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Axial);
        }
        else  if (direction.compare("Sagittal", Qt::CaseInsensitive) == 0)
        {
            GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Sagittal);
        }
        else if (direction.compare("Frontal", Qt::CaseInsensitive) == 0)
        {
            GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Frontal);
        }
        else if (direction.compare("Original", Qt::CaseInsensitive) == 0)
        {
            GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Original);
        }
    }
    
    vtkRenderer *renderer = GetRenderer()->GetVtkRenderer();
    if (!renderer)
        return;

    if (HasAttribute("cornerText0")|| HasAttribute("cornerText1")|| HasAttribute("cornerText2")|| HasAttribute("cornerText3"))
    {
        m_annotation = vtkSmartPointer<vtkCornerAnnotation>::New();
        m_annotation->SetMaximumFontSize(12);
        if (HasAttribute("cornerText0"))
        {
            m_annotation->SetText(0, GetAttribute("cornerText0"));
        }
        if (HasAttribute("cornerText1"))
        {
            m_annotation->SetText(1, GetAttribute("cornerText1"));
        }
        if (HasAttribute("cornerText2"))
        {
            m_annotation->SetText(2, GetAttribute("cornerText2"));
        }
        if (HasAttribute("cornerText3"))
        {
            m_annotation->SetText(3, GetAttribute("cornerText3"));
        }
        if (HasAttribute("cornerTextColor"))
        {
            QString colorStr = GetAttribute("cornerTextColor");
            QStringList color = colorStr.split(",");
            if (color.size()==3)
            {
                m_annotation->GetTextProperty()->SetColor(color[0].toDouble(), color[1].toDouble(), color[2].toDouble());
            }
        }       
        if (!renderer->HasViewProp(m_annotation))
        {
            renderer->AddViewProp(m_annotation);
        }
    }
    vtkSmartPointer<vtkMitkRectangleProp> frame = vtkSmartPointer<vtkMitkRectangleProp>::New();
    if (HasAttribute("rectangleColor"))
    {
        QString colorStr = GetAttribute("rectangleColor");
        QStringList color = colorStr.split(",");
        if (color.size() == 3)
        {
            frame->SetColor(color[0].toDouble(), color[1].toDouble(), color[2].toDouble());
        }
    }
    else
    {
        frame->SetColor(0.0, 0.0, 0.0);
    }
    if (!renderer->HasViewProp(frame))
    {
        renderer->AddViewProp(frame);
    }

    if (HasAttribute("colorRectangleOn"))
    {
        QString b = GetAttribute("colorRectangleOn");
        frame->SetVisibility(b.compare("true",Qt::CaseInsensitive)==0);
    }

    if (HasAttribute("stdPlaneWidgetsOn"))
    {
        QString b = GetAttribute("stdPlaneWidgetsOn");
        bool visible = b.compare("true", Qt::CaseInsensitive) == 0;
        mitk::DataNode* planeNode = GetDataStorage()->GetNamedNode("Widgets");
        if (planeNode)
        {
            planeNode->SetVisibility(visible, GetRenderer());
        } 
    }

    //set orientation marker
    if (HasAttribute("Orientation-Marker"))
    {
        mitk::DataNode* insideMarkerNode = GetDataStorage()->GetNamedNode("orientation marker");
        if (insideMarkerNode)
        {
            insideMarkerNode->SetVisibility(true,GetRenderer());
            return;
        }
        auto reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
        reader->SetFileName(R::Instance()->getImageResourceUrl(GetAttribute("Orientation-Marker")).c_str());
        reader->Update();
        vtkPolyData* polyData = reader->GetOutput();
        if (polyData)
        {
            double bounds[6];
            polyData->GetBounds(bounds);
            double scale = 100.0 / vtkMath::Max(vtkMath::Max(bounds[1] - bounds[0], bounds[3] - bounds[2]), bounds[5] - bounds[4]);
            double center[3];
            polyData->GetCenter(center);
            vtkSmartPointer<vtkTransform> translation =
                vtkSmartPointer<vtkTransform>::New();
            translation->Scale(scale, scale, scale);
            translation->Translate(-center[0], -center[1], -center[2]);
            vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter =
                vtkSmartPointer<vtkTransformPolyDataFilter>::New();
            transformFilter->SetInputData(polyData);
            transformFilter->SetTransform(translation);
            transformFilter->Update();

            m_annotation->SetText(0, "");
            mitk::DataNode::Pointer markerNode = mitk::DataNode::New();
            mitk::Surface::Pointer markerSurface = mitk::Surface::New();
            markerSurface->SetVtkPolyData(transformFilter->GetOutput());
            markerNode->SetData(markerSurface);
            markerNode->SetBoolProperty("helper object", true);
            markerNode->SetBoolProperty("includeInBoundingBox", false);
            markerNode->SetName("orientation marker");
            markerNode->SetVisibility(true, GetRenderer());
            mitk::OrientationMarkerVtkMapper3D::Pointer orientationMapper3D = mitk::OrientationMarkerVtkMapper3D::New();
            markerNode->SetMapper(mitk::BaseRenderer::Standard3D, orientationMapper3D);
            markerNode->SetMapper(mitk::BaseRenderer::Standard2D, orientationMapper3D);
            orientationMapper3D->SetDataNode(markerNode);
            GetDataStorage()->Add(markerNode);
        }
    }
    if (HasAttribute("id"))
    {
        GetMitkRenderWindowInterface()->AddMitkRenderWindow(this, GetAttribute("id"));
    }
}

WndHandle RenderWindow::GetPluginHandle()
{
    return this;
}

void RenderWindow::wheelEvent(QWheelEvent *e)
{
    if (GetRenderer()->GetMapperID()==mitk::BaseRenderer::Standard2D)
    {
        int currentSlice = GetSliceNavigationController()->GetSlice()->GetPos()+ e->delta() / 100;
        if (currentSlice<=GetSliceNavigationController()->GetSlice()->GetRangeMax()||
            currentSlice >= GetSliceNavigationController()->GetSlice()->GetRangeMin())
        {
            GetSliceNavigationController()->GetSlice()->SetPos(currentSlice);
            if (m_annotation)
            {
                QString info = QString("Slice: %1").arg(GetSliceNavigationController()->GetSlice()->GetSteps()-currentSlice);
                m_annotation->SetText(0, info.toStdString().c_str());
            } 
        }     
    }
    QmitkRenderWindow::wheelEvent(e);
}

void RenderWindow::mousePressEvent(QMouseEvent *event)
{
    m_dragging = true;
    m_preMousePt = event->pos();
}

void RenderWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
}

void RenderWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging)
    {
        QPoint currentPoint = event->pos();
        QPoint delta = currentPoint - m_preMousePt;
        mitk::DataNode::Pointer node = DetectTopMostVisibleImage();
        if (node.IsNotNull())
        {
            mitk::LevelWindow lw;
            node->GetLevelWindow(lw, GetRenderer());
            lw.SetLevelWindow(lw.GetLevel() + delta.y(), lw.GetWindow() + delta.x());
            node->SetLevelWindow(lw);
            mitk::RenderingManager::GetInstance()->RequestUpdate(GetVtkRenderWindow());
        }
        m_preMousePt = currentPoint;
    } 
}

mitk::DataNode::Pointer RenderWindow::DetectTopMostVisibleImage()
{
   
    if (!GetDataStorage())
    {
        return NULL;
    }
    // get all images from the data storage which are not a segmentation
    auto isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
    auto isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    auto isNotBinary = mitk::NodePredicateNot::New(isBinary);
    auto isNormalImage = mitk::NodePredicateAnd::New(isImage, isNotBinary);
    auto images = GetDataStorage()->GetSubset(isNormalImage);

    mitk::DataNode::Pointer currentNode;
    int maxLayer = std::numeric_limits<int>::min();
    int layer = 0;

    // iterate over selection
    for (auto it = images->Begin(); it != images->End(); ++it)
    {
        auto node = it->Value();

        if (node.IsNull())
            continue;

        if (node->IsVisible(nullptr) == false)
            continue;

        // we also do not want to assign planar figures to helper objects ( even if they are of type image )
        if (node->GetProperty("helper object") != nullptr)
            continue;

        node->GetIntProperty("layer", layer);

        if (layer < maxLayer)
        {
            continue;
        }
        else
        {
            maxLayer = layer;
            currentNode = node;
        }
    }

    return currentNode;
}