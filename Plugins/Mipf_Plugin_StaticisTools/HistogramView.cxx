#include "HistogramView.h"
#include "iqf_main.h"
#include "Res/R.h"

//qwt
#include "qwt_plot_histogram.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_magnifier.h"

//itk
#include "itkImageToHistogramFilter.h"
#include "itkImageRandomIteratorWithIndex.h"
#include "itkLineIterator.h"
#include "itkSampleToHistogramFilter.h"
#include "itkListSample.h"
#include "itkHistogram.h"


//mitk
#include "mitkImageCast.h"
#include "QmitkStdMultiWidget.h"
#include "mitkImageVtkMapper2D.h"
#include "mitkResliceMethodProperty.h"
#include "mitkAbstractTransformGeometry.h"
#include "vtkMitkThickSlicesFilter.h"
#include "mitkPlaneClipping.h"
#include "vtkMitkLevelWindowFilter.h"
#include "vtkImageExtractComponents.h"

//vtk
#include "vtkJPEGReader.h"
#include "vtkQImageToImageSource.h"
#include "vtkUnsignedCharArray.h"
#include "vtkPointData.h"
#include "mitkVtkResliceInterpolationProperty.h"

//common
#include "ITKImageTypeDef.h"
#include "VesselTools/IQF_VesselSegmentationTool.h"
#include "MitkImageUtils/IQF_MitkImageReslice.h"



//#include ".h"

#include <QVector2D>

class Histogram : public QwtPlotHistogram
{
public:
    Histogram(const QString &, const QColor &);

    void setColor(const QColor &);
    void setValues(uint numValues, const double *);
};

Histogram::Histogram(const QString &title, const QColor &symbolColor) :
    QwtPlotHistogram(title)
{
    setStyle(QwtPlotHistogram::Columns);

    setColor(symbolColor);
}

void Histogram::setColor(const QColor &color)
{
    QColor c = color;
    c.setAlpha(180);
    setBrush(QBrush(c));
}

void Histogram::setValues(uint numValues, const double *values)
{
    QVector<QwtIntervalSample> samples(numValues);
    for (uint i = 0; i < numValues; i++)
    {
        QwtInterval interval(double(i), i + 1.0);
        interval.setBorderFlags(QwtInterval::ExcludeMaximum);

        samples[i] = QwtIntervalSample(values[i], interval);
    }

    setData(new QwtIntervalSeriesData(samples));
}


HistogramView::HistogramView() :MitkPluginView()
{
}

HistogramView::~HistogramView() 
{
}

void HistogramView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(CreatePredicate(Image));
    {
        QwtPlotCanvas *canvas = new QwtPlotCanvas();
        canvas->setPalette(Qt::gray);
        canvas->setBorderRadius(10);
        m_ui.HistogramPlot->setCanvas(canvas);
        m_ui.HistogramPlot->plotLayout()->setAlignCanvasToScales(true);
        m_ui.HistogramPlot->setAxisTitle(QwtPlot::yLeft, "Number of bins");
        m_ui.HistogramPlot->setAxisTitle(QwtPlot::xBottom, "Gray Value");

        //panning
        (void)new QwtPlotPanner(canvas);
        //zooming
        QwtPlotMagnifier *magnifier = new QwtPlotMagnifier(canvas);
        magnifier->setMouseButton(Qt::NoButton);
    }
    
    {
        QwtPlotCanvas *canvas = new QwtPlotCanvas();
        canvas->setPalette(Qt::gray);
        canvas->setBorderRadius(10);
        m_ui.ProfileHistogramPlot->setCanvas(canvas);
        m_ui.ProfileHistogramPlot->plotLayout()->setAlignCanvasToScales(true);
        m_ui.ProfileHistogramPlot->setAxisTitle(QwtPlot::yLeft, "Number of bins");
        m_ui.ProfileHistogramPlot->setAxisTitle(QwtPlot::xBottom, "Gray Value");

        //panning
        (void)new QwtPlotPanner(canvas);
        //zooming
        QwtPlotMagnifier *magnifier = new QwtPlotMagnifier(canvas);
        magnifier->setMouseButton(Qt::NoButton);
    }    

    m_histogram = new Histogram("Histogram", Qt::red);
    m_histogram->attach(m_ui.HistogramPlot);

    m_profileHistogram = new Histogram("Profile Histogram", Qt::red);
    m_profileHistogram->attach(m_ui.ProfileHistogramPlot);


    
    m_ui.ImageWidget->setScaledContents(true);
    m_qImage = new QImage;

    connect(m_ui.PlotBtn, SIGNAL(clicked()), this, SLOT(Plot()));
    connect(m_ui.StartPointBtn, SIGNAL(clicked()), this, SLOT(OnSelectStartPoint()));
    connect(m_ui.EndPointBtn, SIGNAL(clicked()), this, SLOT(OnSelectEndPoint()));
    connect(m_ui.ProfilePlotBtn, SIGNAL(clicked()), this, SLOT(ProfilePlot()));
}

void HistogramView::Plot()
{

    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());
    if (!mitkImage)
    {
        return;
    }
    Int3DImageType::Pointer itkImage;
    mitk::CastToItkImage(mitkImage, itkImage);

    Int3DImageType::RegionType::SizeType imageSize = itkImage->GetLargestPossibleRegion().GetSize();
    

    const unsigned int MeasurementVectorSize = 1; // Grayscale
    const unsigned int binsPerDimension = 255;
    typedef itk::Statistics::ImageToHistogramFilter< Int3DImageType > ImageToHistogramFilterType;
    ImageToHistogramFilterType::HistogramType::MeasurementVectorType lowerBound(binsPerDimension);
    lowerBound.Fill(mitkImage->GetScalarValueMin());
    ImageToHistogramFilterType::HistogramType::MeasurementVectorType upperBound(binsPerDimension);
    upperBound.Fill(mitkImage->GetScalarValueMax());
    ImageToHistogramFilterType::HistogramType::SizeType size(MeasurementVectorSize);
    size.Fill(binsPerDimension);

    ImageToHistogramFilterType::Pointer imageToHistogramFilter = ImageToHistogramFilterType::New();
    imageToHistogramFilter->SetInput(itkImage);
    imageToHistogramFilter->SetHistogramBinMinimum(lowerBound);
    imageToHistogramFilter->SetHistogramBinMaximum(upperBound);
    imageToHistogramFilter->SetHistogramSize(size);
    imageToHistogramFilter->Update();
    ImageToHistogramFilterType::HistogramType* histogram = imageToHistogramFilter->GetOutput();

    uint numValues = histogram->GetSize()[0];
    uint totalNum = imageSize[0] * imageSize[1] * imageSize[2];
    QVector<QwtIntervalSample> samples(numValues);
    for (uint i = 0; i < numValues; i++)
    {
        double value = histogram->GetMeasurementVector(i).GetElement(0);
        QwtInterval interval(value, value + 1.0);
        interval.setBorderFlags(QwtInterval::ExcludeMaximum);
        samples[i] = QwtIntervalSample((double)histogram->GetFrequency(i)/ totalNum, interval);
    }
    std::cout <<"Bin Max:"<< histogram->GetDimensionMaxs(0).at(0)<< std::endl;
    m_histogram->setData(new QwtIntervalSeriesData(samples));
    m_ui.HistogramPlot->replot();
}


void HistogramView::OnSelectStartPoint()
{
    m_startPoint = GetMitkRenderWindowInterface()->GetMitkStdMultiWidget()->GetCrossPosition();
    QString pointStr = QString("%1,%2,%3").arg(m_startPoint.GetElement(0)).arg(m_startPoint.GetElement(1)).arg(m_startPoint.GetElement(2));
    m_ui.StartPointLE->setText(pointStr);
}
void HistogramView::OnSelectEndPoint()
{
    m_endPoint = GetMitkRenderWindowInterface()->GetMitkStdMultiWidget()->GetCrossPosition();
    QString pointStr = QString("%1,%2,%3").arg(m_endPoint.GetElement(0)).arg(m_endPoint.GetElement(1)).arg(m_endPoint.GetElement(2));
    m_ui.EndPointLE->setText(pointStr);
}

void HistogramView::DrawLine()
{
    if (m_pLineNode.IsNull())
    {
        m_pLineNode = mitk::DataNode::New();
        m_pLineNode->SetColor(1.0, 1.0, 0.0);
        m_pLineNode->SetProperty("show contour", mitk::BoolProperty::New(true));
        m_pLineNode->SetProperty("show distances", mitk::BoolProperty::New(true));
        m_pLineNode->SetProperty("helper object", mitk::BoolProperty::New(true));
        GetDataStorage()->Add(m_pLineNode);
    }
    if (m_linePoints.IsNull())
    {
        m_linePoints = mitk::PointSet::New();
        m_pLineNode->SetData(m_linePoints);
    }

    m_linePoints->Clear();
    m_linePoints->InsertPoint(m_startPoint);
    m_linePoints->InsertPoint(m_endPoint);
    m_pLineNode->Modified();


    RequestRenderWindowUpdate();
    ProfilePlot();
}

void HistogramView::ProfilePlot()
{
    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());
    if (!mitkImage)
    {
        return;
    }
    Int3DImageType::Pointer itkImage;
    mitk::CastToItkImage(mitkImage, itkImage);

    Int3DImageType::IndexType startIndex, endIndex;
    mitkImage->GetGeometry()->WorldToIndex(m_startPoint, startIndex);
    mitkImage->GetGeometry()->WorldToIndex(m_endPoint, endIndex);

    //samples
    typedef itk::Vector<Int3DImageType::PixelType, 1>                       MeasurementVectorType;
    typedef itk::Statistics::ListSample< MeasurementVectorType >    SampleType;
    typedef itk::Statistics::Histogram< float,
        itk::Statistics::DenseFrequencyContainer2 >                             HistogramType;


    //create samples along the line
    SampleType::Pointer sample = SampleType::New();
    itk::LineIterator<Int3DImageType> it1(itkImage, startIndex, endIndex);
    it1.GoToBegin();
    MeasurementVectorType mv;
    while (!it1.IsAtEnd())
    {
        mv[0] = it1.Get();
        sample->PushBack(mv); 
        ++it1;
    }

    //Get histogram
    typedef itk::Statistics::SampleToHistogramFilter<SampleType, HistogramType> SampleToHistogramFilterType;
    SampleToHistogramFilterType::Pointer sampleToHistogramFilter =
        SampleToHistogramFilterType::New();
    sampleToHistogramFilter->SetInput(sample);

    SampleToHistogramFilterType::HistogramSizeType histogramSize(1);
    histogramSize.Fill(255);
    sampleToHistogramFilter->SetHistogramSize(histogramSize);
    sampleToHistogramFilter->Update();

    const HistogramType* histogram = sampleToHistogramFilter->GetOutput();


    uint numValues = histogram->GetSize()[0];
    uint totalNum = sample->Size();
    QVector<QwtIntervalSample> samples(numValues);
    for (uint i = 0; i < numValues; i++)
    {
        double value = histogram->GetMeasurementVector(i).GetElement(0);
        QwtInterval interval(value, value + 1.0);
        interval.setBorderFlags(QwtInterval::ExcludeMaximum);
        samples[i] = QwtIntervalSample((double)histogram->GetFrequency(i) / totalNum, interval);
    }
    m_profileHistogram->setData(new QwtIntervalSeriesData(samples));
    m_ui.ProfileHistogramPlot->replot();
}