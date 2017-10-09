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

#include <QWheelEvent>



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
            QString info = QString("Slice: %1").arg(currentSlice);
            m_annotation->SetText(0, info.toStdString().c_str());
        }     
    }
    QmitkRenderWindow::wheelEvent(e);
}