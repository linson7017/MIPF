#include "LevelSetSegmentationView.h"

#include "mitkImageCast.h"

#include "ITKImageTypeDef.h"

#include "itkBinaryThresholdImageFilter.h"
#include "itkGeodesicActiveContourLevelSetImageFilter.h"
#include "itkFastMarchingImageFilter.h"
#include "itkSigmoidImageFilter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"


#include "LSSegmentation.h"
#include "qf_log.h"

LevelSetSegmentationView::LevelSetSegmentationView(QF::IQF_Main* pMain, QWidget* parent) :QWidget(parent), MitkPluginView(pMain),m_pSegmentation(NULL)
{
    m_ui.setupUi(this);

    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(CreateImagePredicate());

    connect(m_ui.ApplyBtn, &QPushButton::clicked, this, &LevelSetSegmentationView::Apply);
    connect(m_ui.StopBtn, &QPushButton::clicked, this, &LevelSetSegmentationView::Stop);

    //seed widget
    m_ui.SeedWidget->SetMultiWidget(GetMitkRenderWindowInterface()->GetMitkStdMultiWidget());

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
    GetDataStorage()->Add(m_PointSetNode);
    m_ui.SeedWidget->SetPointSetNode(m_PointSetNode);


   
    
}


LevelSetSegmentationView::~LevelSetSegmentationView()
{
}


void LevelSetSegmentationView::SlotInteractionEnd(const mitk::Image::Pointer& image)
{
    //mitk::Surface* mitkSurface = dynamic_cast<mitk::Surface*>(m_ObserveNode->GetData());
    //mitkSurface->SetVtkPolyData(surface);
    m_ObserveNode->SetData(image);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void  LevelSetSegmentationView::SlotSegmentationFinished()
{
    QF_INFO << "Segmentation Finished!";
    disconnect(this, &LevelSetSegmentationView::SignalDoSegmentation, m_pSegmentation, &LSSegmentation::SlotDoSegmentation);
    disconnect(this, &LevelSetSegmentationView::SignalStopSegmentation, m_pSegmentation, &LSSegmentation::SlotStopSegmentation);
    disconnect(m_pSegmentation, &LSSegmentation::SignalInteractionEnd, this, &LevelSetSegmentationView::SlotInteractionEnd);
    disconnect(m_pSegmentation, &LSSegmentation::SignalSegmentationFinished, this, &LevelSetSegmentationView::SlotSegmentationFinished);
    
   // setEnabled(true);
}

void LevelSetSegmentationView::Stop()
{
    QF_INFO << "Stop";
    emit SignalStopSegmentation();
}

void LevelSetSegmentationView::Apply()
{
    IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    if (!m_ui.DataSelector->GetSelectedNode())
    {
        QF_ERROR << "Please Select Image Or Mean Image!";
        return;
    }

    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());

    if (!m_pSegmentation)
    {
        IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        m_pSegmentation = new LSSegmentation(pDataManager->GetDataStorage(), m_pMain);
    }
    //set parameters
    m_pSegmentation->PARAMETERS.Alpha = m_ui.AlphaLE->text().toDouble();
    m_pSegmentation->PARAMETERS.Beta = m_ui.BetaLE->text().toDouble();
    m_pSegmentation->PARAMETERS.PropagationScaling = m_ui.PropagationScalingLE->text().toDouble();
    m_pSegmentation->PARAMETERS.AdvectionScaling = m_ui.AdvectionScalingLE->text().toDouble();
    m_pSegmentation->PARAMETERS.CurvatureScaling = m_ui.CurvatureScalingLE->text().toDouble();
    m_pSegmentation->PARAMETERS.StoppingValue = m_ui.StoppingValueLE->text().toDouble();        
    m_pSegmentation->PARAMETERS.NumberOfInteraction = m_ui.NumberOfInteractionLE->text().toInt();

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

    if (!mitkImage )
    {
        QF_ERROR << "Please Select Image!";
        return;
    }
    if (!m_ObserveNode)
    {
        m_ObserveNode = mitk::DataNode::New();
        m_ObserveNode->SetName("Observer");
        //mitk::Surface::Pointer surface = mitk::Surface::New();
       // m_ObserveNode->SetData(surface);
        pDataManager->GetDataStorage()->Add(m_ObserveNode);
    }
    qRegisterMetaType<mitk::Image::Pointer>("mitk::Image::Pointer");
    qRegisterMetaType<mitk::PointSet::Pointer>("mitk::PointSet::Pointer");

    connect(this, &LevelSetSegmentationView::SignalDoSegmentation, m_pSegmentation, &LSSegmentation::SlotDoSegmentation);
    connect(this, &LevelSetSegmentationView::SignalStopSegmentation, m_pSegmentation, &LSSegmentation::SlotStopSegmentation,  Qt::DirectConnection);
    connect(m_pSegmentation, &LSSegmentation::SignalInteractionEnd, this, &LevelSetSegmentationView::SlotInteractionEnd, Qt::BlockingQueuedConnection);
    connect(m_pSegmentation, &LSSegmentation::SignalSegmentationFinished, this, &LevelSetSegmentationView::SlotSegmentationFinished);

   // setEnabled(false);
    m_segmentationThread->start();
    emit SignalDoSegmentation(mitkImage,  m_PointSet.GetPointer());

}

//void LevelSetSegmentationView::Apply()
//{
//    if (!m_ui.DataSelector->GetSelectedNode())
//    {
//        QF_ERROR << "Please Select Image !";
//        return;
//    }
//
//    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());
//    if (!mitkImage)
//    {
//        return;
//    }
//
//    Float3DImageType::Pointer itkImage;
//    mitk::CastToItkImage(mitkImage, itkImage);
//
//    typedef itk::CurvatureAnisotropicDiffusionImageFilter<Float3DImageType, Float3DImageType> SmoothType;
//    typedef itk::GradientMagnitudeRecursiveGaussianImageFilter<Float3DImageType, Float3DImageType> GradientType;
//    typedef itk::SigmoidImageFilter<Float3DImageType, Float3DImageType> SigmoidType;
//    typedef itk::FastMarchingImageFilter<Float3DImageType, Float3DImageType> FastMarchingType;
//    typedef itk::GeodesicActiveContourLevelSetImageFilter<Float3DImageType, Float3DImageType> LevelSetType;
//    typedef itk::BinaryThresholdImageFilter<Float3DImageType, UChar3DImageType> BinaryThresholdType;
//
//    //SmoothType::Pointer smoothFilter = SmoothType::New();
//   // GradientType::Pointer gradienFilter = GradientType::New();
//    SigmoidType::Pointer sigmoidFilter = SigmoidType::New();
//    FastMarchingType::Pointer fmFilter = FastMarchingType::New();
//    LevelSetType::Pointer levelFilter = LevelSetType::New();
//    BinaryThresholdType::Pointer btFilter = BinaryThresholdType::New();
//
//   // smoothFilter->SetInput(itkImage);
//   // gradienFilter->SetInput(smoothFilter->GetOutput());
//    sigmoidFilter->SetInput(itkImage);
//    fmFilter->SetInput(sigmoidFilter->GetOutput());
//    levelFilter->SetInput(fmFilter->GetOutput());
//    levelFilter->SetFeatureImage(sigmoidFilter->GetOutput());
//    btFilter->SetInput(levelFilter->GetOutput());
//
//    //smooth
//    //smoothFilter->SetTimeStep(0.03);
//    //smoothFilter->SetNumberOfIterations(2);
//    //smoothFilter->SetConductanceParameter(9.0);
//
//    //gradient
//    //gradienFilter->SetSigma(m_ui.SigmaLE->text().toDouble());
//
//    //sigmoid
//    sigmoidFilter->SetAlpha(m_ui.AlphaLE->text().toDouble());
//    sigmoidFilter->SetBeta(m_ui.BetaLE->text().toDouble());
//    sigmoidFilter->SetOutputMinimum(0.0);
//    sigmoidFilter->SetOutputMaximum(1.0);
//
//    //fast marching
//    typedef FastMarchingType::NodeContainer  NodeContainer;
//    typedef FastMarchingType::NodeType  NodeType;
//    Float3DImageType::IndexType  seedPosition;
//    NodeContainer::Pointer seeds = NodeContainer::New();
//    const double initialDistance = 5.0;
//    NodeType node;
//    const double seedValue = -initialDistance;
//    node.SetValue(seedValue);
//    seeds->Initialize();
//    for (int i = 0; i < m_PointSet->GetPointSet()->GetNumberOfPoints(); i++)
//    {
//        mitkImage->GetGeometry()->WorldToIndex(m_PointSet->GetPoint(i), seedPosition);
//        node.SetIndex(seedPosition);
//        seeds->InsertElement(i, node);
//    }
//    fmFilter->SetTrialPoints(seeds);
//    fmFilter->SetSpeedConstant(1.0);
//    fmFilter->SetNormalizationFactor(1);
//    fmFilter->SetStoppingValue(m_ui.StoppingValueLE->text().toDouble());
//    fmFilter->SetOutputRegion( itkImage->GetBufferedRegion());
//    fmFilter->SetOutputSpacing(itkImage->GetSpacing());
//    fmFilter->SetOutputOrigin(itkImage->GetOrigin());
//
//
//    //levelset
//    levelFilter->SetAutoGenerateSpeedAdvection(true);
//    levelFilter->SetPropagationScaling(m_ui.PropagationScalingLE->text().toDouble());
//    levelFilter->SetAdvectionScaling(1.0);
//    levelFilter->SetCurvatureScaling(1.0);
//    levelFilter->SetMaximumRMSError(0.02);
//    levelFilter->SetNumberOfIterations(500);
//    levelFilter->SetIsoSurfaceValue(100);
// 
//    //threshold
//    btFilter->SetLowerThreshold(-1000);
//    btFilter->SetUpperThreshold(0);
//    btFilter->SetOutsideValue(0);
//    btFilter->SetInsideValue(255);
//
//    try
//    {
//        btFilter->Update();
//    }
//    catch (itk::ExceptionObject & excep)
//    {
//        std::cerr << "Exception caught !" << std::endl;
//        std::cerr << excep << std::endl;
//        return;
//    }
//
//    //ImportITKImage(smoothFilter->GetOutput(), "smoothed", m_ui.DataSelector->GetSelectedNode());
//    //ImportITKImage(gradienFilter->GetOutput(), "gradiented", m_ui.DataSelector->GetSelectedNode());
//  //  ImportITKImage(sigmoidFilter->GetOutput(), "sigmoid", m_ui.DataSelector->GetSelectedNode());
//    ImportITKImage(fmFilter->GetOutput(), "fastmarching", m_ui.DataSelector->GetSelectedNode());
//    ImportITKImage(levelFilter->GetOutput(), "levelset", m_ui.DataSelector->GetSelectedNode());
//    ImportITKImage(btFilter->GetOutput(), "result", m_ui.DataSelector->GetSelectedNode());
//
//}
