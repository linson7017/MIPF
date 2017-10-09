#include "LevelSetASMSegmentationView.h"

#include "iqf_main.h"

#include "ITKImageTypeDef.h"

#include "MitkMain/IQF_MitkDataManager.h"
#include "MitkMain/IQF_MitkRenderWindow.h"

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
#include "itkNormalVariateGenerator.h"
#include "vnl/vnl_sample.h"
#include "itkNumericSeriesFileNames.h"

#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkFastMarchingImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkSpatialFunctionImageEvaluatorFilter.h"

#include "itkCurvatureFlowImageFilter.h"


template<class TFilter>
class CommandIterationUpdate : public itk::Command
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
    }
};

LevelSetASMSegmentationView::LevelSetASMSegmentationView(QF::IQF_Main* pMain, QWidget* parent) :QWidget(parent), m_pMain(pMain)
{
    m_ui.setupUi(this);

    IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    IQF_MitkRenderWindow* pRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
    m_ui.ImageSelector->SetDataStorage(pDataManager->GetDataStorage());
    m_ui.MeanImageSelector->SetDataStorage(pDataManager->GetDataStorage());
    m_ui.FMImageSelector->SetDataStorage(pDataManager->GetDataStorage());


    m_ui.ImageSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Image>::New());
    m_ui.MeanImageSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Image>::New());
    m_ui.FMImageSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Image>::New());


    connect(m_ui.AddPCAImageBtn, &QPushButton::clicked, this, &LevelSetASMSegmentationView::AddPCAImage);
    connect(m_ui.RemovePCAImageBtn, &QPushButton::clicked, this, &LevelSetASMSegmentationView::RemovePCAImage);
    connect(m_ui.ApplyBtn, &QPushButton::clicked, this, &LevelSetASMSegmentationView::Apply);



    //seed widget
    m_ui.PointListWidget->SetMultiWidget(pRenderWindow->GetMitkStdMultiWidget());

    if (m_PointSet.IsNull())
        m_PointSet = mitk::PointSet::New();
    if (m_PointSetNode.IsNull())
    {
        m_PointSetNode = mitk::DataNode::New();
        m_PointSetNode->SetData(m_PointSet);
        m_PointSetNode->SetName("pca seed points");
        m_PointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true));
        m_PointSetNode->SetProperty("label", mitk::StringProperty::New("P"));
        m_PointSetNode->SetProperty("layer", mitk::IntProperty::New(100));
    }
    pDataManager->GetDataStorage()->Add(m_PointSetNode);
    m_ui.PointListWidget->SetPointSetNode(m_PointSetNode);


}


LevelSetASMSegmentationView::~LevelSetASMSegmentationView()
{

}

void LevelSetASMSegmentationView::AddPCAImage()
{
    IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    std::vector<mitk::DataNode::Pointer> selectedNodes = pDataManager->GetSelectedNodes();

    for (auto node : selectedNodes)
    {
        m_ui.PCAImageList->addItem(node->GetName().c_str());
    }

}

void LevelSetASMSegmentationView::RemovePCAImage()
{
    IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    mitk::DataNode::Pointer selectedNode = pDataManager->GetSelectedNodes().at(0);

    m_ui.PCAImageList->takeItem(m_ui.PCAImageList->currentRow());
}

void LevelSetASMSegmentationView::Apply()
{
    IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);

    if (!m_ui.ImageSelector->GetSelectedNode()|| !m_ui.MeanImageSelector->GetSelectedNode())
    {
        MITK_ERROR << "Please Select Image Or Mean Image!";
        return;
    }

    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());
    mitk::Image* mitkMeanImage = dynamic_cast<mitk::Image*>(m_ui.MeanImageSelector->GetSelectedNode()->GetData());
    mitk::Image* mitkFMImage = dynamic_cast<mitk::Image*>(m_ui.FMImageSelector->GetSelectedNode()->GetData());

    if (!mitkImage||!mitkMeanImage)
    {
        MITK_ERROR << "Please Select Image!";
        return;
    }

    typedef Float3DImageType InternalImageType;  
    typedef float InternalPixelType;
    Float3DImageType::Pointer itkImage;
    Float3DImageType::Pointer itkMeanImage;
    Float3DImageType::Pointer itkFMImage;

    mitk::CastToItkImage(mitkImage, itkImage);
    mitk::CastToItkImage(mitkMeanImage, itkMeanImage);
    mitk::CastToItkImage(mitkFMImage, itkFMImage);


    typedef itk::BinaryThresholdImageFilter<
        InternalImageType,
        UChar3DImageType    >       ThresholdingFilterType;
    ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();
    thresholder->SetLowerThreshold(-1000.0);
    thresholder->SetUpperThreshold(1000);
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


    const double propagationScaling = 1.0;
    const double shapePriorScaling = 1.0;
    geodesicActiveContour->SetPropagationScaling(propagationScaling);
    geodesicActiveContour->SetShapePriorScaling(shapePriorScaling);
    geodesicActiveContour->SetCurvatureScaling(1.0);
    geodesicActiveContour->SetAdvectionScaling(1.0);
    geodesicActiveContour->SetMaximumRMSError(0.005);
    geodesicActiveContour->SetNumberOfIterations(400);
    geodesicActiveContour->SetNumberOfLayers(4);

    //center->SetInput(itkImage);
    smoothing->SetInput(itkImage);
    gradientMagnitude->SetInput(smoothing->GetOutput());
    reciprocal->SetInput(gradientMagnitude->GetOutput());
    geodesicActiveContour->SetInput(itkFMImage);
    geodesicActiveContour->SetFeatureImage(reciprocal->GetOutput());
    thresholder->SetInput(geodesicActiveContour->GetOutput());


    smoothing->SetTimeStep(0.04);
    smoothing->SetNumberOfIterations(5);
    //smoothing->SetConductanceParameter(9.0);

    const double sigma = 0.5;
    gradientMagnitude->SetSigma(sigma);

    typedef FastMarchingFilterType::NodeContainer  NodeContainer;
    typedef FastMarchingFilterType::NodeType       NodeType;
    NodeContainer::Pointer seeds = NodeContainer::New();
    InternalImageType::IndexType  seedPosition;


    const double initialDistance = 0.0;
    NodeType node;
    const double seedValue = -initialDistance;
    node.SetValue(seedValue);
    seeds->Initialize();
    for (int i=0;i<m_PointSet->GetPointSet()->GetNumberOfPoints();i++)
    {
        mitkImage->GetGeometry()->WorldToIndex(m_PointSet->GetPoint(i), seedPosition);
        node.SetIndex(seedPosition);
        seeds->InsertElement(i, node);
    }


    fastMarching->SetTrialPoints(seeds);
    fastMarching->SetSpeedConstant(1.0);

    fastMarching->SetOutputRegion(
        itkImage->GetBufferedRegion());
    fastMarching->SetOutputSpacing(
        itkImage->GetSpacing());
    fastMarching->SetOutputOrigin(
        itkImage->GetOrigin());


    const unsigned int numberOfPCAModes = m_ui.PCAImageList->count();
    typedef itk::PCAShapeSignedDistanceFunction<
        double,
        3,
        InternalImageType >     ShapeFunctionType;
    ShapeFunctionType::Pointer shape = ShapeFunctionType::New();
    shape->SetNumberOfPrincipalComponents(numberOfPCAModes);

    std::vector<InternalImageType::Pointer> shapeModeImages(numberOfPCAModes);
    for (unsigned int k = 0; k < numberOfPCAModes; ++k)
    {
        mitk::Image* im = dynamic_cast<mitk::Image*>(pDataManager->GetDataStorage()->GetNamedNode(m_ui.PCAImageList->item(k)->text().toStdString())->GetData());
        Float3DImageType::Pointer iim;
        mitk::CastToItkImage(im, iim);
        shapeModeImages[k] = iim;
    }
    shape->SetMeanImage(itkMeanImage);
    shape->SetPrincipalComponentImages(shapeModeImages);


    ShapeFunctionType::ParametersType pcaStandardDeviations(numberOfPCAModes);
    pcaStandardDeviations.Fill(1.0);
    shape->SetPrincipalComponentStandardDeviations(pcaStandardDeviations);


    typedef itk::AffineTransform<double>    TransformType;
    TransformType::Pointer transform = TransformType::New();
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


    OptimizerType::ScalesType scales(shape->GetNumberOfParameters());
    scales.Fill(1.0);
    for (unsigned int k = 0; k < numberOfPCAModes; k++)
    {
        scales[k] = 20.0;  // scales for the pca mode multiplier
    }
    scales[numberOfPCAModes] = 350.0;  // scale for 2D rotation
    optimizer->SetScales(scales);


    double initRadius = 1.05;
    double grow = 1.1;
    double shrink = pow(grow, -0.25);
    optimizer->Initialize(initRadius, grow, shrink);
    optimizer->SetEpsilon(1.0e-6); // minimal search radius
    optimizer->SetMaximumIteration(15);

    ShapeFunctionType::ParametersType parameters(
        shape->GetNumberOfParameters());
    parameters.Fill(0.0);
    
    parameters[numberOfPCAModes + 1] = 0.0; // startX
    parameters[numberOfPCAModes + 2] = 0.0; // startY
    parameters[numberOfPCAModes + 3] = 0.0; // startz


    geodesicActiveContour->SetShapeFunction(shape);
    geodesicActiveContour->SetCostFunction(costFunction);
    geodesicActiveContour->SetOptimizer(optimizer);
    geodesicActiveContour->SetInitialParameters(parameters);
    typedef CommandIterationUpdate<GeodesicActiveContourFilterType> CommandType;
    CommandType::Pointer observer = CommandType::New();
    geodesicActiveContour->AddObserver(itk::IterationEvent(), observer);

   // thresholder->Update();
    //center->Update();
    //ImportITKImage(center->GetOutput(), "center");
    smoothing->Update();
    ImportITKImage(smoothing->GetOutput(), "smoothing");
    gradientMagnitude->Update();
    ImportITKImage(gradientMagnitude->GetOutput(), "gradientMagnitude");
    reciprocal->Update();
    ImportITKImage(reciprocal->GetOutput(), "reciprocal");    
    //fastMarching->Update();
   // ImportITKImage(fastMarching->GetOutput(), "fastmarching");

    thresholder->Update();
    ImportITKImage(thresholder->GetOutput(),"result");
}

template<class TImageType>
void LevelSetASMSegmentationView::ImportITKImage(TImageType* itkImage, const char* name, mitk::DataNode* parentNode)
{
    IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    mitk::Image::Pointer image;
    mitk::CastToMitkImage(itkImage, image);
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(image);
    node->SetName(name);

    pDataManager->GetDataStorage()->Add(node, parentNode);
}
