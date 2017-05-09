#include "Registration_MI.h"

#include <itkVersorRigid3DTransform.h>
#include <itkVersorRigid3DTransformOptimizer.h>
#include <itkMattesMutualInformationImageToImageMetric.h>
#include <itkCenteredTransformInitializer.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkImageRegistrationMethod.h>
#include <itkResampleImageFilter.h>
#include "itkImage.h" 

#include "ItkNotifier.h"

class CommandIterationUpdate : public itk::Command
{
public:
    typedef  CommandIterationUpdate   Self;
    typedef  itk::Command             Superclass;
    typedef itk::SmartPointer<Self>   Pointer;
    itkNewMacro(Self);
protected:
    CommandIterationUpdate() { m_transform = TransformType::New(); };
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
            if (m_notifier)
            {
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

        /*std::vector<double> result;
        std::cout << "current parameter:";
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                result.push_back(matrix.GetVnlMatrix().get(i, j));
                std::cout << matrix.GetVnlMatrix().get(i, j) <<", ";
            }

            result.push_back(offset.GetElement(i));
        }
        result.push_back(0);
        result.push_back(0);
        result.push_back(0);
        result.push_back(1);
        std::cout << endl;
        std::cout <<"Value: "<< optimizer->GetValue() <<endl;*/
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
        
    }

    void SetNotifier(ItkNotifier* notifier)
    {
        m_notifier = notifier;
    }
private:
    TransformType::Pointer  m_transform;
     ItkNotifier* m_notifier;
};

template<class FixedImageType, class MovingImageType>
RegistrationMI<FixedImageType, MovingImageType>::RegistrationMI():m_notifier(NULL)
{
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

    CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
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
        emit m_notifier->SignalRegistrationFinsihed();
        return;
    }

   // transform->SetParameters(registration->GetLastTransformParameters());


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

    if (resultImage)
    {
        resultImage->Graft(resample->GetOutput());
    }
}