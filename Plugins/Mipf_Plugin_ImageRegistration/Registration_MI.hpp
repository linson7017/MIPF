#include "Registration_MI.h"

class CommandIterationUpdate : public itk::Command
{
public:
    typedef  CommandIterationUpdate   Self;
    typedef  itk::Command             Superclass;
    typedef itk::SmartPointer<Self>   Pointer;
    itkNewMacro(Self);
protected:
    CommandIterationUpdate()
    { 
        m_transform = TransformType::New(); 
        m_currentLevel = 0;
        m_maxLevel = 1;
    };
public:
    typedef itk::VersorRigid3DTransform< double > TransformType;
    typedef itk::VersorRigid3DTransformOptimizer OptimizerType;
    typedef   const OptimizerType *              OptimizerPointer;
    void Execute(itk::Object *caller, const itk::EventObject & event)
    {
        Execute((const itk::Object *)caller, event);
    }
    void Execute(const itk::Object * object, const itk::EventObject & event)
    {
        OptimizerPointer optimizer =
            dynamic_cast<OptimizerPointer>(object);

        if (itk::EndEvent().CheckEvent(&event))
        {
            m_currentLevel = 0;
            if (optimizer->GetNumberOfIterations() == 1)
            {
                m_notifier->RESULT.SetResultMessage("Stopped ! \n");
                m_notifier->RegistrationFinished();
            }
            return;
        }

        if (!itk::IterationEvent().CheckEvent(&event))
        {
            return;
        }

        m_transform->SetParameters(optimizer->GetCurrentPosition());
        TransformType::MatrixType matrix = m_transform->GetMatrix();
        TransformType::OffsetType offset = m_transform->GetOffset();

        itk::Matrix<double, 4, 4> m;
        m.SetIdentity();
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                m[i][j] = matrix.GetVnlMatrix().get(i, j);
            }
            m[i][3] = offset[i];
        }
        if (m_notifier)
        {
            m_notifier->RegistrationIterationEnd(m);
        }
        
        //output
        std::cout << optimizer->GetCurrentIteration() << "   ";
        std::cout << optimizer->GetValue() << "   ";
        std::cout << optimizer->GetCurrentPosition() << std::endl;

    }
    void SetNotifier(ItkNotifier* notifier)
    {
        m_notifier = notifier;
    }
    void SetMaxLevel(int level) { m_maxLevel = level; }
private:
    TransformType::Pointer  m_transform;
     ItkNotifier* m_notifier;
     int m_currentLevel;
     int m_maxLevel;
};

template<class FixedImageType, class MovingImageType>
RegistrationMI<FixedImageType, MovingImageType>::RegistrationMI()
{
    m_beginStepLength = -1.0;
}

template<class FixedImageType, class MovingImageType>
RegistrationMI<FixedImageType, MovingImageType>::~RegistrationMI()
{
}

template<class FixedImageType, class MovingImageType>
void RegistrationMI<FixedImageType, MovingImageType>::Start(const FixedImageType* fixedImage, const MovingImageType* movingImage, FixedImageType* resultImage, itk::Matrix<double, 4, 4>& initTransformMatrix)
{
    typedef itk::VersorRigid3DTransform<double> TransformType;
    typedef itk::VersorRigid3DTransformOptimizer OptimizerType;
    typedef itk::LinearInterpolateImageFunction<MovingImageType, double> InterpolatorType;
    typedef typename itk::MattesMutualInformationImageToImageMetric<FixedImageType, MovingImageType> MetricType;
    typedef itk::ImageRegistrationMethod<FixedImageType, MovingImageType> RegistrationType;
   // typedef itk::MultiResolutionImageRegistrationMethod<FixedImageType, MovingImageType> RegistrationType;

    //**************Transform****************//
    TransformType::Pointer initTransform = TransformType::New();
    TransformType::MatrixType matrix = initTransform->GetMatrix();
    TransformType::OffsetType offset = initTransform->GetOffset();
    vnl_matrix_fixed<double, 4, 4> initMatrix = initTransformMatrix.GetVnlMatrix();
    offset[0] = initMatrix[0][3];
    offset[1] = initMatrix[1][3];
    offset[2] = initMatrix[2][3];
    initTransform->SetMatrix(matrix);
    initTransform->SetOffset(offset);

    //**************Optimizer****************//
    OptimizerType::Pointer optimizer = OptimizerType::New();
    m_Optimizer = optimizer.GetPointer();
    optimizer->SetNumberOfIterations(RegistrationParameters.GetNumberOfIterations());
    optimizer->SetMinimumStepLength(RegistrationParameters.GetMinimumStepLength());
    optimizer->SetMaximumStepLength(m_beginStepLength>0?m_beginStepLength:RegistrationParameters.GetMaximumStepLength());
    optimizer->SetMinimize(true);
    optimizer->SetRelaxationFactor(0.8);

    TransformType::Pointer transform = TransformType::New();
    typedef OptimizerType::ScalesType       OptimizerScalesType;
    OptimizerScalesType optimizerScales(transform->GetNumberOfParameters());
    FixedImageType::RegionType::SizeType fixedImageSize = fixedImage->GetLargestPossibleRegion().GetSize();
    FixedImageType::SpacingType fixedImageSpacing = fixedImage->GetSpacing();
    double imageSizeInMM[3];
    imageSizeInMM[0] = fixedImageSize[0] * fixedImageSpacing[0];
    imageSizeInMM[1] = fixedImageSize[1] * fixedImageSpacing[1];
    imageSizeInMM[2] = fixedImageSize[2] * fixedImageSpacing[2];
    double translationScale = 1.0 / sqrt(imageSizeInMM[0]* imageSizeInMM[0]+ imageSizeInMM[1] * imageSizeInMM[1]+ imageSizeInMM[2] * imageSizeInMM[2]);
    optimizerScales[0] = 1.0;//original
    optimizerScales[1] = 1.0;
    optimizerScales[2] = 1.0;
    optimizerScales[3] = translationScale;
    optimizerScales[4] = translationScale;
    optimizerScales[5] = translationScale;
    optimizer->SetScales(optimizerScales);

    CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
    observer->SetNotifier(m_notifier);
    observer->SetMaxLevel(m_resolutionLevel);
    optimizer->AddObserver(itk::IterationEvent(), observer);
    optimizer->AddObserver(itk::EndEvent(), observer);

    //**************Metric****************//
    MetricType::Pointer metric = MetricType::New();
    FixedImageType::RegionType fixedImageRegion = fixedImage->GetBufferedRegion();
    const unsigned int numberOfPixels = fixedImageRegion.GetNumberOfPixels();
    unsigned long numberOfSpatialSamples = static_cast<unsigned long>(numberOfPixels * RegistrationParameters.GetRateOfSpatialSamples());
    metric->SetNumberOfHistogramBins(RegistrationParameters.GetNumberOfHistogramBins());
    metric->SetNumberOfSpatialSamples(numberOfSpatialSamples);


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
        m_notifier->RESULT.AppendMessage(registration->GetOptimizer()->GetStopConditionDescription());
    }
    catch (itk::ExceptionObject & err)
    {
        /*std::cout << "ExceptionObject caught !" << std::endl;
        std::cout << err << std::endl;*/
        m_notifier->RESULT.SetResultCode(RESULT_ERROR);
        m_notifier->RESULT.SetResultMessage(err.what());
    }
    if (m_quitAfterFinished)
    {
        m_notifier->RegistrationFinished();
    }

   // transform->SetParameters(registration->GetLastTransformParameters());

    if (resultImage)
    {
        typedef itk::ResampleImageFilter<
            MovingImageType,
            FixedImageType >    ResampleFilterType;
        TransformType::Pointer finalTransform = TransformType::New();

        finalTransform->SetParameters(registration->GetLastTransformParameters());
        finalTransform->SetFixedParameters(transform->GetFixedParameters());

        ResampleFilterType::Pointer resample = ResampleFilterType::New();

        resample->SetTransform(finalTransform);
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

template<class FixedImageType, class MovingImageType>
void RegistrationMI<FixedImageType, MovingImageType>::Stop()
{
    if (m_Optimizer)
    {
        m_Optimizer->SetNumberOfIterations(1);
    }
}