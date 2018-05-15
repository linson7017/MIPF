#include "DSAReader.h" 
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
#include "itkMeanSquaresImageToImageMetric.h"

#include <itkExtractImageFilter.h>

#include <itkSubtractImageFilter.h>

#include "itkResampleImageFilter.h"


#include "QmitkStdMultiWidget.h"

#include <vtkExtractVOI.h>

//opencv
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/types_c.h>

#include "ITKImageTypeDef.h"
#include "ITK_Helpers.h"  

#include "mitkImageCast.h"

#include "CVA/IQF_DSATool.h"
#include "cva/cva_command_def.h"

#include "qf_log.h"

DSAReader::DSAReader() :MitkPluginView() 
{
}
 
DSAReader::~DSAReader() 
{
    m_timer->stop();
    delete m_timer;
}
 
void DSAReader::CreateView()
{
    m_ui.setupUi(this);

    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(CreateImagePredicate());

    connect(m_ui.LoadBtn, &QPushButton::clicked, this, &DSAReader::Load);
    connect(m_ui.ConfirmOriginFrameBtn, &QPushButton::clicked, this, &DSAReader::ConfirmOriginFrame);
    connect(m_ui.ConfirmTargetFrameBtn, &QPushButton::clicked, this, &DSAReader::ConfirmTargetFrame);
    connect(m_ui.CutBtn, &QPushButton::clicked, this, &DSAReader::Cut);
    connect(m_ui.CompareBtn, &QPushButton::clicked, this, &DSAReader::Compare);
    connect(m_ui.AnimationCB, &QPushButton::clicked, this, &DSAReader::EnableAnimation);

    

    m_timer = new QTimer;
    connect(m_timer, &QTimer::timeout, this, &DSAReader::RefreshSlice);

} 

void DSAReader::EnableAnimation(bool enable)
{
     if (!enable)
     {
         if (m_timer->isActive())
         {
             m_timer->stop();
         }
     }
     else
     {
         if (!m_timer->isActive())
         {
             m_timer->start(100);
         }
     }
}

void DSAReader::RefreshSlice()
{
    QString multiViewID = GetAttribute("MultiViewID");
    QmitkRenderWindow* renderWindow = GetMitkRenderWindowInterface()->GetQmitkRenderWindow(multiViewID + "-axial");
    int pos = renderWindow->GetSliceNavigationController()->GetSlice()->GetSteps() - 1 -
        renderWindow->GetSliceNavigationController()->GetSlice()->GetPos();
    QF_INFO << renderWindow->GetSliceNavigationController()->GetSlice()->GetRangeMin() << "," << renderWindow->GetSliceNavigationController()->GetSlice()->GetRangeMax();
    int currentSlice =  renderWindow->GetSliceNavigationController()->GetSlice()->GetPos();
    QF_INFO << currentSlice;
    QF_INFO << renderWindow->GetSliceNavigationController()->GetSlice()->GetSteps();
    if (currentSlice > 0)
    {
        renderWindow->GetSliceNavigationController()->GetSlice()->SetPos(currentSlice-1);
    }
    else
    {
        renderWindow->GetSliceNavigationController()->GetSlice()->SetPos(renderWindow->GetSliceNavigationController()->GetSlice()->GetSteps() - 1);
    }
    RequestRenderWindowUpdate();
}
 
WndHandle DSAReader::GetPluginHandle() 
{
    return this; 
}

void DSAReader::Load()
{
    m_pMain->ExecuteCommand(CVA_COMMAND_LOAD_DSA,0,0);
}

void DSAReader::ConfirmOriginFrame()
{
    QString multiViewID = GetAttribute("MultiViewID");
    QmitkRenderWindow* renderWindow = GetMitkRenderWindowInterface()->GetQmitkRenderWindow(multiViewID + "-axial");
    m_ui.OriginFrameLabel->setText(QString("%1").arg(renderWindow->GetSliceNavigationController()->GetSlice()->GetSteps()-1-
        renderWindow->GetSliceNavigationController()->GetSlice()->GetPos()));
}
void DSAReader::ConfirmTargetFrame()
{
    QString multiViewID = GetAttribute("MultiViewID");
    QmitkRenderWindow* renderWindow = GetMitkRenderWindowInterface()->GetQmitkRenderWindow(multiViewID + "-axial");
    m_ui.TargetFrameLabel->setText(QString("%1").arg(renderWindow->GetSliceNavigationController()->GetSlice()->GetSteps() - 1 -
        renderWindow->GetSliceNavigationController()->GetSlice()->GetPos()));
}

class MIRegistrationParameters
{
    int m_numberOfIterations;
    int m_numberOfHistogramBins;
    double m_rateOfSpatialSamples;
    double m_minimumStepLength;
    double m_maximumStepLength;
public:
    MIRegistrationParameters() :
        m_numberOfIterations(200),
        m_numberOfHistogramBins(64),
        m_rateOfSpatialSamples(0.10),
        m_minimumStepLength(0.001),
        m_maximumStepLength(0.1)
    {}
    void SetNumberOfIterations(int numberOfIterations) { m_numberOfIterations = numberOfIterations; }
    int GetNumberOfIterations() { return m_numberOfIterations; }
    void SetNumberOfHistogramBins(int numberOfHistogramBins) { m_numberOfHistogramBins = numberOfHistogramBins; }
    int GetNumberOfHistogramBins() { return m_numberOfHistogramBins; }
    void SetRateOfSpatialSamples(double rateOfSpatialSamples) { m_rateOfSpatialSamples = rateOfSpatialSamples; }
    double GetRateOfSpatialSamples() { return m_rateOfSpatialSamples; }
    void SetMinimumStepLength(double minimumStepLength) { m_minimumStepLength = minimumStepLength; }
    double GetMinimumStepLength() { return m_minimumStepLength; }
    void SetMaximumStepLength(double maximumStepLength) { m_maximumStepLength = maximumStepLength; }
    double GetMaximumStepLength() { return m_maximumStepLength; }

};


template<class FixedImageType, class MovingImageType,class TransformType>
void Register(const FixedImageType* fixedImage, const MovingImageType* movingImage, FixedImageType* resultImage, TransformType* initTransform)
{
    MIRegistrationParameters RegistrationParameters;

    //typedef itk::Euler2DTransform<double> TransformType;
    typedef itk::RegularStepGradientDescentOptimizer OptimizerType;
    typedef itk::LinearInterpolateImageFunction<MovingImageType, double> InterpolatorType;
   // typedef  itk::MattesMutualInformationImageToImageMetric<FixedImageType, MovingImageType> MetricType;
    typedef itk::MeanSquaresImageToImageMetric<FixedImageType, MovingImageType> MetricType;
    typedef itk::ImageRegistrationMethod<FixedImageType, MovingImageType> RegistrationType;

    //**************Optimizer****************//
    OptimizerType::Pointer optimizer = OptimizerType::New();
    optimizer->SetNumberOfIterations(RegistrationParameters.GetNumberOfIterations());
    optimizer->SetMinimumStepLength(RegistrationParameters.GetMinimumStepLength());
    optimizer->SetMaximumStepLength(RegistrationParameters.GetMaximumStepLength());
    optimizer->SetMinimize(true);
    optimizer->SetRelaxationFactor(0.8);

    TransformType::Pointer transform = TransformType::New();
    typedef OptimizerType::ScalesType       OptimizerScalesType;
    OptimizerScalesType optimizerScales(transform->GetNumberOfParameters());
    FixedImageType::RegionType::SizeType fixedImageSize = fixedImage->GetLargestPossibleRegion().GetSize();
    FixedImageType::SpacingType fixedImageSpacing = fixedImage->GetSpacing();
    double imageSizeInMM[2];
    imageSizeInMM[0] = fixedImageSize[0] * fixedImageSpacing[0];
    imageSizeInMM[1] = fixedImageSize[1] * fixedImageSpacing[1];
    double translationScale = 1.0 / sqrt(imageSizeInMM[0] * imageSizeInMM[0] + imageSizeInMM[1] * imageSizeInMM[1]);
    optimizerScales[0] = 1.0;//original
    optimizerScales[1] = translationScale;
    optimizerScales[2] = translationScale;
    optimizer->SetScales(optimizerScales);

    //**************Metric****************//
    MetricType::Pointer metric = MetricType::New();
    FixedImageType::RegionType fixedImageRegion = fixedImage->GetBufferedRegion();
    const unsigned int numberOfPixels = fixedImageRegion.GetNumberOfPixels();
    unsigned long numberOfSpatialSamples = static_cast<unsigned long>(numberOfPixels * RegistrationParameters.GetRateOfSpatialSamples());
//    metric->SetNumberOfHistogramBins(RegistrationParameters.GetNumberOfHistogramBins());
//    metric->SetNumberOfSpatialSamples(numberOfSpatialSamples);

    //**************Interpolator****************//
    InterpolatorType::Pointer interpolator = InterpolatorType::New();

    //**************Registration****************//
    RegistrationType::Pointer registration = RegistrationType::New();
    registration->SetTransform(transform);
    registration->SetInitialTransformParameters(initTransform->GetParameters());
    registration->SetMetric(metric);
    registration->SetOptimizer(optimizer);
    registration->SetInterpolator(interpolator);
    registration->SetFixedImage(fixedImage);
    registration->SetMovingImage(movingImage);
    registration->SetFixedImageRegion(fixedImage->GetLargestPossibleRegion());
    try
    {
        registration->Update();
        std::cout << "Optimizer stop condition = "
            << registration->GetOptimizer()->GetStopConditionDescription()
            << std::endl;
    }
    catch (itk::ExceptionObject & err)
    {
        std::cout << "ExceptionObject caught !" << std::endl;
        std::cout << err << std::endl;
    }

    if (resultImage)
    {
        typedef itk::ResampleImageFilter<
            MovingImageType,
            FixedImageType >    ResampleFilterType;

        initTransform->SetParameters(registration->GetLastTransformParameters());
        initTransform->SetFixedParameters(transform->GetFixedParameters());


        ResampleFilterType::Pointer resample = ResampleFilterType::New();

        resample->SetTransform(initTransform);
        resample->SetInput(movingImage);

        resample->SetSize(fixedImage->GetLargestPossibleRegion().GetSize());
        resample->SetOutputOrigin(fixedImage->GetOrigin());
        resample->SetOutputSpacing(fixedImage->GetSpacing());
        resample->SetOutputDirection(fixedImage->GetDirection());
        resample->SetDefaultPixelValue(100);
        resample->Update();

        resultImage->Graft(resample->GetOutput());
    }
}

void DSAReader::Compare()
{
    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());
    if (!mitkImage)
    {
        return;
    }
    int extent[6];
    int sliceIndex = m_ui.TargetFrameLabel->text().toInt();
    mitkImage->GetVtkImageData()->GetExtent(extent);
    vtkSmartPointer<vtkExtractVOI> extractor = vtkSmartPointer<vtkExtractVOI>::New();
    extractor->SetInputData(mitkImage->GetVtkImageData());
    extractor->SetVOI(extent[0], extent[1], extent[2], extent[3], sliceIndex, sliceIndex);
    extractor->Update();
    vtkImageData* sliceImage = extractor->GetOutput();
    //copy image value to cv Mat
    cv::Mat mat(abs(extent[1] - extent[0]), abs(extent[3] - extent[2]), CV_32FC1);
    mat.setTo(cv::Scalar(0));
    for (int j = extent[0]; j < extent[1]; j++)
    {
        for (int k = extent[2]; k < extent[3]; k++)
        {
            float value = sliceImage->GetScalarComponentAsFloat(j, k, sliceIndex, 0);
            mat.at<float>(k, j) = value;
        }
    }
}

template <class PixelType>
mitk::Image::Pointer DSAReader::CutImage(mitk::Image* mitkImage)
{
    typedef  itk::Image<PixelType, 2> ImageType2D;
    typedef  itk::Image<PixelType, 3> ImageType3D;
    Int3DImageType::Pointer itkImage;
    mitk::CastToItkImage(mitkImage, itkImage);
    Int3DImageType::SizeType inSize = itkImage->GetLargestPossibleRegion().GetSize();
    Int2DImageType::Pointer originSlice = Int2DImageType::New();
    ITKHelpers::Extract2DSlice<Int3DImageType, Int2DImageType>(itkImage, originSlice, 0);
    Int3DImageType::Pointer resutlImage = Int3DImageType::New();
    resutlImage->Graft(itkImage);
    for (int i = 0; i < inSize[2]; i++)
    {
        Int2DImageType::Pointer targetSlice = Int2DImageType::New();
        ITKHelpers::Extract2DSlice<Int3DImageType, Int2DImageType>(itkImage, targetSlice, i);
        typedef itk::SubtractImageFilter <Int2DImageType, Int2DImageType >
            SubtractImageFilterType;
        SubtractImageFilterType::Pointer subtractFilter
            = SubtractImageFilterType::New();
        subtractFilter->SetInput1(targetSlice);
        subtractFilter->SetInput2(originSlice);
        subtractFilter->Update();
        ITKHelpers::Assign2DSlice<Int3DImageType, Int2DImageType>(resutlImage, subtractFilter->GetOutput(), i);
    }
    typedef itk::RescaleIntensityImageFilter< Int3DImageType, ImageType3D > RescaleFilterType;
    RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
    rescaleFilter->SetInput(resutlImage);
    rescaleFilter->SetOutputMinimum(mitkImage->GetScalarValueMin());
    rescaleFilter->SetOutputMaximum(mitkImage->GetScalarValueMax());
    rescaleFilter->Update();

    mitk::Image::Pointer mitkResultImage = mitk::Image::New();
    mitk::CastToMitkImage(rescaleFilter->GetOutput(), mitkResultImage);
    return mitkResultImage;
}

void DSAReader::Cut()
{
    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());
    if (!mitkImage)
    {
        return;
    }
    mitk::Image::Pointer mitkResultImage = nullptr;
    switch (mitkImage->GetVtkImageData()->GetScalarType())
    {
    case VTK_INT:
        mitkResultImage = CutImage<int>(mitkImage);
        break;
    case VTK_CHAR:
        mitkResultImage = CutImage<char>(mitkImage);
        break;
    case VTK_SHORT:
        mitkResultImage = CutImage<short>(mitkImage);
        break;
    case VTK_UNSIGNED_INT:
        mitkResultImage = CutImage<unsigned int>(mitkImage);
        break;
    case VTK_UNSIGNED_CHAR:
        mitkResultImage = CutImage<unsigned char>(mitkImage);
        break;
    case VTK_UNSIGNED_SHORT:
        mitkResultImage = CutImage<unsigned short>(mitkImage);
        break;
    case VTK_FLOAT:
        mitkResultImage = CutImage<float>(mitkImage);
        break;
    default:
        break;
    }

    mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
    resultNode->SetData(mitkResultImage);
    GetDataStorage()->Add(resultNode);
    IQF_DSATool* pDSATool = (IQF_DSATool*)m_pMain->GetInterfacePtr(QF_INTERFACE_DSA_TOOL);
    std::string referenceImageFileName;
    if (m_ui.DataSelector->GetSelectedNode()->GetStringProperty("filename", referenceImageFileName))
    {
        pDSATool->SaveDSADicomFile(mitkResultImage.GetPointer(),referenceImageFileName.c_str());
        return;
    }

    Int3DImageType::Pointer itkImage;
    mitk::CastToItkImage(mitkImage, itkImage);
    Int3DImageType::SizeType inSize = itkImage->GetLargestPossibleRegion().GetSize();

    Int2DImageType::Pointer originSlice = Int2DImageType::New();
    ITKHelpers::Extract2DSlice<Int3DImageType,Int2DImageType>(itkImage, originSlice, 0);

    Int3DImageType::Pointer resutlImage = Int3DImageType::New();
    resutlImage->Graft(itkImage);


    typedef itk::Euler2DTransform<double> TransformType;
    TransformType::Pointer initTransform = TransformType::New();
    for (int i=0;i<inSize[2];i++)
    {
        Int2DImageType::Pointer targetSlice = Int2DImageType::New();
        ITKHelpers::Extract2DSlice<Int3DImageType, Int2DImageType>(itkImage, targetSlice, i);

        typedef itk::SubtractImageFilter <Int2DImageType, Int2DImageType >
            SubtractImageFilterType;
        Int2DImageType::Pointer result = Int2DImageType::New();
        if (m_ui.PreRegisterCB->isChecked())
        {
            initTransform->SetIdentity();
            QF_INFO << "Init Matrix:";
            QF_INFO << initTransform->GetMatrix();
            Register<Int2DImageType,Int2DImageType,itk::Euler2DTransform<double>>(targetSlice.GetPointer(), originSlice.GetPointer(), result.GetPointer(), initTransform);
        }
        else
        {
            result->Graft(originSlice);
        }
        SubtractImageFilterType::Pointer subtractFilter
            = SubtractImageFilterType::New();
        subtractFilter->SetInput1(targetSlice);
        subtractFilter->SetInput2(result);
        subtractFilter->Update();

       ITKHelpers::Assign2DSlice<Int3DImageType, Int2DImageType>(resutlImage, subtractFilter->GetOutput(), i);
    }
 
    mitk::DataNode::Pointer node = ImportITKImage<Int3DImageType>(resutlImage.GetPointer(), "result");
    mitk::LevelWindow lw;
    lw.SetLevelWindow(-400, 1200);
    node->SetLevelWindow(lw);


    QString multiViewID = GetAttribute("MultiViewID");
    QmitkRenderWindow* renderWindow = GetMitkRenderWindowInterface()->GetQmitkRenderWindow(multiViewID + "-axial");
    renderWindow->GetSliceNavigationController()->GetSlice()->SetPos(renderWindow->GetSliceNavigationController()->GetSlice()->GetSteps() - 1);
    RequestRenderWindowUpdate();

    if (m_ui.AnimationCB->isChecked())
    {
        m_timer->start(100);
    }  
}