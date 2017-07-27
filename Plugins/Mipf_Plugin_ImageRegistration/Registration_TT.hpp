#include "Registration_TT.h"

 class CommandIterationUpdateTT : public itk::Command
 {
 public:
 	typedef  CommandIterationUpdateTT   Self;
 	typedef  itk::Command             Superclass;
 	typedef itk::SmartPointer<Self>   Pointer;
 	itkNewMacro(Self);
 protected:
 	CommandIterationUpdateTT()
 	{
 		m_transform = TransformType::New();
 		m_currentLevel = 0;
 		m_maxLevel = 1;
 	};
 public:
 	typedef itk::TranslationTransform< double > TransformType;
 	typedef itk::RegularStepGradientDescentOptimizer OptimizerType;
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
//  		TransformType::MatrixType matrix = m_transform->GetMatrix();  //这两行
//  		TransformType::OffsetType offset = m_transform->GetOffset();  //无此接口
		TransformType::OutputVectorType offset = m_transform->GetOffset();

 		itk::Matrix<double, 4, 4> m;
 		m.SetIdentity();
 		for (int i = 0; i < 3; i++)
 		{
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
RegistrationTT<FixedImageType, MovingImageType>::RegistrationTT()
{
}

template<class FixedImageType, class MovingImageType>
RegistrationTT<FixedImageType, MovingImageType>::~RegistrationTT()
{
}

template<class FixedImageType, class MovingImageType>
void RegistrationTT<FixedImageType, MovingImageType>::Start(const FixedImageType* fixedImage, const MovingImageType* movingImage, FixedImageType* resultImage, itk::Matrix<double, 4, 4>& initTransformMatrix)
{
	typedef itk::TranslationTransform<double,3> TransformType;
	typedef itk::RegularStepGradientDescentOptimizer OptimizerType;
	typedef itk::LinearInterpolateImageFunction<MovingImageType, double> InterpolatorType;
	typedef typename itk::MattesMutualInformationImageToImageMetric<FixedImageType, MovingImageType> MetricType;
	typedef itk::ImageRegistrationMethod<FixedImageType, MovingImageType> RegistrationType;
	// typedef itk::MultiResolutionImageRegistrationMethod<FixedImageType, MovingImageType> RegistrationType;

	//**************Transform****************//
	TransformType::Pointer initTransform = TransformType::New();

	//**************Optimizer****************//
	OptimizerType::Pointer optimizer = OptimizerType::New();
	m_Optimizer = optimizer.GetPointer();
	optimizer->SetNumberOfIterations(RegistrationParameters.GetNumberOfIterations());
	optimizer->SetMinimumStepLength(RegistrationParameters.GetMinimumStepLength());
	optimizer->SetMaximumStepLength(RegistrationParameters.GetMaximumStepLength());
	optimizer->SetMinimize(true);
	optimizer->SetRelaxationFactor(0.8);

	TransformType::Pointer transform = TransformType::New();
	typedef OptimizerType::ScalesType       OptimizerScalesType;
	OptimizerScalesType optimizerScales(transform->GetNumberOfParameters());
	double translationScale = 1.0 ;
	//optimizerScales[0] = 1.0;//original
	//optimizerScales[1] = 1.0;
	//optimizerScales[2] = 1.0;
	optimizerScales[0] = translationScale;
	optimizerScales[1] = translationScale;
	optimizerScales[2] = translationScale;
	optimizer->SetScales(optimizerScales);

	CommandIterationUpdateTT::Pointer observer = CommandIterationUpdateTT::New();
	observer->SetNotifier(m_notifier);
	//observer->SetMaxLevel(m_resolutionLevel);
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
	typedef RegistrationType::ParametersType ParametersType;
	ParametersType initialParameters(transform->GetNumberOfParameters());
	initialParameters[0] = initTransformMatrix[0][3];  // Initial offset in mm along X
	initialParameters[1] = initTransformMatrix[1][3];  // Initial offset in mm along Y
	initialParameters[2] = initTransformMatrix[2][3];  // Initial offset in mm along Z

	RegistrationType::Pointer registration = RegistrationType::New();
	registration->SetTransform(transform);
	registration->SetInitialTransformParameters(initialParameters);
	//registration->SetInitialTransformParameters(initTransform->GetParameters());
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
        m_notifier->RESULT.AppendMessage(registration->GetOptimizer()->GetStopConditionDescription());
	}
	catch (itk::ExceptionObject & err)
	{
        m_notifier->RESULT.SetResultCode(RESULT_ERROR);
        m_notifier->RESULT.SetResultMessage(err.what());
	}
    if (m_quitAfterFinished)
    {
        m_notifier->RegistrationFinished();
    }
    

	TransformType::Pointer finalTrans = TransformType::New();
	finalTrans->SetParameters(registration->GetLastTransformParameters());
	TransformType::OutputVectorType finalOffset = finalTrans->GetOffset();
	finalMatrix.SetIdentity();
	for (int i = 0; i < 3; i++)
	{
		finalMatrix[i][3] = finalOffset[i];
	}
	

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
void RegistrationTT<FixedImageType, MovingImageType>::Stop()
{
	if (m_Optimizer)
	{
		m_Optimizer->SetNumberOfIterations(1);
	}
}

template<class FixedImageType, class MovingImageType>
itk::Matrix<double, 4, 4> RegistrationTT<FixedImageType, MovingImageType>::GetFinalMatrix()
{
	return finalMatrix;
}

