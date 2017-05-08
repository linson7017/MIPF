#include "Registration_MIv4.h"

#include <itkVersorRigid3DTransform.h>
#include <itkRegularStepGradientDescentOptimizerv4.h>
#include <itkMattesMutualInformationImageToImageMetricv4.h>
#include <itkCenteredTransformInitializer.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkImageRegistrationMethodv4.h>
#include <itkResampleImageFilter.h>
#include "itkImage.h" 

class CommandIterationUpdatev4 : public itk::Command
{
public:
    typedef  CommandIterationUpdatev4   Self;
    typedef  itk::Command             Superclass;
    typedef itk::SmartPointer<Self>   Pointer;
    itkNewMacro(Self);
protected:
    CommandIterationUpdatev4() { m_transform = TransformType::New(); };
public:
    typedef itk::VersorRigid3DTransform< double > TransformType;
    typedef itk::RegularStepGradientDescentOptimizerv4<double> OptimizerType;
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
            //    m_notifier->EmitRegistrationComplete();
            return;
        }

        if (!itk::IterationEvent().CheckEvent(&event))
        {
            return;
        }

        m_transform->SetParameters(optimizer->GetCurrentPosition());
        TransformType::MatrixType matrix = m_transform->GetMatrix();
        TransformType::OffsetType offset = m_transform->GetOffset();

        std::vector<double> result;
        std::cout << "current parameter:";
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                result.push_back(matrix.GetVnlMatrix().get(i, j));
                std::cout << matrix.GetVnlMatrix().get(i, j) << ", ";
            }

            result.push_back(offset.GetElement(i));
        }
        result.push_back(0);
        result.push_back(0);
        result.push_back(0);
        result.push_back(1);
        std::cout << std::endl;
        std::cout << "Value: " << optimizer->GetValue() << std::endl;
        //   m_notifier->EmitRegistrationIterationEnd(result);
    }

    // void SetNotifier(const QSharedPointer<SpItkNotifier> &notifier)
    // {
    //     m_notifier = notifier;
    // }
private:
    TransformType::Pointer  m_transform;
    // QSharedPointer<SpItkNotifier> m_notifier;
};

template<class FixedImageType, class MovingImageType>
RegistrationMIv4<FixedImageType, MovingImageType>::RegistrationMIv4()
{

}

template<class FixedImageType, class MovingImageType>
RegistrationMIv4<FixedImageType, MovingImageType>::~RegistrationMIv4()
{

}

template<class FixedImageType, class MovingImageType>
void RegistrationMIv4<FixedImageType, MovingImageType>::Start(const FixedImageType* fixedImage, const MovingImageType* movingImage, FixedImageType* resultImage, itk::Matrix<double, 4, 4>& initTransformMatrix)
{
    typedef itk::VersorRigid3DTransform<double> TransformType;
    typedef itk::RegularStepGradientDescentOptimizerv4<double> OptimizerType;
    typedef itk::LinearInterpolateImageFunction<MovingImageType, double> InterpolatorType;
    typedef typename itk::MattesMutualInformationImageToImageMetricv4<FixedImageType, MovingImageType> MetricType;
    typedef itk::ImageRegistrationMethodv4<FixedImageType, MovingImageType, TransformType> RegistrationType;

    //**************Transform****************//  
    TransformType::Pointer transform = TransformType::New();
    TransformType::Pointer initTransform = TransformType::New();
    TransformType::OffsetType offset = initTransform->GetOffset();
    offset[0] = -initTransformMatrix[0][3];
    offset[1] = -initTransformMatrix[1][3];
    offset[2] = -initTransformMatrix[2][3];
    initTransform->SetOffset(offset);

    typedef itk::CenteredTransformInitializer<
        TransformType,
        FixedImageType,
        MovingImageType >  TransformInitializerType;
    TransformInitializerType::Pointer initializer =
        TransformInitializerType::New();
    initializer->SetTransform(initTransform);
    initializer->SetFixedImage(fixedImage);
    initializer->SetMovingImage(movingImage);
    initializer->MomentsOn();
    initializer->InitializeTransform();

    // transform->SetCenter(centerFixed);

    //**************Optimizer****************//
    OptimizerType::Pointer optimizer = OptimizerType::New();
    /*optimizer->SetNumberOfIterations(RegistrationParameters.GetNumberOfIterations());
    optimizer->SetMinimumStepLength(RegistrationParameters.GetMinimumStepLength());
    optimizer->SetMaximumStepLength(RegistrationParameters.GetMaximumStepLength());
    optimizer->SetMinimize(true);
    optimizer->SetRelaxationFactor(0.8);*/


    typedef OptimizerType::ScalesType       OptimizerScalesType;
    OptimizerScalesType optimizerScales(initTransform->GetNumberOfParameters());
    const double translationScale = 1.0 / 1000.0;
    optimizerScales[0] = 1.0;
    optimizerScales[1] = 1.0;
    optimizerScales[2] = 1.0;
    optimizerScales[3] = translationScale;
    optimizerScales[4] = translationScale;
    optimizerScales[5] = translationScale;
    optimizer->SetScales(optimizerScales);
    optimizer->SetNumberOfIterations(200);
    optimizer->SetLearningRate(0.2);
    optimizer->SetMinimumStepLength(0.001);
    optimizer->SetReturnBestParametersAndValue(true);

    CommandIterationUpdatev4::Pointer observer = CommandIterationUpdatev4::New();
    // observer->SetNotifier(m_notifier);
    optimizer->AddObserver(itk::IterationEvent(), observer);
    optimizer->AddObserver(itk::EndEvent(), observer);

    //**************Metric****************//
    MetricType::Pointer metric = MetricType::New();
    FixedImageType::RegionType fixedImageRegion = fixedImage->GetBufferedRegion();
    const unsigned int numberOfPixels = fixedImageRegion.GetNumberOfPixels();
    unsigned long numberOfSpatialSamples = static_cast<unsigned long>(numberOfPixels * RegistrationParameters.GetRateOfSpatialSamples());
    metric->SetNumberOfHistogramBins(RegistrationParameters.GetNumberOfHistogramBins());
//    metric->SetNumberOfSpatialSamples(numberOfSpatialSamples);


    //**************Interpolator****************//
    InterpolatorType::Pointer interpolator = InterpolatorType::New();


    //**************Registration****************//
    RegistrationType::Pointer registration = RegistrationType::New();
//    registration->SetTransform(transform);
    registration->SetInitialTransform(initTransform);
    registration->SetMetric(metric);
    registration->SetOptimizer(optimizer);
//    registration->SetInterpolator(interpolator);
    registration->SetFixedImage(fixedImage);
    registration->SetMovingImage(movingImage);
//    registration->SetFixedImageRegion(fixedImageRegion);
    registration->SetNumberOfLevels(1);

    RegistrationType::ShrinkFactorsArrayType shrinkFactorsPerLevel;
    shrinkFactorsPerLevel.SetSize(1);
    shrinkFactorsPerLevel[0] = 1;

    RegistrationType::SmoothingSigmasArrayType smoothingSigmasPerLevel;
    smoothingSigmasPerLevel.SetSize(1);
    smoothingSigmasPerLevel[0] = 0;
    registration->SetSmoothingSigmasPerLevel(smoothingSigmasPerLevel);
    registration->SetShrinkFactorsPerLevel(shrinkFactorsPerLevel);

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
        return;
    }

    // transform->SetParameters(registration->GetLastTransformParameters());


    typedef itk::ResampleImageFilter<
        MovingImageType,
        FixedImageType >    ResampleFilterType;
    TransformType::Pointer finalTransform = TransformType::New();

    const TransformType::ParametersType finalParameters =
        registration->GetOutput()->Get()->GetParameters();
    finalTransform->SetFixedParameters(registration->GetOutput()->Get()->GetFixedParameters());
    finalTransform->SetParameters(finalParameters);

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