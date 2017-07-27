#include "Registration_SFD.h"

//class CommandIterationUpdateLSM : public itk::Command
//{
//public:
//	typedef  CommandIterationUpdateLSM   Self;
//	typedef  itk::Command             Superclass;
//	typedef itk::SmartPointer<Self>   Pointer;
//	itkNewMacro(Self);
//protected:
//	CommandIterationUpdateLSM()
//	{
//		m_transform = TransformType::New();
//		m_currentLevel = 0;
//		m_maxLevel = 1;
//	};
//public:
//	typedef itk::VersorRigid3DTransform< double > TransformType;
//	typedef itk::VersorRigid3DTransformOptimizer OptimizerType;
//	typedef   const OptimizerType *              OptimizerPointer;
//	void Execute(itk::Object *caller, const itk::EventObject & event)
//	{
//		Execute((const itk::Object *)caller, event);
//	}
//	void Execute(const itk::Object * object, const itk::EventObject & event)
//	{
//		OptimizerPointer optimizer =
//			dynamic_cast<OptimizerPointer>(object);
//
//		if (itk::EndEvent().CheckEvent(&event))
//		{
//			if (/*m_notifier&&m_currentLevel== m_maxLevel-1*/1)
//			{
//				m_currentLevel = 0;
//				m_notifier->RegistrationFinished();
//				return;
//			}
//			m_currentLevel++;
//			return;
//		}
//
//		if (!itk::IterationEvent().CheckEvent(&event))
//		{
//			return;
//		}
//
//		m_transform->SetParameters(optimizer->GetCurrentPosition());
//		TransformType::MatrixType matrix = m_transform->GetMatrix();
//		TransformType::OffsetType offset = m_transform->GetOffset();
//
//		itk::Matrix<double, 4, 4> m;
//		m.SetIdentity();
//		for (int i = 0; i < 3; i++)
//		{
//			for (int j = 0; j < 3; j++)
//			{
//				m[i][j] = matrix.GetVnlMatrix().get(i, j);
//			}
//			m[i][3] = offset[i];
//		}
//		if (m_notifier)
//		{
//			m_notifier->RegistrationIterationEnd(m);
//		}
//
//		//output
//		std::cout << optimizer->GetCurrentIteration() << "   ";
//		std::cout << optimizer->GetValue() << "   ";
//		std::cout << optimizer->GetCurrentPosition() << std::endl;
//
//	}
//	void SetNotifier(ItkNotifier* notifier)
//	{
//		m_notifier = notifier;
//	}
//	void SetMaxLevel(int level) { m_maxLevel = level; }
//private:
//	TransformType::Pointer  m_transform;
//	ItkNotifier* m_notifier;
//	int m_currentLevel;
//	int m_maxLevel;
//};
#include "Registration_SFD.h"
#include "itkImageFileWriter.h"
#include "itkMinimumMaximumImageCalculator.h"


class CommandIterationUpdateSFD : public itk::Command
{
public:
	typedef  CommandIterationUpdateSFD                     Self;
	typedef  itk::Command                               Superclass;
	typedef  itk::SmartPointer<CommandIterationUpdateSFD>  Pointer;
	itkNewMacro(CommandIterationUpdateSFD);
protected:
	CommandIterationUpdateSFD() {};

	typedef itk::Image< float, 3 >            InternalImageType;
	typedef itk::Vector< float, 3 >           VectorPixelType;
	typedef itk::Image<  VectorPixelType, 3 > DisplacementFieldType;

	typedef itk::SymmetricForcesDemonsRegistrationFilter<
	                            InternalImageType,
	                            InternalImageType,
	                            DisplacementFieldType>   RegistrationFilterType;

public:

	void Execute(itk::Object *caller, const itk::EventObject & event)
	{
		Execute((const itk::Object *)caller, event);
	}

	void Execute(const itk::Object * object, const itk::EventObject & event)
	{
		const RegistrationFilterType * filter = static_cast<const RegistrationFilterType *>(object);
		if (!(itk::IterationEvent().CheckEvent(&event)))
		{
			return;
		}
		std::cout << filter->GetMetric() << std::endl;
	}
};

template<class FixedImageType, class MovingImageType>
RegistrationSFD<FixedImageType, MovingImageType>::RegistrationSFD()
{
}

template<class FixedImageType, class MovingImageType>
RegistrationSFD<FixedImageType, MovingImageType>::~RegistrationSFD()
{
}

template<class FixedImageType, class MovingImageType>
void RegistrationSFD<FixedImageType, MovingImageType>::Start(const FixedImageType* fixedImage, const MovingImageType* movingImage, FixedImageType* resultImage, itk::Matrix<double, 4, 4>& initTransformMatrix)
{
    //Resample Image
    typedef itk::MinimumMaximumImageCalculator <MovingImageType>
        ImageCalculatorFilterType;
    ImageCalculatorFilterType::Pointer imageCalculatorFilter
        = ImageCalculatorFilterType::New();
    imageCalculatorFilter->SetImage(movingImage);
    imageCalculatorFilter->Compute();

    typedef itk::AffineTransform< double, 3 > ResampleTransformType;
    ResampleTransformType::Pointer resampleTransform = ResampleTransformType::New();
    ResampleTransformType::MatrixType matrix = resampleTransform->GetMatrix();
    ResampleTransformType::OffsetType offset = resampleTransform->GetOffset();
    for (int i=0;i<3;i++)
    {
        for (int j=0;j<3;j++)
        {
            matrix[i][j] = initTransformMatrix[i][j];
        }
        offset[i] = initTransformMatrix[i][3];
    }
    resampleTransform->SetMatrix(matrix);
    resampleTransform->SetOffset(offset);

    typedef itk::ResampleImageFilter<
        MovingImageType,
        FixedImageType >    ResampleFilterType;
    ResampleFilterType::Pointer resample = ResampleFilterType::New();
    resample->SetTransform(resampleTransform);
    resample->SetInput(movingImage);
    resample->SetSize(fixedImage->GetLargestPossibleRegion().GetSize());
    resample->SetOutputOrigin(fixedImage->GetOrigin());
    resample->SetOutputSpacing(fixedImage->GetSpacing());
    resample->SetOutputDirection(fixedImage->GetDirection());
    resample->SetDefaultPixelValue(imageCalculatorFilter->GetMinimum());
    resample->Update();
    

	//**************Matcher****************//
    const unsigned int Dimension = 3;
	typedef itk::HistogramMatchingImageFilter<
		MovingImageType,
		MovingImageType >   MatchingFilterType;
	MatchingFilterType::Pointer matcher = MatchingFilterType::New();
	matcher->SetInput(resample->GetOutput());
	matcher->SetReferenceImage(fixedImage);
	matcher->SetNumberOfHistogramLevels(RegistrationParameters.GetNumberOfHistogramLevels());
	matcher->SetNumberOfMatchPoints(RegistrationParameters.GetNumberOfMatchPoints());
	matcher->ThresholdAtMeanIntensityOn();

	//**************Registration****************//
	typedef itk::Vector< FixedImageType::PixelType, Dimens3D >                VectorPixelType;
	typedef itk::Image<  VectorPixelType, Dimens3D >      DisplacementFieldType;
	typedef itk::SymmetricForcesDemonsRegistrationFilter<
		FixedImageType,
		MovingImageType,
		DisplacementFieldType> RegistrationFilterType;
	RegistrationFilterType::Pointer filter = RegistrationFilterType::New();

	CommandIterationUpdateSFD::Pointer observer = CommandIterationUpdateSFD::New();
	filter->AddObserver(itk::IterationEvent(), observer);

	filter->SetFixedImage(fixedImage);
	filter->SetMovingImage(matcher->GetOutput());

	filter->SetNumberOfIterations(RegistrationParameters.GetNumberOfIterations());
	filter->SetStandardDeviations(RegistrationParameters.GetStandardDeviations());

	filter->Update();

	typedef itk::WarpImageFilter<
		MovingImageType,
		FixedImageType,
		DisplacementFieldType  >     WarperType;
	typedef itk::LinearInterpolateImageFunction<
		MovingImageType,
		double          >  InterpolatorType;
	WarperType::Pointer warper = WarperType::New();
	InterpolatorType::Pointer interpolator = InterpolatorType::New();

	warper->SetInput(resample->GetOutput());
	warper->SetInterpolator(interpolator);
	warper->SetOutputSpacing(fixedImage->GetSpacing());
	warper->SetOutputOrigin(fixedImage->GetOrigin());
	warper->SetOutputDirection(fixedImage->GetDirection());

	warper->SetDisplacementField(filter->GetOutput());

	warper->Update();
    m_notifier->ResultImageGenerated(warper->GetOutput());
    return;

    /*typedef  float                         OutputPixelType;
    typedef itk::Image< OutputPixelType, Dimension > OutputImageType;

    typedef itk::ImageFileWriter< OutputImageType >  WriterType;

    WriterType::Pointer      writer = WriterType::New();

    writer->SetFileName("D:/temp/lin2-2output.mhd");

    writer->SetInput(warper->GetOutput());
    writer->Update();*/
	
	//if (resultImage)
	//{
	//	typedef itk::WarpImageFilter<
	//		MovingImageType,
	//		MovingImageType,
	//		DisplacementFieldType  >     WarperType;
	//	typedef itk::LinearInterpolateImageFunction<
	//		MovingImageType,
	//		double          >  InterpolatorType;
	//	WarperType::Pointer warper = WarperType::New();
	//	InterpolatorType::Pointer interpolator = InterpolatorType::New();
	//	
	//	warper->SetInput(movingImage);
	//	warper->SetInterpolator(interpolator);
	//	warper->SetOutputSpacing(fixedImage->GetSpacing());
	//	warper->SetOutputOrigin(fixedImage->GetOrigin());
	//	warper->SetOutputDirection(fixedImage->GetDirection());
	//	warper->Update();

	//	warper->SetDisplacementField(filter->GetOutput());

	//	resultImage->Graft(resample->GetOutput());
	//}
}

template<class FixedImageType, class MovingImageType>
void RegistrationSFD<FixedImageType, MovingImageType>::Stop()
{	
	//if (m_Optimizer)
	//{
	//	m_Optimizer->SetNumberOfIterations(1);
	//}
}