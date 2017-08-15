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

//vtk
#include "vtkJPEGReader.h"
#include "vtkQImageToImageSource.h"
#include "vtkUnsignedCharArray.h"
#include "vtkPointData.h"

//common
#include "ITKImageTypeDef.h"


#include "CurveProjectionMapper2D.h"
#include "TransparentBackgroundMapper2D.h"

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

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(Refresh()));
}

int imageNum = 0;

void HistogramView::Refresh()
{
    QString fileName = QString("S:/Data/UR/%1.jpg").arg(imageNum);
    if (imageNum>61)
    {
        imageNum = 0;
    }
    else
    {
        imageNum++;
    }

    m_qImage->load(fileName);
    m_ui.ImageWidget->setFixedSize(m_qImage->width()/3, m_qImage->height()/3);
    m_qImage->convertToFormat(QImage::Format_RGBA8888);

    m_ui.ImageWidget->setPixmap(QPixmap::fromImage(*m_qImage));



    vtkSmartPointer<vtkQImageToImageSource> converter = vtkSmartPointer<vtkQImageToImageSource>::New();
    converter->SetQImage(m_qImage);
    converter->Update();

    vtkImageData* image = converter->GetOutput();
    

    m_image->Initialize(image);
    m_image->GetVtkImageData()->DeepCopy(image);
    

    RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
   // m_pMitkRenderWindow->GetMitkStdMultiWidget()->ForceImmediateUpdate();
   // RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_2DWINDOWS);

}

void HistogramView::Plot()
{
   //cut image in cpu
    /************************************************************************/
    /* conver vtk rgb image to rgba image                                                                     */
    /************************************************************************/
    /*vtkJPEGReader* reader = vtkJPEGReader::New();
    reader->SetFileName("D:/temp/UR.jpg");
    reader->Update();
    vtkImageData* originImage = reader->GetOutput();
    vtkImageData* image = vtkImageData::New();
    image->ShallowCopy(reader->GetOutput());
    image->AllocateScalars(VTK_UNSIGNED_CHAR, 4);

    int dimens[3];
    image->GetDimensions(dimens);
    unsigned char* data = new unsigned char[4 * dimens[0]*dimens[1]];
    memset(data, 0, 4 * dimens[0] * dimens[1]);
    vtkUnsignedCharArray* array = vtkUnsignedCharArray::SafeDownCast(image->GetPointData()->GetScalars());
    array->SetVoidArray(data, 4 * dimens[0] * dimens[1], 0, vtkUnsignedCharArray::VTK_DATA_ARRAY_DELETE);

    double smallRadiusSquared = 258 * 258;
    double largeRadiusSquared = 882 * 882;
    QVector2D u(0, 1);
    QVector2D b(564 - 436, 1 + 225);
    double cosTheta = QVector2D::dotProduct(u.normalized(), b.normalized());
    for (int y = 0; y < dimens[1]; y++)
    {
        for (int x = 0; x < dimens[0]; x++)
        {
            unsigned char* opixel = static_cast<unsigned char*>(originImage->GetScalarPointer(x, y, 0));
            unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(x, y, 0));

            QVector2D p(x - 436, 884 - y);
            double distance = p.lengthSquared();
            if (distance< smallRadiusSquared ||
                distance> largeRadiusSquared ||
                QVector2D::dotProduct(u.normalized(), p.normalized()) < cosTheta)
            {
                pixel[3] = 0;
            }
            else
            {
                pixel[0] = opixel[0];
                pixel[1] = opixel[1];
                pixel[2] = opixel[2];
                pixel[3] = 255;
            }
        }
    }*/

    /************************************************************************/
    /*Cut image in CPU                                                                     */
    /************************************************************************/
   /* int numberOfComponent = image->GetNumberOfScalarComponents();

    int* dims = image->GetDimensions();

    double smallRadiusSquared = 258 * 258;
    double largeRadiusSquared = 882 * 882;
    QVector2D u(0, 1);
    QVector2D b(564 - 436, 1 + 225);
    double cosTheta = QVector2D::dotProduct(u.normalized(), b.normalized());

    for (int y = 0; y < dims[1]; y++)
    {
        for (int x = 0; x < dims[0]; x++)
        {
            unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(x, y, 0));

            QVector2D p(x - 436, 884 - y);
            double distance = p.lengthSquared();
            if (distance< smallRadiusSquared ||
                distance> largeRadiusSquared ||
                QVector2D::dotProduct(u.normalized(), p.normalized()) < cosTheta)
            {
                pixel[3] = 0;
            }
        }
    }*/

    m_image = mitk::Image::New();

    m_dataNode = mitk::DataNode::New();
    Refresh();
    m_dataNode->SetData(m_image);
    m_dataNode->SetName("UR");
    GetDataStorage()->Add(m_dataNode);
    m_pMitkRenderWindow->Reinit(m_dataNode);

    /************************************************************************/
    /* Cut image in GPU                                                                     */
    /************************************************************************/
    mitk::TransparentBackgroundMapper2D::Pointer mapper = mitk::TransparentBackgroundMapper2D::New();
    mapper->SetDataNode(m_dataNode);
    m_dataNode->SetMapper(mitk::BaseRenderer::Standard2D, mapper);
    m_dataNode->SetStringProperty("2d mapper type", "shader rendering");

    float color[3] = {0.0,0.0,1.0};
    mitk::Color bkcolor(color);
    //m_pMitkRenderWindow->GetMitkStdMultiWidget()->SetGradientBackgroundColorForRenderWindow(bkcolor, bkcolor,3);
    m_pMitkRenderWindow->GetMitkStdMultiWidget()->SetGradientBackgroundColors(bkcolor, bkcolor);

    m_timer.start(20);

    return;

    //mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());
    //if (!mitkImage)
    //{
    //    return;
    //}
    //Int3DImageType::Pointer itkImage;
    //mitk::CastToItkImage(mitkImage, itkImage);

    //Int3DImageType::RegionType::SizeType imageSize = itkImage->GetLargestPossibleRegion().GetSize();
    //

    //const unsigned int MeasurementVectorSize = 1; // Grayscale
    //const unsigned int binsPerDimension = 255;
    //typedef itk::Statistics::ImageToHistogramFilter< Int3DImageType > ImageToHistogramFilterType;
    //ImageToHistogramFilterType::HistogramType::MeasurementVectorType lowerBound(binsPerDimension);
    //lowerBound.Fill(mitkImage->GetScalarValueMin());
    //ImageToHistogramFilterType::HistogramType::MeasurementVectorType upperBound(binsPerDimension);
    //upperBound.Fill(mitkImage->GetScalarValueMax());
    //ImageToHistogramFilterType::HistogramType::SizeType size(MeasurementVectorSize);
    //size.Fill(binsPerDimension);

    //ImageToHistogramFilterType::Pointer imageToHistogramFilter = ImageToHistogramFilterType::New();
    //imageToHistogramFilter->SetInput(itkImage);
    //imageToHistogramFilter->SetHistogramBinMinimum(lowerBound);
    //imageToHistogramFilter->SetHistogramBinMaximum(upperBound);
    //imageToHistogramFilter->SetHistogramSize(size);
    //imageToHistogramFilter->Update();
    //ImageToHistogramFilterType::HistogramType* histogram = imageToHistogramFilter->GetOutput();

    //uint numValues = histogram->GetSize()[0];
    //uint totalNum = imageSize[0] * imageSize[1] * imageSize[2];
    //QVector<QwtIntervalSample> samples(numValues);
    //for (uint i = 0; i < numValues; i++)
    //{
    //    double value = histogram->GetMeasurementVector(i).GetElement(0);
    //    QwtInterval interval(value, value + 1.0);
    //    interval.setBorderFlags(QwtInterval::ExcludeMaximum);
    //    samples[i] = QwtIntervalSample((double)histogram->GetFrequency(i)/ totalNum, interval);
    //}
    //std::cout <<"Bin Max:"<< histogram->GetDimensionMaxs(0).at(0)<< std::endl;
    //m_histogram->setData(new QwtIntervalSeriesData(samples));
    //m_ui.HistogramPlot->replot();



    
}


void HistogramView::OnSelectStartPoint()
{
    m_startPoint = m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetCrossPosition();
    QString pointStr = QString("%1,%2,%3").arg(m_startPoint.GetElement(0)).arg(m_startPoint.GetElement(1)).arg(m_startPoint.GetElement(2));
    m_ui.StartPointLE->setText(pointStr);
}
void HistogramView::OnSelectEndPoint()
{
    m_endPoint = m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetCrossPosition();
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