#include "LSSegmentation.h"


#include "mitkImageCast.h"

#include "ITKImageTypeDef.h"
#include "MitkSegmentation/IQF_MitkSurfaceTool.h"

#include "ITKImageTypeDef.h"

#include "itkBinaryThresholdImageFilter.h"

#include "itkFastMarchingImageFilter.h"
#include "itkSigmoidImageFilter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"

#include "iqf_main.h"

template<class TFilter>
class LSSegmentation::CommandIterationUpdate : public itk::Command
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
        std::cout << filter->GetRMSChange() << " " << std::endl;
       // std::cout << filter->GetCurrentParameters() << std::endl;

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

        /*auto resultSurface = vtkSmartPointer<vtkPolyData>::New();
        m_pSurfaceTool->ExtractSurface(image, resultSurface.Get());*/

        emit m_object->SignalInteractionEnd(image);
    }

    void SetObject(LSSegmentation* object)
    {
        m_object = object;
    }
    void SetSurfaceTool(IQF_MitkSurfaceTool* pTool)
    {
        m_pSurfaceTool = pTool;
    }
private:
    LSSegmentation* m_object;
    IQF_MitkSurfaceTool* m_pSurfaceTool;
};

LSSegmentation::LSSegmentation(mitk::DataStorage* pDataStorage, QF::IQF_Main* pMain) :m_pDataStorage(pDataStorage), m_pMain(pMain)
{
}


LSSegmentation::~LSSegmentation()
{
}

void LSSegmentation::SlotDoSegmentation(const mitk::Image::Pointer& inputImage, const  mitk::PointSet::Pointer& pSeedPointSet)
{
    if (!inputImage)
    {
        return;
    }

    Float3DImageType::Pointer itkImage;
    mitk::CastToItkImage(inputImage, itkImage);

    typedef itk::CurvatureAnisotropicDiffusionImageFilter<Float3DImageType, Float3DImageType> SmoothType;
    typedef itk::GradientMagnitudeRecursiveGaussianImageFilter<Float3DImageType, Float3DImageType> GradientType;
    typedef itk::SigmoidImageFilter<Float3DImageType, Float3DImageType> SigmoidType;
    typedef itk::FastMarchingImageFilter<Float3DImageType, Float3DImageType> FastMarchingType;
    
    typedef itk::BinaryThresholdImageFilter<Float3DImageType, UChar3DImageType> BinaryThresholdType;

    SmoothType::Pointer smoothFilter = SmoothType::New();
    GradientType::Pointer gradienFilter = GradientType::New();
    SigmoidType::Pointer sigmoidFilter = SigmoidType::New();
    FastMarchingType::Pointer fmFilter = FastMarchingType::New();
    m_levelFilter = LevelSetType::New();
    BinaryThresholdType::Pointer btFilter = BinaryThresholdType::New();

     smoothFilter->SetInput(itkImage);
    // gradienFilter->SetInput(smoothFilter->GetOutput());
    sigmoidFilter->SetInput(smoothFilter->GetOutput());
    fmFilter->SetInput(sigmoidFilter->GetOutput());
    m_levelFilter->SetInput(fmFilter->GetOutput());
    m_levelFilter->SetFeatureImage(sigmoidFilter->GetOutput());
    btFilter->SetInput(m_levelFilter->GetOutput());

    //smooth
    //smoothFilter->SetTimeStep(0.03);
    //smoothFilter->SetNumberOfIterations(2);
    //smoothFilter->SetConductanceParameter(9.0);

    //gradient
   // gradienFilter->SetSigma(0.5);

    //sigmoid
    sigmoidFilter->SetAlpha(PARAMETERS.Alpha);
    sigmoidFilter->SetBeta(PARAMETERS.Beta);
    sigmoidFilter->SetOutputMinimum(0.0);
    sigmoidFilter->SetOutputMaximum(1.0);

    //fast marching
    typedef FastMarchingType::NodeContainer  NodeContainer;
    typedef FastMarchingType::NodeType  NodeType;
    Float3DImageType::IndexType  seedPosition;
    NodeContainer::Pointer seeds = NodeContainer::New();
    const double initialDistance = 5.0;
    NodeType node;
    const double seedValue = -initialDistance;
    node.SetValue(seedValue);
    seeds->Initialize();
    for (int i = 0; i < pSeedPointSet->GetPointSet()->GetNumberOfPoints(); i++)
    {
        inputImage->GetGeometry()->WorldToIndex(pSeedPointSet->GetPoint(i), seedPosition);
        node.SetIndex(seedPosition);
        seeds->InsertElement(i, node);
    }
    fmFilter->SetTrialPoints(seeds);
    fmFilter->SetSpeedConstant(1.0);
    fmFilter->SetNormalizationFactor(1);
    fmFilter->SetStoppingValue(PARAMETERS.StoppingValue);
    fmFilter->SetOutputRegion(itkImage->GetBufferedRegion());
    fmFilter->SetOutputSpacing(itkImage->GetSpacing());
    fmFilter->SetOutputOrigin(itkImage->GetOrigin());


    //levelset
    m_levelFilter->SetAutoGenerateSpeedAdvection(true);
    m_levelFilter->SetPropagationScaling(PARAMETERS.PropagationScaling);
    m_levelFilter->SetAdvectionScaling(PARAMETERS.AdvectionScaling);
    m_levelFilter->SetCurvatureScaling(PARAMETERS.CurvatureScaling);
    m_levelFilter->SetMaximumRMSError(0.02);
    m_levelFilter->SetNumberOfIterations(PARAMETERS.NumberOfInteraction);
    m_levelFilter->SetIsoSurfaceValue(100);
    typedef CommandIterationUpdate<LevelSetType> CommandType;
    CommandType::Pointer observer = CommandType::New();
    m_levelFilter->AddObserver(itk::IterationEvent(), observer);
    observer->SetObject(this);
    IQF_MitkSurfaceTool* pSurfaceTool = (IQF_MitkSurfaceTool*)m_pMain->GetInterfacePtr(QF_MitkSurface_Tool);
    observer->SetSurfaceTool(pSurfaceTool);

    //threshold
    btFilter->SetLowerThreshold(-1000);
    btFilter->SetUpperThreshold(0);
    btFilter->SetOutsideValue(0);
    btFilter->SetInsideValue(255);

    try
    {
        btFilter->Update();
        emit SignalSegmentationFinished();
    }
    catch (itk::ExceptionObject & excep)
    {
        std::cerr << "Exception caught !" << std::endl;
        std::cerr << excep << std::endl;
        return;
    }
}


void LSSegmentation::SlotStopSegmentation()
{
    MITK_INFO << "SlotStopSegmentation";
     if (m_levelFilter.IsNotNull())
     {
         
         m_levelFilter->SetNumberOfIterations(1);
     }
}