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
#include <LSASMSegmentation.h>
#include <MitkPluginView.h>

#include "qf_log.h"


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


        /*if (m_observerNode)
        {
            mitk::Image::Pointer image;
            mitk::CastToMitkImage(filter->GetOutput(), image);
            m_observerNode->SetData(image);
            m_observerNode->Modified();
        }*/

    }
    void SetObserverNode(mitk::DataNode* node)
    {
        m_observerNode = node;
    }
private:
    mitk::DataNode* m_observerNode;
};

LevelSetASMSegmentationView::LevelSetASMSegmentationView(QF::IQF_Main* pMain, QWidget* parent) :QWidget(parent), m_pMain(pMain)
{
    m_ui.setupUi(this);

    IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    IQF_MitkRenderWindow* pRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
    m_ui.ImageSelector->SetDataStorage(pDataManager->GetDataStorage());
    m_ui.MeanImageSelector->SetDataStorage(pDataManager->GetDataStorage());

    auto imagePredicate = mitk::NodePredicateAnd::New(mitk::TNodePredicateDataType<mitk::Image>::New(),
        mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(false)));
    m_ui.ImageSelector->SetPredicate(imagePredicate);
    m_ui.MeanImageSelector->SetPredicate(imagePredicate);


    connect(m_ui.AddPCAImageBtn, &QPushButton::clicked, this, &LevelSetASMSegmentationView::AddPCAImage);
    connect(m_ui.RemovePCAImageBtn, &QPushButton::clicked, this, &LevelSetASMSegmentationView::RemovePCAImage);
    connect(m_ui.ApplyBtn, &QPushButton::clicked, this, &LevelSetASMSegmentationView::Apply);
    connect(m_ui.CenterImageBtn, &QPushButton::clicked, this, &LevelSetASMSegmentationView::CenterImage);



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



    if (!m_pSegmentation)
    {
        IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        m_pSegmentation = new LSASMSegmentation(pDataManager->GetDataStorage(),m_pMain);
    }
    
}


LevelSetASMSegmentationView::~LevelSetASMSegmentationView()
{

}

void LevelSetASMSegmentationView::CenterImage()
{
    if (!m_ui.ImageSelector->GetSelectedNode())
    {
        QF_ERROR << "Please Select Image !";
        return;
    }

    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());

    if (!mitkImage )
    {
        QF_ERROR << "Please Select Image!";
        return;
    }

    typedef Float3DImageType InternalImageType;
    typedef float InternalPixelType;
    Float3DImageType::Pointer itkImage;

    mitk::CastToItkImage(mitkImage, itkImage);


    typedef itk::ChangeInformationImageFilter<
        InternalImageType >  CenterFilterType;
    CenterFilterType::Pointer center = CenterFilterType::New();
    center->CenterImageOn();
    center->SetInput(itkImage);
    center->Update();

    mitk::Image::Pointer centeredMitkImage;
    mitk::CastToMitkImage(center->GetOutput(), centeredMitkImage);
    m_ui.ImageSelector->GetSelectedNode()->SetData(centeredMitkImage);


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
    if (!m_ui.ImageSelector->GetSelectedNode() || !m_ui.MeanImageSelector->GetSelectedNode())
    {
        QF_ERROR << "Please Select Image Or Mean Image!";
        return;
    }

    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());
    mitk::Image* mitkMeanImage = dynamic_cast<mitk::Image*>(m_ui.MeanImageSelector->GetSelectedNode()->GetData());


    m_segmentationThread = new QThread;
    m_pSegmentation->moveToThread(m_segmentationThread);
    disconnect(m_segmentationThread, &QThread::finished,
        m_segmentationThread, &QThread::deleteLater);
    disconnect(m_segmentationThread, &QThread::finished,
        m_pSegmentation, &QThread::deleteLater);

    connect(m_segmentationThread, &QThread::finished,
        m_segmentationThread, &QThread::deleteLater);
    connect(m_segmentationThread, &QThread::finished,
        m_pSegmentation, &QThread::deleteLater);

    if (!mitkImage || !mitkMeanImage)
    {
        QF_ERROR << "Please Select Image!";
        return;
    }
    if (!m_ObserveNode)
    {
        m_ObserveNode = mitk::DataNode::New();
        m_ObserveNode->SetName("Observer");
        mitk::Surface::Pointer surface = mitk::Surface::New();
        m_ObserveNode->SetData(surface);
        pDataManager->GetDataStorage()->Add(m_ObserveNode);
    }

    mitk::Image::Pointer resultImage = mitk::Image::New();
    QVector<mitk::Image*> pcaImages;
    for (unsigned int k = 0; k < m_ui.PCAImageList->count(); ++k)
    {
        mitk::Image* im = dynamic_cast<mitk::Image*>(pDataManager->GetDataStorage()->GetNamedNode(m_ui.PCAImageList->item(k)->text().toStdString())->GetData());
        pcaImages.push_back(im);
    }
    qRegisterMetaType<QVector<mitk::Image*>>("QVector<mitk::Image*>");
    connect(this, &LevelSetASMSegmentationView::SignalDoSegmentation, m_pSegmentation, &LSASMSegmentation::SlotDoSegmentation);
    connect(m_pSegmentation, &LSASMSegmentation::SignalInteractionEnd, this, &LevelSetASMSegmentationView::SlotInteractionEnd, Qt::BlockingQueuedConnection);

    m_segmentationThread->start();
    emit SignalDoSegmentation(mitkImage,mitkMeanImage,m_PointSet.GetPointer(), pcaImages,resultImage.GetPointer());

}

void LevelSetASMSegmentationView::SlotInteractionEnd(vtkPolyData* surface)
{
    mitk::Surface* mitkSurface = dynamic_cast<mitk::Surface*>(m_ObserveNode->GetData());
    mitkSurface->SetVtkPolyData(surface);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
}


void LevelSetASMSegmentationView::DoSegmentation()
{
    IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);

    if (!m_ui.ImageSelector->GetSelectedNode() || !m_ui.MeanImageSelector->GetSelectedNode())
    {
        QF_ERROR << "Please Select Image Or Mean Image!";
        return;
    }

    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());
    mitk::Image* mitkMeanImage = dynamic_cast<mitk::Image*>(m_ui.MeanImageSelector->GetSelectedNode()->GetData());

    if (!mitkImage || !mitkMeanImage)
    {
        QF_ERROR << "Please Select Image!";
        return;
    }

    typedef Float3DImageType InternalImageType;
    typedef float InternalPixelType;
    Float3DImageType::Pointer itkImage;
    Float3DImageType::Pointer itkMeanImage;

    mitk::CastToItkImage(mitkImage, itkImage);
    mitk::CastToItkImage(mitkMeanImage, itkMeanImage);




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


    const double propagationScaling = 1.0;
    const double shapePriorScaling = 1.0;
    geodesicActiveContour->SetPropagationScaling(propagationScaling);
    geodesicActiveContour->SetShapePriorScaling(shapePriorScaling);
    geodesicActiveContour->SetCurvatureScaling(1.0);
    geodesicActiveContour->SetAdvectionScaling(1.0);
    geodesicActiveContour->SetMaximumRMSError(0.005);
    geodesicActiveContour->SetNumberOfIterations(400);
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


    const double initialDistance = 0.0;
    NodeType node;
    const double seedValue = -initialDistance;
    node.SetValue(seedValue);
    seeds->Initialize();
    for (int i = 0; i<m_PointSet->GetPointSet()->GetNumberOfPoints(); i++)
    {
        mitkImage->GetGeometry()->WorldToIndex(m_PointSet->GetPoint(i), seedPosition);
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


    const unsigned int numberOfPCAModes = m_ui.PCAImageList->count();
    QF_INFO << "Principal Components Number£º" << numberOfPCAModes;
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
    //QF_INFO << "Shape Translate:" << translate[0] <<", "<< translate[1] << ", " << translate[2];
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

    QF_INFO << "Name of shape parameters :" << shape->GetNumberOfParameters();
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
    parameters[numberOfPCAModes] = mitkMeanImage->GetVtkImageData()->GetCenter()[0] - m_PointSet->GetPoint(0)[0];
    parameters[numberOfPCAModes + 1] = mitkMeanImage->GetVtkImageData()->GetCenter()[1] - m_PointSet->GetPoint(0)[1];
    parameters[numberOfPCAModes + 2] = mitkMeanImage->GetVtkImageData()->GetCenter()[2] - m_PointSet->GetPoint(0)[2];



    geodesicActiveContour->SetShapeFunction(shape);
    geodesicActiveContour->SetCostFunction(costFunction);
    geodesicActiveContour->SetOptimizer(optimizer);
    geodesicActiveContour->SetInitialParameters(parameters);
    typedef CommandIterationUpdate<GeodesicActiveContourFilterType> CommandType;
    CommandType::Pointer observer = CommandType::New();
    geodesicActiveContour->AddObserver(itk::IterationEvent(), observer);
    if (!m_ObserveNode)
    {
        m_ObserveNode = mitk::DataNode::New();
        m_ObserveNode->SetName("Observer");
        pDataManager->GetDataStorage()->Add(m_ObserveNode);
    }
    observer->SetObserverNode(m_ObserveNode);


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
    ImportITKImage(sigmoid->GetOutput(), "sigmoid");
    fastMarching->Update();
    fastMarching->GetOutput()->SetObjectName("fastmarching");
    ImportITKImage(fastMarching->GetOutput(), "fastmarching");

    /* for (int  i=0;i<geodesicActiveContour->GetNumberOfInputs();i++)
    {
    QF_INFO << "iniput "<<i<<" :" << geodesicActiveContour->GetInput(i)->GetObjectName();
    }

    return;*/
    try
    {
        thresholder->Update();
    }
    catch (itk::ExceptionObject &excep)
    {
        QF_ERROR << "Exception caught: " << excep.GetDescription();

        return;
    }

    ImportITKImage(geodesicActiveContour->GetOutput(), "gac");
    ImportITKImage(thresholder->GetOutput(), "threshold");
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
