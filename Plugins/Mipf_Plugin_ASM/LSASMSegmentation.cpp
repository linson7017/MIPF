#include "LSASMSegmentation.h"

 //mitk
#include "mitkNodePredicateDataType.h"
#include "mitkDataNode.h"
#include "mitkImage.h"
#include "mitkImageCast.h"
#include "mitkProperties.h"

//itk
#include "itkGeodesicActiveContourShapePriorLevelSetImageFilter.h"
#include "itkChangeInformationImageFilter.h"
#include "itkBoundedReciprocalImageFilter.h"

#include "itkPCAShapeSignedDistanceFunction.h"
#include "itkEuler2DTransform.h"
#include "itkOnePlusOneEvolutionaryOptimizer.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkNormalVariateGenerator.h"
#include "itkSigmoidImageFilter.h"
#include "vnl/vnl_sample.h"
#include "itkNumericSeriesFileNames.h"
#include "itkEuler3DTransform.h"
#include "itkTranslationTransform.h"
#include "itkVersorRigid3DTransform.h"

#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkFastMarchingImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkSpatialFunctionImageEvaluatorFilter.h"

#include "itkCurvatureFlowImageFilter.h"


#include "ITKImageTypeDef.h"
#include "MitkSegmentation/IQF_MitkSurfaceTool.h"

#include "iqf_main.h"


template<class TFilter>
class LSASMSegmentation::CommandIterationUpdate : public itk::Command
{
public:
    typedef CommandIterationUpdate   Self;
    typedef itk::Command             Superclass;
    typedef itk::SmartPointer<Self>  Pointer;
    itkNewMacro(Self);
protected:
    CommandIterationUpdate() {};
public:
    void Execute(itk::Object *caller,
        const itk::EventObject & event) ITK_OVERRIDE
    {
        Execute((const itk::Object *) caller, event);
    }
    void Execute(const itk::Object * object,
        const itk::EventObject & event) ITK_OVERRIDE
    {
        const TFilter * filter = static_cast<const TFilter *>(object);
        if (typeid(event) != typeid(itk::IterationEvent))
        {
            return;
        }
        std::cout << filter->GetElapsedIterations() << ": ";
        std::cout << filter->GetRMSChange() << " ";
        std::cout << filter->GetCurrentParameters() << std::endl;

        typedef itk::BinaryThresholdImageFilter<
            Float3DImageType,
            UChar3DImageType    >       ThresholdingFilterType;
        ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();
        thresholder->SetLowerThreshold(-1000.0);
        thresholder->SetUpperThreshold(0.0);
        thresholder->SetOutsideValue(0);
        thresholder->SetInsideValue(255);
        thresholder->SetInput(filter->GetOutput());
        thresholder->Update();

        mitk::Image::Pointer image;
        mitk::CastToMitkImage(thresholder->GetOutput(), image);

        auto resultSurface = vtkSmartPointer<vtkPolyData>::New();
        m_pSurfaceTool->ExtractSurface(image, resultSurface.Get());

        emit m_object->SignalInteractionEnd(resultSurface.Get());
    }

    void SetObject(LSASMSegmentation* object)
    {
        m_object = object;
    }
    void SetSurfaceTool(IQF_MitkSurfaceTool* pTool)
    {
        m_pSurfaceTool = pTool;
    }
private:
    LSASMSegmentation* m_object;
    IQF_MitkSurfaceTool* m_pSurfaceTool;
};

LSASMSegmentation::LSASMSegmentation(mitk::DataStorage* pDataStorage,  QF::IQF_Main* pMain):m_pDataStorage(pDataStorage) ,m_pMain(pMain)
{
}


LSASMSegmentation::~LSASMSegmentation()
{
}


void LSASMSegmentation::SlotDoSegmentation(mitk::Image* inputImage, mitk::Image* meanImage, mitk::PointSet* pSeedPointSet, const QVector<mitk::Image*>& pcaImageList, mitk::Image* outputImage)
{

    if (!inputImage || !inputImage)
    {
        MITK_ERROR << "Please Select Image!";
        return;
    }

    typedef Float3DImageType InternalImageType;
    typedef float InternalPixelType;
    Float3DImageType::Pointer itkImage;
    Float3DImageType::Pointer itkMeanImage;

    mitk::CastToItkImage(inputImage, itkImage);
    mitk::CastToItkImage(meanImage, itkMeanImage);




    typedef itk::BinaryThresholdImageFilter<
        InternalImageType,
        UChar3DImageType    >       ThresholdingFilterType;
    ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();
    thresholder->SetLowerThreshold(-1000.0);
    thresholder->SetUpperThreshold(0.0);
    thresholder->SetOutsideValue(0);
    thresholder->SetInsideValue(255);


    typedef   itk::CurvatureFlowImageFilter<
        InternalImageType,
        InternalImageType >  SmoothingFilterType;
    SmoothingFilterType::Pointer smoothing = SmoothingFilterType::New();

    typedef   itk::GradientMagnitudeRecursiveGaussianImageFilter<
        InternalImageType,
        InternalImageType >  GradientFilterType;
    GradientFilterType::Pointer  gradientMagnitude = GradientFilterType::New();

    typedef itk::SigmoidImageFilter<Float3DImageType, Float3DImageType> SigmoidType;
    SigmoidType::Pointer sigmoid = SigmoidType::New();

    typedef  itk::FastMarchingImageFilter<
        InternalImageType,
        InternalImageType >    FastMarchingFilterType;
    FastMarchingFilterType::Pointer  fastMarching = FastMarchingFilterType::New();

    typedef itk::GeodesicActiveContourShapePriorLevelSetImageFilter<
        InternalImageType, InternalImageType >
        GeodesicActiveContourFilterType;
    GeodesicActiveContourFilterType::Pointer geodesicActiveContour =
        GeodesicActiveContourFilterType::New();

    typedef itk::ChangeInformationImageFilter<
        InternalImageType >  CenterFilterType;
    CenterFilterType::Pointer center = CenterFilterType::New();
    center->CenterImageOn();

    typedef   itk::BoundedReciprocalImageFilter<
        InternalImageType,
        InternalImageType >  ReciprocalFilterType;
    ReciprocalFilterType::Pointer reciprocal = ReciprocalFilterType::New();


    const double propagationScaling = 5;
    const double shapePriorScaling = 1.0;
    geodesicActiveContour->SetPropagationScaling(propagationScaling);
    geodesicActiveContour->SetShapePriorScaling(shapePriorScaling);
    geodesicActiveContour->SetCurvatureScaling(1.0);
    geodesicActiveContour->SetAdvectionScaling(5.0);
    geodesicActiveContour->SetMaximumRMSError(0.005);
    geodesicActiveContour->SetNumberOfIterations(2000);
    geodesicActiveContour->SetNumberOfLayers(4);

    sigmoid->SetAlpha(40);
    sigmoid->SetBeta(40);
    sigmoid->SetOutputMinimum(0.0);
    sigmoid->SetOutputMaximum(1.0);

    //center->SetInput(itkImage);
    //smoothing->SetInput(itkImage);
    // gradientMagnitude->SetInput(smoothing->GetOutput());
    // reciprocal->SetInput(gradientMagnitude->GetOutput());
    sigmoid->SetInput(itkImage);
    fastMarching->SetInput(sigmoid->GetOutput());
    geodesicActiveContour->SetInput(fastMarching->GetOutput());
    geodesicActiveContour->SetFeatureImage(sigmoid->GetOutput());
    thresholder->SetInput(geodesicActiveContour->GetOutput());


    smoothing->SetTimeStep(0.04);
    smoothing->SetNumberOfIterations(2);
    //smoothing->SetConductanceParameter(9.0);

    const double sigma = 0.5;
    gradientMagnitude->SetSigma(sigma);

    typedef FastMarchingFilterType::NodeContainer  NodeContainer;
    typedef FastMarchingFilterType::NodeType       NodeType;
    NodeContainer::Pointer seeds = NodeContainer::New();
    InternalImageType::IndexType  seedPosition;


    const double initialDistance = 15;
    NodeType node;
    const double seedValue = -initialDistance;
    node.SetValue(seedValue);
    seeds->Initialize();
    for (int i = 0; i<pSeedPointSet->GetPointSet()->GetNumberOfPoints(); i++)
    {
        inputImage->GetGeometry()->WorldToIndex(pSeedPointSet->GetPoint(i), seedPosition);
        node.SetIndex(seedPosition);
        seeds->InsertElement(i, node);
    }


    fastMarching->SetTrialPoints(seeds);
    fastMarching->SetSpeedConstant(1.0);
    fastMarching->SetNormalizationFactor(1);
    fastMarching->SetStoppingValue(150);
    fastMarching->SetOutputRegion(
        itkImage->GetBufferedRegion());
    fastMarching->SetOutputSpacing(
        itkImage->GetSpacing());
    fastMarching->SetOutputOrigin(
        itkImage->GetOrigin());


    const unsigned int numberOfPCAModes = pcaImageList.size();
    MITK_INFO << "Principal Components Number£º" << numberOfPCAModes;
    typedef itk::PCAShapeSignedDistanceFunction<
        double,
        3,
        InternalImageType >     ShapeFunctionType;
    ShapeFunctionType::Pointer shape = ShapeFunctionType::New();
    shape->SetNumberOfPrincipalComponents(numberOfPCAModes);

    std::vector<InternalImageType::Pointer> shapeModeImages(numberOfPCAModes);
    for (unsigned int k = 0; k < numberOfPCAModes; ++k)
    {
        mitk::Image* im = pcaImageList.at(k);
        Float3DImageType::Pointer iim;
        mitk::CastToItkImage(im, iim);
        shapeModeImages[k] = iim;
    }
    shape->SetMeanImage(itkMeanImage);
    shape->SetPrincipalComponentImages(shapeModeImages);


    ShapeFunctionType::ParametersType pcaStandardDeviations(numberOfPCAModes);
    pcaStandardDeviations.SetElement(2, 2.5249e+08);
    pcaStandardDeviations.SetElement(1, 1.35526e+08);
    pcaStandardDeviations.SetElement(0, 5.22946e+07);
    shape->SetPrincipalComponentStandardDeviations(pcaStandardDeviations);


    typedef itk::TranslationTransform<double>    TransformType;
    TransformType::Pointer transform = TransformType::New();
    //itk::Vector<double, 3> translate;
    //translate[0] = m_PointSet->GetPoint(0)[0] - mitkMeanImage->GetVtkImageData()->GetCenter()[0];
    //translate[1] = m_PointSet->GetPoint(0)[1] - mitkMeanImage->GetVtkImageData()->GetCenter()[1];
    //translate[2] = m_PointSet->GetPoint(0)[2] - mitkMeanImage->GetVtkImageData()->GetCenter()[2];
    //MITK_INFO << "Shape Translate:" << translate[0] <<", "<< translate[1] << ", " << translate[2];
    //transform->Translate(-translate);
    shape->SetTransform(transform);

    typedef itk::ShapePriorMAPCostFunction<
        InternalImageType,
        InternalPixelType >     CostFunctionType;
    CostFunctionType::Pointer costFunction = CostFunctionType::New();
    CostFunctionType::WeightsType weights;
    weights[0] = 1.0;  // weight for contour fit term
    weights[1] = 20.0; // weight for image fit term
    weights[2] = 1.0;  // weight for shape prior term
    weights[3] = 1.0;  // weight for pose prior term
    costFunction->SetWeights(weights);

    CostFunctionType::ArrayType mean(shape->GetNumberOfShapeParameters());
    CostFunctionType::ArrayType stddev(shape->GetNumberOfShapeParameters());
    mean.Fill(0.0);
    stddev.Fill(1.0);
    costFunction->SetShapeParameterMeans(mean);
    costFunction->SetShapeParameterStandardDeviations(stddev);

    typedef itk::OnePlusOneEvolutionaryOptimizer    OptimizerType;
    OptimizerType::Pointer optimizer = OptimizerType::New();

    typedef itk::Statistics::NormalVariateGenerator GeneratorType;
    GeneratorType::Pointer generator = GeneratorType::New();
    generator->Initialize(20020702);
    optimizer->SetNormalVariateGenerator(generator);

    MITK_INFO << "Name of shape parameters :" << shape->GetNumberOfParameters();
    OptimizerType::ScalesType scales(shape->GetNumberOfParameters());
    scales.Fill(1.0);
    for (unsigned int k = 0; k < numberOfPCAModes; k++)
    {
        scales[k] = 200;  // scales for the pca mode multiplier
    }
    // scales[numberOfPCAModes] = 350.0;  // scale for 2D rotation
    //scales[numberOfPCAModes+1] = 350.0;  // scale for 2D rotation
    //scales[numberOfPCAModes+2] = 350.0;  // scale for 2D rotation
    optimizer->SetScales(scales);


    double initRadius = 1.05;
    double grow = 2.0;
    double shrink = pow(grow, -0.25);
    optimizer->Initialize(initRadius, grow, shrink);
    optimizer->SetEpsilon(1.0e-6); // minimal search radius
    optimizer->SetMaximumIteration(15);

    ShapeFunctionType::ParametersType parameters(
        shape->GetNumberOfParameters());
    parameters.Fill(0.0);
    parameters[numberOfPCAModes] = inputImage->GetVtkImageData()->GetCenter()[0] - pSeedPointSet->GetPoint(0)[0];
    parameters[numberOfPCAModes + 1] = inputImage->GetVtkImageData()->GetCenter()[1] - pSeedPointSet->GetPoint(0)[1];
    parameters[numberOfPCAModes + 2] = inputImage->GetVtkImageData()->GetCenter()[2] - pSeedPointSet->GetPoint(0)[2];



    geodesicActiveContour->SetShapeFunction(shape);
    geodesicActiveContour->SetCostFunction(costFunction);
    geodesicActiveContour->SetOptimizer(optimizer);
    geodesicActiveContour->SetInitialParameters(parameters);
    typedef CommandIterationUpdate<GeodesicActiveContourFilterType> CommandType;
    CommandType::Pointer observer = CommandType::New();
    geodesicActiveContour->AddObserver(itk::IterationEvent(), observer);
    observer->SetObject(this);
    IQF_MitkSurfaceTool* pSurfaceTool = (IQF_MitkSurfaceTool*)m_pMain->GetInterfacePtr(QF_MitkSurface_Tool);
    observer->SetSurfaceTool(pSurfaceTool);

    //center->Update();
    //ImportITKImage(center->GetOutput(), "center");
    // smoothing->Update();
    // ImportITKImage(smoothing->GetOutput(), "smoothing");
    //  gradientMagnitude->Update();
    // ImportITKImage(gradientMagnitude->GetOutput(), "gradientMagnitude");
    //  reciprocal->Update();
    // ImportITKImage(reciprocal->GetOutput(), "reciprocal"); 
    sigmoid->Update();
    sigmoid->GetOutput()->SetObjectName("sigmoid");
    //ImportITKImage(sigmoid->GetOutput(), "sigmoid");
    fastMarching->Update();
    fastMarching->GetOutput()->SetObjectName("fastmarching");
    //ImportITKImage(fastMarching->GetOutput(), "fastmarching");

    /* for (int  i=0;i<geodesicActiveContour->GetNumberOfInputs();i++)
    {
    MITK_INFO << "iniput "<<i<<" :" << geodesicActiveContour->GetInput(i)->GetObjectName();
    }

    return;*/
    try
    {
        thresholder->Update();
    }
    catch (itk::ExceptionObject &excep)
    {
        MITK_ERROR << "Exception caught: " << excep.GetDescription();

        return;
    }

   // ImportITKImage(geodesicActiveContour->GetOutput(), "gac");
   // ImportITKImage(thresholder->GetOutput(), "threshold");
}
