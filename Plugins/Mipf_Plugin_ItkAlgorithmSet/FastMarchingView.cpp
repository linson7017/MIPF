#include "FastMarchingView.h"


//itk
#include "itkFastMarchingImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkSigmoidImageFilter.h"


#include "mitkImageCast.h"

#include "ITKImageTypeDef.h"

FastMarchingView::FastMarchingView()
{
}


FastMarchingView::~FastMarchingView()
{
}


void FastMarchingView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Image>::New());
    connect(m_ui.DataSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnImageSelectionChanged(const mitk::DataNode *)));

    m_ui.StoppingValueSlider->setMinimum(0);
    m_ui.StoppingValueSlider->setMaximum(10000);
    m_ui.StoppingValueSlider->setPageStep(10);
    m_ui.StoppingValueSlider->setSingleStep(1);
    m_ui.StoppingValueSlider->setValue(2000);
    m_ui.StoppingValueSlider->setDecimals(0);
    m_ui.StoppingValueSlider->setTracking(false);


    m_ui.SigmaValueSlider->setMinimum(0.1);
    m_ui.SigmaValueSlider->setMaximum(5.0);
    m_ui.SigmaValueSlider->setPageStep(0.1);
    m_ui.SigmaValueSlider->setSingleStep(0.01);
    m_ui.SigmaValueSlider->setValue(1.0);
    m_ui.SigmaValueSlider->setTracking(false);

    m_ui.AlphaValueSlider->setMinimum(-10);
    m_ui.AlphaValueSlider->setMaximum(0);
    m_ui.AlphaValueSlider->setPageStep(0.1);
    m_ui.AlphaValueSlider->setSingleStep(0.01);
    m_ui.AlphaValueSlider->setValue(-2.5);
    m_ui.AlphaValueSlider->setTracking(false);

    m_ui.BetaValueSlider->setMinimum(0);
    m_ui.BetaValueSlider->setMaximum(100);
    m_ui.BetaValueSlider->setPageStep(0.1);
    m_ui.BetaValueSlider->setSingleStep(0.01);
    m_ui.BetaValueSlider->setValue(3.5);
    m_ui.BetaValueSlider->setTracking(false);

    m_ui.ThresholdValueSlider->setMinimum(-100);
    m_ui.ThresholdValueSlider->setMaximum(5000);
    m_ui.ThresholdValueSlider->setMinimumValue(-100);
    m_ui.ThresholdValueSlider->setMaximumValue(2000);
    m_ui.ThresholdValueSlider->setDecimals(0);
    m_ui.ThresholdValueSlider->setTracking(false);

    m_ui.SigmaValueSlider->setDecimals(2);
    m_ui.AlphaValueSlider->setDecimals(2);
    m_ui.BetaValueSlider->setDecimals(2);

    //seed point 
    m_pPointSetNode = mitk::DataNode::New();
    m_pPointSet = mitk::PointSet::New();
    m_pPointSetNode->SetData(m_pPointSet);
    m_pPointSetNode->SetName("seed points for fast marching");
    m_pPointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true));
    m_pPointSetNode->SetProperty("layer", mitk::IntProperty::New(1024));
    GetDataStorage()->Add(m_pPointSetNode);
    m_ui.SeedWidget->SetPointSetNode(m_pPointSetNode);
    m_ui.SeedWidget->SetMultiWidget(m_pMitkRenderWindow->GetMitkStdMultiWidget());

    connect(m_ui.SeedWidget, SIGNAL(PointListChanged()), this, SLOT(SeedPointsChanged()));


    itk::SimpleMemberCommand<FastMarchingView>::Pointer pointAddedCommand =
        itk::SimpleMemberCommand<FastMarchingView>::New();
    pointAddedCommand->SetCallbackFunction(this, &FastMarchingView::SeedPointsChanged);
    m_pPointSet->AddObserver(mitk::PointSetAddEvent(), pointAddedCommand);

    itk::SimpleMemberCommand<FastMarchingView>::Pointer pointRemovedCommand =
        itk::SimpleMemberCommand<FastMarchingView>::New();
    pointRemovedCommand->SetCallbackFunction(this, &FastMarchingView::SeedPointsChanged);
    m_pPointSet->AddObserver(mitk::PointSetRemoveEvent(), pointRemovedCommand);

}

WndHandle FastMarchingView::GetPluginHandle()
{
    return this;
}

void FastMarchingView::OnImageSelectionChanged(const mitk::DataNode *node)
{
    if (!node)
    {
        return;
    }
    mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
    if (!image)
    {
        return;
    }
    
}

void FastMarchingView::SeedPointsChanged()
{
    MITK_INFO << "Seed Points Changed";
     if (!m_ui.DataSelector->GetSelectedNode())
     {
         return;
     }
     mitk::Image* image = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());
     if (!image)
     {
         return;
     }

     Float3DImageType::Pointer itkImage;
     mitk::CastToItkImage(image, itkImage);

     typedef itk::BinaryThresholdImageFilter<Float3DImageType, UChar3DImageType> ThresholdingFilterType;
     typedef itk::CurvatureAnisotropicDiffusionImageFilter<Float3DImageType, Float3DImageType> SmoothingFilterType;
     typedef itk::GradientMagnitudeRecursiveGaussianImageFilter<Float3DImageType, Float3DImageType> GradientFilterType;
     typedef itk::SigmoidImageFilter<Float3DImageType, Float3DImageType> SigmoidFilterType;
     typedef itk::FastMarchingImageFilter<Float3DImageType, Float3DImageType> FastMarchingFilterType;
     typedef FastMarchingFilterType::NodeContainer NodeContainer;
     typedef FastMarchingFilterType::NodeType NodeType;


     auto m_ThresholdFilter = ThresholdingFilterType::New();
     m_ThresholdFilter->SetLowerThreshold(m_ui.ThresholdValueSlider->minimumValue());
     m_ThresholdFilter->SetUpperThreshold(m_ui.ThresholdValueSlider->maximumValue());
     m_ThresholdFilter->SetOutsideValue(0);
     m_ThresholdFilter->SetInsideValue(1.0);

     auto m_SmoothFilter = SmoothingFilterType::New();
     m_SmoothFilter->SetTimeStep(0.05);
     m_SmoothFilter->SetNumberOfIterations(2);
     m_SmoothFilter->SetConductanceParameter(9.0);

     auto m_GradientMagnitudeFilter = GradientFilterType::New();
     m_GradientMagnitudeFilter->SetSigma(m_ui.SigmaValueSlider->value());

     auto m_SigmoidFilter = SigmoidFilterType::New();
     m_SigmoidFilter->SetAlpha(m_ui.AlphaValueSlider->value());
     m_SigmoidFilter->SetBeta(m_ui.BetaValueSlider->value());
     m_SigmoidFilter->SetOutputMinimum(0.0);
     m_SigmoidFilter->SetOutputMaximum(1.0);

     auto m_FastMarchingFilter = FastMarchingFilterType::New();
     m_FastMarchingFilter->SetStoppingValue(m_ui.StoppingValueSlider->value());

     m_SmoothFilter->SetInput(itkImage);
     m_GradientMagnitudeFilter->SetInput(m_SmoothFilter->GetOutput());
     m_SigmoidFilter->SetInput(m_GradientMagnitudeFilter->GetOutput());
     m_FastMarchingFilter->SetInput(m_SigmoidFilter->GetOutput());
     m_ThresholdFilter->SetInput(m_FastMarchingFilter->GetOutput());
 
     NodeContainer::Pointer seeds = NodeContainer::New();
     mitk::Point3D tempPosition;
     itk::Index<3> seedPosition;
     NodeType node;     

     for (int i=0;i< m_pPointSet->GetPointSet()->GetNumberOfPoints();i++)
     {
         tempPosition = m_pPointSet->GetPoint(i);
         image->GetGeometry()->WorldToIndex(tempPosition,
             tempPosition);
         
         seedPosition[0] = tempPosition[0];
         seedPosition[1] = tempPosition[1];
         seedPosition[2] = tempPosition[2];

         const double seedValue = 0.0;
         node.SetValue(seedValue);
         node.SetIndex(seedPosition);

         seeds->InsertElement(i, node);
     }
    
     m_FastMarchingFilter->SetTrialPoints(seeds);

     try
     {
         m_ThresholdFilter->Update();
     }
     catch (itk::ExceptionObject &excep)
     {
         MITK_ERROR << "Exception caught: " << excep.GetDescription();

         return;
     }

     ImportITKImage(m_ThresholdFilter->GetOutput(), "fm", m_ui.DataSelector->GetSelectedNode());
}