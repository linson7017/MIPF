#include "ProfileGrayScaleDistributionView.h"

//qwt
#include "qwt_plot_histogram.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_marker.h"
#include "qwt_symbol.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_magnifier.h"

//itk
#include "itkLineIterator.h"

//vtk
#include "vtkLineSource.h"
#include "vtkSTLReader.h"

//QT
#include <QPointF>
#include <QMatrix4x4>


//mitk
#include "mitkImageCast.h"
#include "QmitkStdMultiWidget.h"
#include "mitkMeshMapper2D.h"
#include "mitkContour.h"

#include "CurveProjectionVtkMapper2D.h"
#include "CurveProjectionMapper2D.h"
#include "CustomObjectFactory.h"

//common
#include "ITKImageTypeDef.h"

class ArrowSymbol : public QwtSymbol
{
public:
    ArrowSymbol(bool down=true)
    {
        QPen pen(Qt::black, 0);
        pen.setJoinStyle(Qt::MiterJoin);

        setPen(pen);
        setBrush(Qt::red);

        QPainterPath path;
        path.moveTo(0, 8);
        path.lineTo(0, 5);
        path.lineTo(-3, 5);
        path.lineTo(0, 0);
        path.lineTo(3, 5);
        path.lineTo(0, 5);

        QTransform transform;
        if (down)
        {
            transform.rotate(-30.0);
        }
        else
        {
            transform.rotate(-150.0);
        }
        path = transform.map(path);

        setPath(path);
        setPinPoint(QPointF(0, 0));

        setSize(10, 14);
    }
};

ProfileGrayScaleDistributionView::ProfileGrayScaleDistributionView()
{
    //RegisterCustomObjectFactory();
}


ProfileGrayScaleDistributionView::~ProfileGrayScaleDistributionView()
{
}

void ProfileGrayScaleDistributionView::CreateView()
{
    m_ui.setupUi(this);

    m_ui.ImageSelector->SetDataStorage(GetDataStorage());
    m_ui.ImageSelector->SetPredicate(CreatePredicate(Image));
    {
        QwtPlotCanvas *canvas = new QwtPlotCanvas();
        canvas->setPalette(Qt::gray);
        canvas->setBorderRadius(4);
        m_ui.Plot->setCanvas(canvas);
        m_ui.Plot->plotLayout()->setAlignCanvasToScales(true);
        m_ui.Plot->setAxisTitle(QwtPlot::yLeft, "Gray Value");
        m_ui.Plot->setAxisTitle(QwtPlot::xBottom, "Order");
        QFont font;
        font.setPixelSize(10);
        m_ui.Plot->setAxisFont(QwtPlot::yLeft, font);
        m_ui.Plot->setAxisFont(QwtPlot::xBottom, font);
        //panning
        (void)new QwtPlotPanner(canvas);
        //zooming
        QwtPlotMagnifier *magnifier = new QwtPlotMagnifier(canvas);
        magnifier->setMouseButton(Qt::NoButton);
    }

    //max
    {
        m_maxY = new QwtPlotMarker();
        m_maxY->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
        m_maxY->setLineStyle(QwtPlotMarker::HLine);
        m_maxY->setLinePen(Qt::black, 0, Qt::DashDotLine);
        m_maxY->attach(m_ui.Plot);

        m_maxX = new QwtPlotMarker();
        m_maxX->setLabelAlignment(Qt::AlignLeft | Qt::AlignBottom);
        m_maxX->setLabelOrientation(Qt::Vertical);
        m_maxX->setLineStyle(QwtPlotMarker::VLine);
        m_maxX->setLinePen(Qt::black, 0, Qt::DashDotLine);
        m_maxX->attach(m_ui.Plot);

        m_maxPos = new QwtPlotMarker("Marker");
        m_maxPos->setRenderHint(QwtPlotItem::RenderAntialiased, true);
        m_maxPos->setItemAttribute(QwtPlotItem::Legend, true);
        m_maxPos->setSymbol(new ArrowSymbol(false));
        m_maxPos->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
        m_maxPos->attach(m_ui.Plot);

        m_maxY->setVisible(false);
        m_maxX->setVisible(false);
        m_maxPos->setVisible(false);
    }

    //min
    {
        m_minY = new QwtPlotMarker();
        m_minY->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
        m_minY->setLineStyle(QwtPlotMarker::HLine);
        m_minY->setLinePen(Qt::black, 0, Qt::DashDotLine);
        m_minY->attach(m_ui.Plot);

        m_minX = new QwtPlotMarker();
        m_minX->setLabelAlignment(Qt::AlignLeft | Qt::AlignBottom);
        m_minX->setLabelOrientation(Qt::Vertical);
        m_minX->setLineStyle(QwtPlotMarker::VLine);
        m_minX->setLinePen(Qt::black, 0, Qt::DashDotLine);
        m_minX->attach(m_ui.Plot);

        m_minPos = new QwtPlotMarker("Marker");
        m_minPos->setRenderHint(QwtPlotItem::RenderAntialiased, true);
        m_minPos->setItemAttribute(QwtPlotItem::Legend, true);
        m_minPos->setSymbol(new ArrowSymbol());
        m_minPos->setLabelAlignment(Qt::AlignRight | Qt::AlignBottom);
        m_minPos->attach(m_ui.Plot);

        m_minY->setVisible(false);
        m_minX->setVisible(false);
        m_minPos->setVisible(false);
    }



    m_curve = new QwtPlotCurve("profile");
    m_curve->setPen(Qt::green);
    m_curve->attach(m_ui.Plot);

    connect(m_ui.ClearBtn, SIGNAL(clicked()), this, SLOT(Clear()));
    connect(m_ui.StartBtn, SIGNAL(clicked()), this, SLOT(OnSelectStartPoint()));
    connect(m_ui.EndBtn, SIGNAL(clicked()), this, SLOT(OnSelectEndPoint()));
}

void ProfileGrayScaleDistributionView::Clear()
{

    m_linePoints->Clear();
    m_lineNode->Modified();

    m_ui.StartPointLE->clear();
    m_ui.EndPointLE->clear();

    m_curve->setSamples(NULL, NULL, 0);

    m_maxY->setVisible(false);
    m_maxX->setVisible(false);
    m_maxPos->setVisible(false);

    m_minY->setVisible(false);
    m_minX->setVisible(false);
    m_minPos->setVisible(false);

    m_ui.Plot->replot();

    RequestRenderWindowUpdate();
}

void ProfileGrayScaleDistributionView::OnSelectStartPoint()
{
    m_startPoint = m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetCrossPosition();
    QString pointStr = QString("%1,%2,%3").arg(m_startPoint.GetElement(0)).arg(m_startPoint.GetElement(1)).arg(m_startPoint.GetElement(2));
    m_ui.StartPointLE->setText(pointStr);
    DrawLine();
}

void ProfileGrayScaleDistributionView::OnSelectEndPoint()
{
    m_endPoint = m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetCrossPosition();
    QString pointStr = QString("%1,%2,%3").arg(m_endPoint.GetElement(0)).arg(m_endPoint.GetElement(1)).arg(m_endPoint.GetElement(2));
    m_ui.EndPointLE->setText(pointStr);
    DrawLine();
}

void ProfileGrayScaleDistributionView::DrawLine()
{
      if (m_lineNode.IsNull())
      {

          vtkSTLReader* reader = vtkSTLReader::New();
          reader->SetFileName("D:/temp/SurgicalRoute.stl");
          reader->Update();

          m_lineNode = mitk::DataNode::New();
          mitk::CurveProjectionVtkMapper2D::Pointer mapper = mitk::CurveProjectionVtkMapper2D::New();
          mapper->SetDataNode(m_lineNode);
          m_lineNode->SetMapper(mitk::BaseRenderer::Standard2D, mapper);
          m_lineNode->SetProperty("2d mapper type", mitk::StringProperty::New("curve projection"));

          mitk::Surface::Pointer surface = mitk::Surface::New();
          surface->SetVtkPolyData(reader->GetOutput());

          QMatrix4x4 rotate;
          rotate.setToIdentity();
          rotate.translate(50, 50, -200);
          rotate.rotate(45, 1, 0, 0);   
          vtkMatrix4x4* transform = vtkMatrix4x4::New();
          vtkMatrix4x4* rm = vtkMatrix4x4::New();
          for (int i = 0; i < 4; i++)
          {
              for (int j = 0; j < 4; j++)
              {
                  transform->SetElement(i, j, rotate(i, j));
              }
          }
          vtkMatrix4x4* matrix  = surface->GetGeometry()->GetVtkMatrix();
          vtkMatrix4x4::Multiply4x4(transform, matrix, rm);
          surface->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(rm);

          m_lineNode->SetData(surface);
          
          GetDataStorage()->Add(m_lineNode);
      }

      /*if (!m_ui.StartPointLE->text().isEmpty()&& !m_ui.EndPointLE->text().isEmpty())
      {
          vtkSmartPointer<vtkLineSource> lineSource =
              vtkSmartPointer<vtkLineSource>::New();
          lineSource->SetPoint1(m_startPoint.GetDataPointer());
          lineSource->SetPoint2(m_endPoint.GetDataPointer());
          lineSource->Update();

          mitk::Surface* lineData = static_cast<mitk::Surface*>(m_lineNode->GetData());
          lineData->SetVtkPolyData(lineSource->GetOutput());
          m_lineNode->Modified();
          RequestRenderWindowUpdate();
          Plot();
      }  */
}


void ProfileGrayScaleDistributionView::Plot()
{
    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());
    if (!mitkImage)
    {
        return;
    }
    Int3DImageType::Pointer itkImage;
    mitk::CastToItkImage(mitkImage, itkImage);

    Int3DImageType::IndexType startIndex, endIndex;
    mitkImage->GetGeometry()->WorldToIndex(m_startPoint, startIndex);
    mitkImage->GetGeometry()->WorldToIndex(m_endPoint, endIndex);


    //create samples along the line
    QVector<double> tick;
    QVector<double> val;
    itk::LineIterator<Int3DImageType> it1(itkImage, startIndex, endIndex);
    it1.GoToBegin();
    int i = 0;
    int max = 0;
    int min = 0;
    double maxValue = FLT_MIN;
    double minValue = FLT_MAX;
    while (!it1.IsAtEnd())
    {
        tick.push_back(i);
        val.push_back(it1.Get());
        if (it1.Get()>maxValue)
        {
            max = i;
            maxValue = it1.Get();
        }
        if (it1.Get()<minValue)
        {
            min = i;
            minValue = it1.Get();
        }

        ++i;
        ++it1;
    }
    m_curve->setSamples(tick, val);

    //mark the largest and smallest 
    //max
    {
        m_maxY->setYValue(maxValue);
        m_maxX->setXValue(max);
        m_maxPos->setValue(QPointF(max, maxValue));
        m_maxPos->setLabel(QString("max:%1,%2").arg(max).arg(maxValue));

        m_maxY->setVisible(true);
        m_maxX->setVisible(true);
        m_maxPos->setVisible(true);
    }

    //min
    {
        m_minY->setYValue(minValue);
        m_minX->setXValue(min);
        m_minPos->setValue(QPointF(min, minValue));
        m_minPos->setLabel(QString("min:%1,%2").arg(min).arg(minValue));

        m_minY->setVisible(true);
        m_minX->setVisible(true);
        m_minPos->setVisible(true);
    }
   

    m_ui.Plot->replot();
}