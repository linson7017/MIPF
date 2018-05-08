#include "ExtractImageSliceView.h"
#include "iqf_main.h"
#include "Res/R.h"


#include "QmitkRenderWindow.h"
#include "mitkImageCast.h"

#include "ITK_Helpers.h"

#include <vtkMath.h>

#include "mitkPointSet.h"

//itk
#include "itkSymmetricForcesDemonsRegistrationFilter.h"
#include "itkHistogramMatchingImageFilter.h"
#include "itkWarpImageFilter.h"
#include "itkBSplineDeformableTransform.h"

#include "itkImageRegistrationMethod.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkLBFGSBOptimizer.h"

#include "itkOrientImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkTimeProbesCollectorBase.h"

#include "ITKVTK_Helpers.h"
#include <vtkContourFilter.h>
#include <vtkPolyDataConnectivityFilter.h>


ExtractImageSliceView::ExtractImageSliceView() :MitkPluginView()
{
    //m_pMain->Attach(this);
}


ExtractImageSliceView::~ExtractImageSliceView()
{
}

void ExtractImageSliceView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.ImageSelector->SetPredicate(CreateImagePredicate());
    m_ui.ImageSelector->SetDataStorage(GetDataStorage());

    m_ui.FixedImageSelector->SetPredicate(CreateImagePredicate());
    m_ui.FixedImageSelector->SetDataStorage(GetDataStorage());
    m_ui.MovingImageSelector->SetPredicate(CreateImagePredicate());
    m_ui.MovingImageSelector->SetDataStorage(GetDataStorage());    
    m_ui.TemplatePointsSelector->SetPredicate(mitk::NodePredicateAnd::New(mitk::TNodePredicateDataType<mitk::PointSet>::New(),
        mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))));
    m_ui.TemplatePointsSelector->SetDataStorage(GetDataStorage());

    connect(m_ui.ExtractBtn, SIGNAL(clicked()), this, SLOT(Extract()));
    connect(m_ui.RegisterBtn, SIGNAL(clicked()), this, SLOT(Register()));
}

void ExtractImageSliceView::Extract()
{
    QString multiViewID = GetAttribute("MultiViewID");
    QmitkRenderWindow* renderWindow = GetMitkRenderWindowInterface()->GetQmitkRenderWindow(multiViewID + "-axial");
    
    int sliceNum = renderWindow->GetSliceNavigationController()->GetSlice()->GetSteps() - 1 -
        renderWindow->GetSliceNavigationController()->GetSlice()->GetPos();


    mitk::Image* image = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());
    if (!image)
    {
        return;
    }
    Int3DImageType::Pointer itkImage;
    mitk::CastToItkImage(image, itkImage);

    Int2DImageType::Pointer sliceImage = Int2DImageType::New();
    ITKHelpers::Extract2DSlice(itkImage.GetPointer(), sliceImage.GetPointer(),sliceNum);

    ImportITKImage(sliceImage.GetPointer(), "slice");
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

#include "itkEuler2DTransform.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkImageRegistrationMethod.h"
#include "itkMeanSquaresImageToImageMetric.h"
template<class FixedImageType, class MovingImageType, class TransformType>
void RegisterMI(const FixedImageType* fixedImage, const MovingImageType* movingImage, FixedImageType* resultImage, TransformType* initTransform)
{
    MIRegistrationParameters RegistrationParameters;

    //typedef itk::Euler2DTransform<double> TransformType;
    typedef itk::RegularStepGradientDescentOptimizer OptimizerType;
    typedef itk::LinearInterpolateImageFunction<MovingImageType, double> InterpolatorType;
     typedef  itk::MattesMutualInformationImageToImageMetric<FixedImageType, MovingImageType> MetricType;
    //typedef itk::MeanSquaresImageToImageMetric<FixedImageType, MovingImageType> MetricType;
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
        resample->SetDefaultPixelValue(0);
        resample->Update();

        resultImage->Graft(resample->GetOutput());
    }
}

template<class FixedImageType, class MovingImageType,class FieldImageType>
void StartRegister(const FixedImageType* fixedImage, const MovingImageType* movingImage, FixedImageType* resultImage, 
    FieldImageType* fieldImage,itk::Matrix<double, 4, 4>& initTransformMatrix)
{
    //Resample Image
    typedef itk::MinimumMaximumImageCalculator <MovingImageType>
        ImageCalculatorFilterType;
    ImageCalculatorFilterType::Pointer imageCalculatorFilter
        = ImageCalculatorFilterType::New();
    imageCalculatorFilter->SetImage(movingImage);
    imageCalculatorFilter->Compute();

    typedef itk::AffineTransform< double, 2 > ResampleTransformType;
    ResampleTransformType::Pointer resampleTransform = ResampleTransformType::New();
    //ResampleTransformType::MatrixType matrix = resampleTransform->GetMatrix();

    typedef itk::ResampleImageFilter<
        MovingImageType,
        FixedImageType >    ResampleFilterType;
    ResampleFilterType::Pointer resample = ResampleFilterType::New();
    resample->SetTransform(resampleTransform);
    resample->SetInput(movingImage);
    resample->SetSize(fixedImage->GetLargestPossibleRegion().GetSize());
    resample->SetOutputOrigin(fixedImage->GetOrigin());
    resample->SetOutputSpacing(fixedImage->GetSpacing());
    resample->SetOutputDirection(fixedImage->GetDirection());
    resample->SetDefaultPixelValue(imageCalculatorFilter->GetMinimum());
    resample->Update();


    //**************Matcher****************//
    const unsigned int Dimension = 2;
    typedef itk::HistogramMatchingImageFilter<
        MovingImageType,
        MovingImageType >   MatchingFilterType;
    MatchingFilterType::Pointer matcher = MatchingFilterType::New();
    matcher->SetInput(resample->GetOutput());
    matcher->SetReferenceImage(fixedImage);
    matcher->SetNumberOfHistogramLevels(64);
    matcher->SetNumberOfMatchPoints(7);
    matcher->ThresholdAtMeanIntensityOn();

    //**************Registration****************//
    typedef itk::Vector< FixedImageType::PixelType, Dimension >                VectorPixelType;
    typedef itk::Image<  VectorPixelType, Dimension >      DisplacementFieldType;
    typedef itk::SymmetricForcesDemonsRegistrationFilter<
        FixedImageType,
        MovingImageType,
        DisplacementFieldType> RegistrationFilterType;
    RegistrationFilterType::Pointer filter = RegistrationFilterType::New();

   // CommandIterationUpdateSFD::Pointer observer = CommandIterationUpdateSFD::New();
    //filter->AddObserver(itk::IterationEvent(), observer);

    filter->SetFixedImage(fixedImage);
    filter->SetMovingImage(matcher->GetOutput());

    filter->SetNumberOfIterations(200);
    filter->SetStandardDeviations(1.0);

    filter->Update();

    typedef itk::WarpImageFilter<
        MovingImageType,
        FixedImageType,
        DisplacementFieldType  >     WarperType;
    typedef itk::LinearInterpolateImageFunction<
        MovingImageType,
        double          >  InterpolatorType;
    WarperType::Pointer warper = WarperType::New();
    InterpolatorType::Pointer interpolator = InterpolatorType::New();

    warper->SetInput(resample->GetOutput());
    warper->SetInterpolator(interpolator);
    warper->SetOutputSpacing(fixedImage->GetSpacing());
    warper->SetOutputOrigin(fixedImage->GetOrigin());
    warper->SetOutputDirection(fixedImage->GetDirection());

    warper->SetDisplacementField(filter->GetOutput());

    fieldImage->Graft(filter->GetOutput());

   // ITKHelpers::SaveImage(filter->GetOutput(), "D:/field.mha");

    warper->Update();
    resultImage->Graft(warper->GetOutput());
}

template <class InputImageType, class OutputImageType>
int DoIt(const InputImageType* fixedImage, const InputImageType* movingImage, OutputImageType* resultImage)
{

    bool ConstrainDeformation = true;
    double MaximumDeformation = 10.0;
    double gridSize = 2;
    int Iterations = 100;
    // typedefs
    const    unsigned int ImageDimension = 2;


    typedef itk::OrientImageFilter<InputImageType, InputImageType> OrientFilterType;
    typedef itk::ResampleImageFilter<
        InputImageType,
        OutputImageType>    ResampleFilterType;

    const unsigned int SpaceDimension = ImageDimension;
    const unsigned int SplineOrder = 3;
    typedef double                                                  CoordinateRepType;
    typedef itk::ContinuousIndex<CoordinateRepType, ImageDimension> ContinuousIndexType;

    typedef itk::BSplineDeformableTransform<
        CoordinateRepType,
        SpaceDimension,
        SplineOrder>     TransformType;
    typedef itk::AffineTransform<CoordinateRepType,2> AffineTransformType;
    typedef itk::LBFGSBOptimizer                    OptimizerType;
    typedef itk::MattesMutualInformationImageToImageMetric<
        InputImageType,
        InputImageType>    MetricType;
    typedef itk::LinearInterpolateImageFunction<
        InputImageType,
        double>    InterpolatorType;
    typedef itk::ImageRegistrationMethod<
        InputImageType,
        OutputImageType>    RegistrationType;

    typename MetricType::Pointer         metric = MetricType::New();
    typename OptimizerType::Pointer      optimizer = OptimizerType::New();
    typename InterpolatorType::Pointer   interpolator = InterpolatorType::New();
    typename TransformType::Pointer      transform = TransformType::New();
    typename RegistrationType::Pointer   registration = RegistrationType::New();

    typedef TransformType::RegionType     RegionType;
    typedef TransformType::SpacingType    SpacingType;
    typedef TransformType::OriginType     OriginType;
    typedef TransformType::ParametersType ParametersType;

    // Reorient to axials to avoid issues with registration metrics not
    // transforming image gradients with the image orientation in
    // calculating the derivative of metric wrt transformation
    // parameters.
    //
    // Forcing image to be axials avoids this problem. Note, that
    // reorientation only affects the internal mapping from index to
    // physical coordinates.  The reoriented data spans the same
    // physical space as the original data.  Thus, the registration
    // transform calculated on the reoriented data is also the
    // transform forthe original un-reoriented data.
    //
    typename OrientFilterType::Pointer fixedOrient = OrientFilterType::New();
    typename OrientFilterType::Pointer movingOrient = OrientFilterType::New();

    fixedOrient->UseImageDirectionOn();
    fixedOrient->SetDesiredCoordinateOrientationToAxial();
    fixedOrient->SetInput(fixedImage);

    movingOrient->UseImageDirectionOn();
    movingOrient->SetDesiredCoordinateOrientationToAxial();
    movingOrient->SetInput(movingImage);

    // Add a time probe
    itk::TimeProbesCollectorBase collector;

    collector.Start("Read fixed volume");
    fixedOrient->Update();
    collector.Stop("Read fixed volume");

    collector.Start("Read moving volume");
    movingOrient->Update();
    collector.Stop("Read moving volume");

    // Setup BSpline deformation
    //
    //  Note that the B-spline computation requires a finite support
    //  region ( 1 grid node at the lower borders and 2 grid nodes at
    //  upper borders).
    RegionType bsplineRegion;
    typename RegionType::SizeType   gridSizeOnImage;
    typename RegionType::SizeType   gridBorderSize;
    typename RegionType::SizeType   totalGridSize;

    gridSizeOnImage.Fill(gridSize);
    gridBorderSize.Fill(3);    // Border for spline order = 3 ( 1 lower, 2 upper )
    totalGridSize = gridSizeOnImage + gridBorderSize;

    bsplineRegion.SetSize(totalGridSize);

    SpacingType spacing = fixedOrient->GetOutput()->GetSpacing();
    OriginType  origin = fixedOrient->GetOutput()->GetOrigin();;

    typename InputImageType::RegionType fixedRegion =
        fixedOrient->GetOutput()->GetLargestPossibleRegion();
    typename InputImageType::SizeType fixedImageSize =
        fixedRegion.GetSize();
    for (unsigned int r = 0; r < ImageDimension; r++)
    {
        spacing[r] *= floor(static_cast<double>(fixedImageSize[r] - 1)
            / static_cast<double>(gridSizeOnImage[r] - 1));
        origin[r] -= spacing[r];
    }

    transform->SetGridSpacing(spacing);
    transform->SetGridOrigin(origin);
    transform->SetGridRegion(bsplineRegion);

    const unsigned int numberOfParameters =
        transform->GetNumberOfParameters();

    ParametersType parameters(numberOfParameters);
    parameters.Fill(0.0);

    transform->SetParameters(parameters);

    // Initialize the transform with a bulk transform using either a
    // transform that aligns the centers of the volumes or a specified
    // bulk transform
    //
    //
    typename TransformType::InputPointType centerFixed;
    typename InputImageType::RegionType::SizeType sizeFixed =
        fixedOrient->GetOutput()->GetLargestPossibleRegion().GetSize();
    // Find the center
    ContinuousIndexType indexFixed;
    for (unsigned j = 0; j < 2; j++)
    {
        indexFixed[j] = (sizeFixed[j] - 1) / 2.0;
    }
    fixedOrient->GetOutput()->TransformContinuousIndexToPhysicalPoint(indexFixed, centerFixed);

    typename TransformType::InputPointType centerMoving;
    typename InputImageType::RegionType::SizeType sizeMoving =
        movingOrient->GetOutput()->GetLargestPossibleRegion().GetSize();
    // Find the center
    ContinuousIndexType indexMoving;
    for (unsigned j = 0; j < 2; j++)
    {
        indexMoving[j] = (sizeMoving[j] - 1) / 2.0;
    }
    movingOrient->GetOutput()->TransformContinuousIndexToPhysicalPoint(indexMoving, centerMoving);

    typename AffineTransformType::Pointer centeringTransform;
    centeringTransform = AffineTransformType::New();

    centeringTransform->SetIdentity();
    centeringTransform->SetCenter(centerFixed);
    centeringTransform->Translate(centerMoving - centerFixed);
    std::cout << "Centering transform: "; centeringTransform->Print(std::cout);

    transform->SetBulkTransform(centeringTransform);

    // Setup optimizer
    //
    //
    typename OptimizerType::BoundSelectionType boundSelect(transform->GetNumberOfParameters());
    typename OptimizerType::BoundValueType     upperBound(transform->GetNumberOfParameters());
    typename OptimizerType::BoundValueType     lowerBound(transform->GetNumberOfParameters());
    if (ConstrainDeformation)
    {
        boundSelect.Fill(2);
        upperBound.Fill(MaximumDeformation);
        lowerBound.Fill(-MaximumDeformation);
    }
    else
    {
        boundSelect.Fill(0);
        upperBound.Fill(0.0);
        lowerBound.Fill(0.0);
    }

    optimizer->SetBoundSelection(boundSelect);
    optimizer->SetUpperBound(upperBound);
    optimizer->SetLowerBound(lowerBound);

    optimizer->SetCostFunctionConvergenceFactor(1e+1);
    optimizer->SetProjectedGradientTolerance(1e-7);
    optimizer->SetMaximumNumberOfIterations(Iterations);
    optimizer->SetMaximumNumberOfEvaluations(500);
    optimizer->SetMaximumNumberOfCorrections(12);

    // Setup metric
    //
    //
    metric->ReinitializeSeed(76926294);
    metric->SetNumberOfHistogramBins(128);
    metric->SetNumberOfSpatialSamples(15000);

    std::cout << std::endl << "Starting Registration" << std::endl;

    // Registration
    //
    //
    registration->SetFixedImage(fixedOrient->GetOutput());
    registration->SetMovingImage(movingOrient->GetOutput());
    registration->SetMetric(metric);
    registration->SetOptimizer(optimizer);
    registration->SetInterpolator(interpolator);
    registration->SetTransform(transform);
    registration->SetInitialTransformParameters(transform->GetParameters());

    try
    {
        collector.Start("Registration");
        registration->Update();
        collector.Stop("Registration");
    }
    catch (itk::ExceptionObject & err)
    {
        std::cerr << "ExceptionObject caught !" << std::endl;
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }

    typename OptimizerType::ParametersType finalParameters =
        registration->GetLastTransformParameters();
    std::cout << "Final parameters: " << finalParameters[50] << std::endl;
    transform->SetParameters(finalParameters);

    // Resample to the original coordinate frame (not the reoriented
    // axial coordinate frame) of the fixed image
    //
    typename ResampleFilterType::Pointer resample = ResampleFilterType::New();

    resample->SetTransform(transform);
    resample->SetInput(movingImage);
    resample->SetDefaultPixelValue(-3000);
    resample->SetOutputParametersFromImage(fixedImage);

    collector.Start("Resample");
    resample->Update();
    collector.Stop("Resample");

    resultImage->Graft(resample->GetOutput());

    //// Write out an equivalent warp field
    ////
    ////
    //if (OutputWarp != "")
    //{
    //    typedef itk::Vector<CoordinateRepType, ImageDimension> VectorType;
    //    typedef itk::Image<VectorType, ImageDimension>         VectorImageType;

    //    typename VectorImageType::Pointer warp = VectorImageType::New();
    //    warp->CopyInformation(fixedImageReader->GetOutput());
    //    warp->SetRegions(fixedImageReader->GetOutput()->GetBufferedRegion());
    //    warp->Allocate();

    //    itk::ImageRegionIteratorWithIndex<InputImageType>  it(fixedImageReader->GetOutput(), warp->GetBufferedRegion());
    //    itk::ImageRegionIteratorWithIndex<VectorImageType> oit(warp, warp->GetBufferedRegion());

    //    typename InputImageType::IndexType index1;
    //    typename InputImageType::PointType p1, p2;
    //    VectorType v1;

    //    while (!it.IsAtEnd())
    //    {
    //        // get the position of this pixel
    //        index1 = it.GetIndex();
    //        fixedImageReader->GetOutput()->TransformIndexToPhysicalPoint(index1, p1);

    //        // transform the position
    //        p2 = transform->TransformPoint(p1);

    //        // calculate the displacement
    //        v1 = p2 - p1;

    //        // set the vector
    //        oit.Set(v1);

    //        ++it;
    //        ++oit;
    //    }

    //    typedef itk::ImageFileWriter<VectorImageType> VectorWriterType;
    //    typename VectorWriterType::Pointer vwriter = VectorWriterType::New();

    //    vwriter->SetInput(warp);
    //    vwriter->SetFileName(OutputWarp);

    //    try
    //    {
    //        collector.Start("Write warp");
    //        vwriter->Update();
    //        collector.Stop("Write warp");
    //    }
    //    catch (itk::ExceptionObject & err)
    //    {
    //        std::cerr << "ExceptionObject caught !" << std::endl;
    //        std::cerr << err << std::endl;
    //        return EXIT_FAILURE;
    //    }
    //}

    // Report the time taken by the registration
    collector.Report();

    return true;
}

#include <itkThresholdImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkMaskImageFilter.h>
#include <itkEuler2DTransform.h>
#include <vtkLineSource.h>

void ExtractImageSliceView::ExtractBrain(Float2DImageType* input, Float2DImageType* output)
{
    double min, max;
    ITKHelpers::GetImageScalarRange(input, min, max);

    typedef itk::BinaryThresholdImageFilter<Float2DImageType, UShort2DImageType> ThresholdFilterType;
    ThresholdFilterType::Pointer thresholdFilter = ThresholdFilterType::New();
    thresholdFilter->SetInput(input);
    thresholdFilter->SetLowerThreshold(0);
    thresholdFilter->SetUpperThreshold(60);
    thresholdFilter->SetOutsideValue(0);
    thresholdFilter->SetInsideValue(1);
    thresholdFilter->Update();

    UShort2DImageType::Pointer maskImage = UShort2DImageType::New();
    /*UShort2DImageType::IndexType seed;
    seed.Fill(0);
    ITKHelpers::BinaryFillLargeHolesByRegionGrowing(thresholdFilter->GetOutput(), maskImage.GetPointer(), seed);

    */
    ITKHelpers::ExtractLargestConnected(thresholdFilter->GetOutput(), maskImage.GetPointer());

    typedef itk::BinaryImageToShapeLabelMapFilter<UShort2DImageType> BinaryImageToShapeLabelMapFilterType;
    BinaryImageToShapeLabelMapFilterType::Pointer binaryImageToShapeLabelMapFilter = BinaryImageToShapeLabelMapFilterType::New();
    binaryImageToShapeLabelMapFilter->SetFullyConnected(true);
    binaryImageToShapeLabelMapFilter->SetInputForegroundValue(1);
    binaryImageToShapeLabelMapFilter->SetInput(maskImage);
    binaryImageToShapeLabelMapFilter->Update();
    BinaryImageToShapeLabelMapFilterType::OutputImageType::LabelObjectType* labelObject =
        binaryImageToShapeLabelMapFilter->GetOutput()->GetNthLabelObject(0);

    itk::Matrix< double, 2, 2 > pa = labelObject->GetPrincipalAxes();
    mitk::Point2D center = labelObject->GetCentroid();
    //draw main axe
    mitk::Vector2D pl;
    pl[0] = 0;
    pl[1] = 1.0;
    pl[0] = pa[0][0] * pl[0] + pa[0][1] * pl[1];
    pl[1] = pa[1][0] * pl[0] + pa[1][1] * pl[1];
    pl.Normalize();
    /*vtkSmartPointer<vtkLineSource> lineSource =
        vtkSmartPointer<vtkLineSource>::New();
    lineSource->SetPoint1(p0);
    lineSource->SetPoint2(p1);
    lineSource->Update();
    ImportVtkPolyData(lineSource->GetOutput(), "Principal Moments")->SetColor(1.0,1.0,0.0);*/
    double t[2] = { 0.0,1.0 };
    double theta = acos(t[0]* pl[0]+t[1]* pl[1]);


    Float2DImageType::SizeType size = input->GetLargestPossibleRegion().GetSize();
    mitk::Point2D origin = input->GetOrigin();
    mitk::Point2D spacing = input->GetSpacing();
    mitk::Point2D imageCenter;
    imageCenter[0] = origin[0] + (size[0]-1) * spacing[0] / 2.0;
    imageCenter[1] = origin[1] + (size[1]-1) * spacing[1] / 2.0;


    typedef itk::Euler2DTransform<double> TransformType;
    TransformType::Pointer transfrom = TransformType::New();
    transfrom->SetRotation(-theta);
    transfrom->SetCenter(center);
    TransformType::TranslationType translate;
    translate[0] = center[0] - imageCenter[0];
    translate[1] = center[1] - imageCenter[1];
    transfrom->SetTranslation(translate);

    typedef itk::MaskImageFilter< Float2DImageType, UShort2DImageType > MaskFilterType;
    MaskFilterType::Pointer maskFilter = MaskFilterType::New();
    maskFilter->SetInput(input);
    maskFilter->SetMaskImage(maskImage);
    maskFilter->Update();


    typedef itk::ResampleImageFilter<
        Float2DImageType,
        Float2DImageType >    ResampleFilterType;

    ResampleFilterType::Pointer resample = ResampleFilterType::New();
    resample->SetTransform(transfrom);
    resample->SetInput(maskFilter->GetOutput());
    resample->SetSize(input->GetLargestPossibleRegion().GetSize());
    resample->SetOutputOrigin(input->GetOrigin());
    resample->SetOutputSpacing(input->GetSpacing());
    resample->SetOutputDirection(input->GetDirection());
    resample->SetDefaultPixelValue(min);
    resample->Update();
    output->Graft(resample->GetOutput());

   
}


template <class ReferenceImageType, class OutputImageType>
 void CreateLandMarkImage(ReferenceImageType* refImage, OutputImageType* outputImage, mitk::PointSet* points)
 {
     ReferenceImageType::RegionType refRegion = refImage->GetLargestPossibleRegion();
     ReferenceImageType::SizeType refSize = refRegion.GetSize();
     ReferenceImageType::IndexType refStart = refRegion.GetIndex();
     ReferenceImageType::SpacingType refSpacing = refImage->GetSpacing();
     ReferenceImageType::PointType refOrigin = refImage->GetOrigin();

     outputImage->SetRegions(refRegion);
     outputImage->SetSpacing(refSpacing);
     outputImage->SetOrigin(refOrigin);
     outputImage->Allocate();
     outputImage->FillBuffer(0);

     for (int i=0;i<points->GetPointSet()->GetNumberOfPoints();i++)
     {
         mitk::Point3D p = points->GetPoint(i);
         OutputImageType::IndexType index;
         index[0] = round((p[0] - refOrigin[0]) / refSpacing[0]);
         index[1] = round((p[1] - refOrigin[1]) / refSpacing[1]);
         outputImage->SetPixel(index, i+1);
     }
 }


void ExtractImageSliceView::Register()
{
    mitk::LevelWindow lw;
    lw.SetLevelWindow(50, 100);

    mitk::Image* fixedMitkImage = dynamic_cast<mitk::Image*>(m_ui.FixedImageSelector->GetSelectedNode()->GetData());
    mitk::Image* movingMitkImage = dynamic_cast<mitk::Image*>(m_ui.MovingImageSelector->GetSelectedNode()->GetData());

    if (!fixedMitkImage|| !movingMitkImage)
    {
        return;
    }
    Float2DImageType::Pointer fixedItkImage;
    mitk::CastToItkImage(fixedMitkImage, fixedItkImage);
    Float2DImageType::Pointer movingItkImage;
    mitk::CastToItkImage(movingMitkImage, movingItkImage);

    ExtractBrain(fixedItkImage.GetPointer(), fixedItkImage.GetPointer());
    ExtractBrain(movingItkImage.GetPointer(), movingItkImage.GetPointer());

    typedef itk::BinaryThresholdImageFilter<Float2DImageType, UShort2DImageType> ThresholdFilterType;
    ThresholdFilterType::Pointer thres = ThresholdFilterType::New();
    thres->SetInput(movingItkImage);
    thres->SetLowerThreshold(0.001);
    thres->SetUpperThreshold(1000);
    thres->SetInsideValue(1);
    thres->SetOutsideValue(0);
    thres->Update();
    //ImportITKImage<UShort2DImageType>(thres->GetOutput(), "mask");
    auto vtkImage = vtkSmartPointer<vtkImageData>::New();
    ITKVTKHelpers::ConvertITKImageToVTKImage<UShort2DImageType>(thres->GetOutput(), vtkImage.Get());
    auto coutourFilter = vtkSmartPointer<vtkContourFilter>::New();
    coutourFilter->SetInputData(vtkImage);
    coutourFilter->SetValue(0, 1);
    coutourFilter->ComputeScalarsOn();
    coutourFilter->ComputeGradientsOn();
    coutourFilter->ComputeNormalsOn();
    coutourFilter->Update();
    auto confilter = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
    confilter->SetInputData(coutourFilter->GetOutput());
    confilter->SetExtractionModeToLargestRegion();
    confilter->Update();
    ImportVtkPolyData(confilter->GetOutput(), "outline")->SetColor(0.0, 1.0, 0.0);

    ImportITKImage(fixedItkImage.GetPointer(), "fixedImage")->SetLevelWindow(lw);
    ImportITKImage(movingItkImage.GetPointer(), "movingImage")->SetLevelWindow(lw);
    return;
    mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(m_ui.TemplatePointsSelector->GetSelectedNode()->GetData());
    UShort2DImageType::Pointer landMarkImage = UShort2DImageType::New();
    CreateLandMarkImage<Float2DImageType, UShort2DImageType>(fixedItkImage.GetPointer(), landMarkImage.GetPointer(), pointSet);
    ImportITKImage(landMarkImage.GetPointer(), "landMarkImage");

  

    Float2DImageType::Pointer resultItkImage = Float2DImageType::New();

    typedef itk::Euler2DTransform<double> TransformType;
    TransformType::Pointer initTransform = TransformType::New();
    RegisterMI<Float2DImageType, Float2DImageType, itk::Euler2DTransform<double>>(fixedItkImage.GetPointer(), movingItkImage.GetPointer(), resultItkImage.GetPointer(), initTransform);
   
    double angle = initTransform->GetRotation();
    TransformType::CenterType center = initTransform->GetCenter();
    TransformType::TranslationType translate = initTransform->GetTranslation();
    auto trans = vtkSmartPointer<vtkTransform>::New();
    trans->Translate(-center[0], -center[1], 0);
    trans->RotateY(angle);
    trans->Translate(center[0], center[1], 0);
    trans->Translate(-translate[0], -translate[1], 0);
    for (int i=0;i<pointSet->GetPointSet()->GetNumberOfPoints();i++)
    {
        mitk::Point3D p = pointSet->GetPoint(i);
        double* rp = trans->TransformDoublePoint(p.GetDataPointer());
        pointSet->SetPoint(i, rp);
    }
    pointSet->Modified();
    ImportITKImage(resultItkImage.GetPointer(), "Rigid Registered");
    //return;
   
    itk::Matrix<double, 4, 4> initMatrix;
    initMatrix.SetIdentity();

    typedef  itk::Image<  itk::Vector< float, 2>, 2 >  FieldImageType;
    FieldImageType::Pointer fieldImage = FieldImageType::New();
    StartRegister<Float2DImageType, Float2DImageType,FieldImageType>(fixedItkImage, resultItkImage, resultItkImage, fieldImage,initMatrix);

    for (int i = 0; i < pointSet->GetPointSet()->GetNumberOfPoints(); i++)
    {
        mitk::Point2D w;
        w[0] = pointSet->GetPoint(i)[0];
        w[1] = pointSet->GetPoint(i)[1];
        FieldImageType::IndexType index;
        fieldImage->TransformPhysicalPointToIndex(w, index);
        FieldImageType::PixelType pixel = fieldImage->GetPixel(index);
        w[0] += -pixel[0];
        w[1] += -pixel[1];

        mitk::Point3D p;
        p.Fill(0);
        p[0] = w[0];
        p[1] = w[1];
        pointSet->SetPoint(i,p);
    }
    pointSet->Modified();


    ImportITKImage(resultItkImage.GetPointer(), "resultImage")->SetLevelWindow(lw);
    ImportITKImage(fieldImage.GetPointer(), "field");



    m_ui.FixedImageSelector->GetSelectedNode()->SetLevelWindow(lw);
    m_ui.MovingImageSelector->GetSelectedNode()->SetLevelWindow(lw);

}
