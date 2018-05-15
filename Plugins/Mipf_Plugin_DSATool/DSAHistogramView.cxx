#include "DSAHistogramView.h" 
#include "iqf_main.h"  
#include <QFileDialog>

#include "mitkImageCast.h"

#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkNumericSeriesFileNames.h"

#include "itkEuler2DTransform.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkImageRegistrationMethod.h"
//itk
#include "itkImageToHistogramFilter.h"
#include "itkImageRandomIteratorWithIndex.h"
#include "itkLineIterator.h"
#include "itkSampleToHistogramFilter.h"
#include "itkListSample.h"
#include "itkHistogram.h"


#include <itkExtractImageFilter.h>

#include <itkSubtractImageFilter.h>

#include "itkResampleImageFilter.h"


#include "QmitkStdMultiWidget.h"


//qwt
#include "qwt_plot_histogram.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_marker.h"
#include "qwt_symbol.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_magnifier.h"

#include "ITKImageTypeDef.h"
#include "ITK_Helpers.h"

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

  
DSAHistogramView::DSAHistogramView() :MitkPluginView() 
{
}
 
DSAHistogramView::~DSAHistogramView() 
{
}
 
void DSAHistogramView::CreateView()
{
    m_ui.setupUi(this);

    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(CreateImagePredicate());

    connect(m_ui.ApplyBtn, &QPushButton::clicked, this, &DSAHistogramView::Apply);

    {
        QwtPlotCanvas *canvas = new QwtPlotCanvas();
        canvas->setPalette(Qt::gray);
        canvas->setBorderRadius(10);
        m_ui.Plot->setCanvas(canvas);
        m_ui.Plot->plotLayout()->setAlignCanvasToScales(true);
        m_ui.Plot->setAxisTitle(QwtPlot::yLeft, "Number of bins");
        m_ui.Plot->setAxisTitle(QwtPlot::xBottom, "Gray Value");

        //panning
        (void)new QwtPlotPanner(canvas);
        //zooming
        QwtPlotMagnifier *magnifier = new QwtPlotMagnifier(canvas);
        magnifier->setMouseButton(Qt::NoButton);
    }
   
    m_curve = new QwtPlotCurve("profile");
    m_curve->setPen(Qt::green);
    m_curve->attach(m_ui.Plot);

   /* m_histogram = new Histogram("Histogram", Qt::red);
    m_histogram->attach(m_ui.Plot);*/

} 
 
WndHandle DSAHistogramView::GetPluginHandle() 
{
    return this; 
}

void DSAHistogramView::Apply()
{
        mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());
        if (!mitkImage)
        {
            return;
        }
        Int3DImageType::Pointer itkImage;
        mitk::CastToItkImage(mitkImage, itkImage);
    
        Int3DImageType::RegionType::SizeType imageSize = itkImage->GetLargestPossibleRegion().GetSize();
        QVector<QwtIntervalSample> samples(imageSize[2]);
        QVector<double> tick;
        QVector<double> val;
        double maxValue = 0;
        int maxIndex;
        for (int i=0;i<imageSize[2];i++)
        {
            tick.push_back(i);
            Int3DImageType::IndexType start;
            start[0] = 0;
            start[1] = 0;
            start[2] = 0;
            Int3DImageType::SizeType size;
            size[0] = imageSize[0];
            size[1] = imageSize[1];
            size[2] = 1;
            Int3DImageType::RegionType region1;
            region1.SetSize(size);
            region1.SetIndex(start);

            start[0] = 0;
            start[1] = 0;
            start[2] = i;
            Int3DImageType::RegionType region2;
            region2.SetSize(size);
            region2.SetIndex(start);
            itk::ImageRegionIterator<Int3DImageType> iter1(itkImage, region1);
            itk::ImageRegionIterator<Int3DImageType> iter2(itkImage, region2);
            double value = 0;
            while (!iter1.IsAtEnd())
            {
                value += abs(iter2.Get() - iter1.Get());
                ++iter1;
                ++iter2;
            }
            if (value>maxValue)
            {
                maxValue = value;
                maxIndex = i;
            }
            val.push_back(value);
        }

        m_curve->setSamples(tick, val);
        m_ui.Plot->replot();


        Int2DImageType::Pointer targetSlice = Int2DImageType::New();
        ITKHelpers::Extract2DSlice<Int3DImageType,Int2DImageType>(itkImage, targetSlice, maxIndex);
        ImportITKImage<Int2DImageType>(targetSlice, "target");
}

//void DSAHistogramView::Apply()
//{
//    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());
//    if (!mitkImage)
//    {
//        return;
//    }
//    Int3DImageType::Pointer itkImage;
//    mitk::CastToItkImage(mitkImage, itkImage);
//
//    Int3DImageType::RegionType::SizeType imageSize = itkImage->GetLargestPossibleRegion().GetSize();
//
//    QString multiViewID = GetAttribute("MultiViewID");
//    QmitkRenderWindow* renderWindow = GetMitkRenderWindowInterface()->GetQmitkRenderWindow(multiViewID + "-axial");
//    int sliceIndex = renderWindow->GetSliceNavigationController()->GetSlice()->GetSteps() - 1 -
//        renderWindow->GetSliceNavigationController()->GetSlice()->GetPos();
//
//    //origin slice
//    Int3DImageType::IndexType start;
//    start[0] = 0;
//    start[1] = 0;
//    start[2] = sliceIndex;
//    Int3DImageType::SizeType size;
//    size[0] = imageSize[0];
//    size[1] = imageSize[1];
//    size[2] = 1;
//    Int3DImageType::RegionType region;
//    region.SetSize(size);
//    region.SetIndex(start);
//
//    Int2DImageType::IndexType start2D;
//    start2D[0] = 0;
//    start2D[1] = 0;
//    Int2DImageType::SizeType size2D;
//    size2D[0] = imageSize[0];
//    size2D[1] = imageSize[1];
//    Int2DImageType::RegionType region2D;
//    region2D.SetSize(size2D);
//    region2D.SetIndex(start2D);
//    Int2DImageType::Pointer slice = Int2DImageType::New();
//    slice->SetRegions(region2D);
//    slice->Allocate();
//    itk::ImageRegionIterator<Int3DImageType> targetIterator(itkImage, region);
//    itk::ImageRegionIterator<Int2DImageType> iter(slice, region2D);
//    iter.GoToBegin();
//    long br=0;
//    while (!iter.IsAtEnd())
//    {
//        iter.Set(targetIterator.Get());
//        br += targetIterator.Get();
//        ++iter;
//        ++targetIterator;
//    }
//    QF_INFO << "BR of "<< sliceIndex<<" :" << br;
//
//
//    const unsigned int MeasurementVectorSize = 1; // Grayscale
//    const unsigned int binsPerDimension = 255;
//    typedef itk::Statistics::ImageToHistogramFilter< Int2DImageType > ImageToHistogramFilterType;
//    ImageToHistogramFilterType::HistogramType::MeasurementVectorType lowerBound(binsPerDimension);
//    lowerBound.Fill(mitkImage->GetScalarValueMin());
//    ImageToHistogramFilterType::HistogramType::MeasurementVectorType upperBound(binsPerDimension);
//    upperBound.Fill(mitkImage->GetScalarValueMax());
//    ImageToHistogramFilterType::HistogramType::SizeType vsize(MeasurementVectorSize);
//    vsize.Fill(binsPerDimension);
//
//    ImageToHistogramFilterType::Pointer imageToHistogramFilter = ImageToHistogramFilterType::New();
//    imageToHistogramFilter->SetInput(slice);
//    imageToHistogramFilter->SetHistogramBinMinimum(lowerBound);
//    imageToHistogramFilter->SetHistogramBinMaximum(upperBound);
//    imageToHistogramFilter->SetHistogramSize(vsize);
//    imageToHistogramFilter->Update();
//    ImageToHistogramFilterType::HistogramType* histogram = imageToHistogramFilter->GetOutput();
//
//    uint numValues = histogram->GetSize()[0];
//    uint totalNum = imageSize[0] * imageSize[1];
//    QVector<QwtIntervalSample> samples(numValues);
//    double brightness = 0;
//    for (uint i = 0; i < numValues; i++)
//    {
//        double value = histogram->GetMeasurementVector(i).GetElement(0);
//        QwtInterval interval(value, value + 1.0);
//        interval.setBorderFlags(QwtInterval::ExcludeMaximum);
//        samples[i] = QwtIntervalSample((double)histogram->GetFrequency(i) / totalNum, interval);
//        brightness += ((double)histogram->GetFrequency(i) / (double)totalNum)*i;
//    }
//    QF_INFO << "brightness:" << brightness;
//    m_histogram->setData(new QwtIntervalSeriesData(samples));
//
//
//    m_ui.Plot->replot();
//}
