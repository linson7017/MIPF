#include "Registration_MMI.h"

#include <itkVersorRigid3DTransform.h>
#include <itkVersorRigid3DTransformOptimizer.h>
#include <itkMattesMutualInformationImageToImageMetric.h>
#include <itkCenteredTransformInitializer.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkImageRegistrationMethod.h>
#include <itkMultiResolutionImageRegistrationMethod.h>
#include <itkResampleImageFilter.h>
#include "itkImage.h" 

#include "ItkNotifier.h"

class CommandMMIIterationUpdate : public itk::Command
{
public:
    typedef  CommandMMIIterationUpdate   Self;
    typedef  itk::Command             Superclass;
    typedef itk::SmartPointer<Self>   Pointer;
    itkNewMacro(Self);
protected:
    CommandMMIIterationUpdate()
    {
        m_transform = TransformType::New(); 
        m_currentLevel = 1;
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
            if (m_notifier&&m_currentLevel==3)
            {
                m_notifier->RegistrationFinished();
                m_currentLevel = 1;
            }
            m_currentLevel++;
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
            m_notifier->SignalRegistrationIterationEnd(m);
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
private:
    TransformType::Pointer  m_transform;
    ItkNotifier* m_notifier;
    int m_currentLevel;
};

template<class FixedImageType, class MovingImageType>
RegistrationMMI<FixedImageType, MovingImageType>::RegistrationMMI()
{
}

template<class FixedImageType, class MovingImageType>
RegistrationMMI<FixedImageType, MovingImageType>::~RegistrationMMI()
{
}

template<class FixedImageType, class MovingImageType>
void RegistrationMMI<FixedImageType, MovingImageType>::Start(const FixedImageType* fixedImage, const MovingImageType* movingImage, FixedImageType* resultImage, itk::Matrix<double, 4, 4>& initTransformMatrix)
{
    typedef itk::VersorRigid3DTransform<double> TransformType;
    typedef itk::VersorRigid3DTransformOptimizer OptimizerType;
    typedef itk::LinearInterpolateImageFunction<MovingImageType, double> InterpolatorType;
    typedef typename itk::MattesMutualInformationImageToImageMetric<FixedImageType, MovingImageType> MetricType;
    typedef itk::MultiResolutionImageRegistrationMethod<FixedImageType, MovingImageType> RegistrationType;

    //**************Transform****************//
    TransformType::Pointer initTransform = TransformType::New();
    TransformType::OffsetType offset = initTransform->GetOffset();
    offset[0] = -initTransformMatrix[0][3];
    offset[1] = -initTransformMatrix[1][3];
    offset[2] = -initTransformMatrix[2][3];
    initTransform->SetOffset(offset);

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
    double translationScale = 1.0 / 1000;
    optimizerScales[0] = 1.0;//original
    optimizerScales[1] = 1.0;
    optimizerScales[2] = 1.0;
    optimizerScales[3] = translationScale;
    optimizerScales[4] = translationScale;
    optimizerScales[5] = translationScale;
    optimizer->SetScales(optimizerScales);

    CommandMMIIterationUpdate::Pointer observer = CommandMMIIterationUpdate::New();
    observer->SetNotifier(m_notifier);
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
    registration->SetFixedImageRegion(fixedImageRegion);
    registration->SetNumberOfLevels(3);

    typedef RegistrationInterfaceCommand<RegistrationType,OptimizerType> CommandType;
    CommandType::Pointer command = CommandType::New();
    registration->AddObserver(itk::IterationEvent(), command);

    try
    {
        std::cout << "Multi Resolution Registration Start !" << std::endl;
        std::cout << "Parameter: " << std::endl;
        std::cout << "Maximum Step Length: " << RegistrationParameters.GetMaximumStepLength()<<std::endl;
        std::cout << "Minimum Step Length: " << RegistrationParameters.GetMinimumStepLength() << std::endl;
        std::cout << "Number Of Iterations: " << RegistrationParameters.GetNumberOfIterations() << std::endl;

        registration->Update();
        std::cout << "Optimizer stop condition = "
            << registration->GetOptimizer()->GetStopConditionDescription()
            << std::endl;
    }
    catch (itk::ExceptionObject & err)
    {
        std::cout << "ExceptionObject caught !" << std::endl;
        std::cout << err << std::endl;
        emit m_notifier->SignalRegistrationFinsihed();
        return;
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