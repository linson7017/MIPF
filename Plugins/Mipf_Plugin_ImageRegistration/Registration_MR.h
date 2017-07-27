#ifndef Registration_MR_h__
#define Registration_MR_h__
#include "Registration_Base.h"

#include <itkVersorRigid3DTransform.h>
#include <itkVersorRigid3DTransformOptimizer.h>
#include <itkMattesMutualInformationImageToImageMetric.h>
#include <itkCenteredTransformInitializer.h>
#include <itkLinearInterpolateImageFunction.h>
//#include <itkImageRegistrationMethod.h>
#include <itkMultiResolutionImageRegistrationMethod.h>
#include <itkResampleImageFilter.h>
#include "itkImage.h" 

#include "ItkNotifier.h"

class MRRegistrationParameters
{
	int m_numberOfIterations;
	int m_numberOfHistogramBins;
	double m_rateOfSpatialSamples;
	double m_minimumStepLength;
	double m_maximumStepLength;

	int m_numberOflevels;

public:
	MRRegistrationParameters() :
		m_numberOfIterations(200),
		m_numberOfHistogramBins(128),
		m_rateOfSpatialSamples(0.01),
		m_minimumStepLength(0.001),
		m_maximumStepLength(2.0),
		m_numberOflevels(2)
	{}
	void SetNumberOfIterations(int numberOfIterations) { m_numberOfIterations = numberOfIterations; }
	int GetNumberOfIterations() { return m_numberOfIterations; }
	void SetNumberOfHistogramBins(int numberOfHistogramBins) { m_numberOfHistogramBins = numberOfHistogramBins; }
	int GetNumberOfHistogramBins() { return m_numberOfHistogramBins; }
	void SetRateOfSpatialSamples(double rateOfSpatialSamples) { m_rateOfSpatialSamples = rateOfSpatialSamples; }
	double GetRateOfSpatialSamples() { return m_rateOfSpatialSamples; }
	void SetMinimumStepLength(double minimumStepLength) { m_minimumStepLength = minimumStepLength; }
	double GetMinimumStepLength() { return m_minimumStepLength; }
	void SetMaximumStepLength(double maximumStepLength) { m_maximumStepLength = maximumStepLength; }
	double GetMaximumStepLength() { return m_maximumStepLength; }

	void SetNumberOfLevels(int numberOfLevels) { m_numberOflevels = numberOfLevels; }
	int GetNumberOfLevels() { return m_numberOflevels; }

};

class ItkNotifier;

template<class FixedImageType, class MovingImageType>
class RegistrationMR : public RegistrationBase<FixedImageType, MovingImageType>
{
public:
	RegistrationMR();
	~RegistrationMR();
	void Start(const FixedImageType* fixedImage, const MovingImageType* movingImage, FixedImageType* resultImage, itk::Matrix<double, 4, 4>& initTransformMatrix = itk::Matrix<double, 4, 4>());
	void Stop();
	MRRegistrationParameters RegistrationParameters;

private:
	itk::RegularStepGradientDescentBaseOptimizer* m_Optimizer;
};

#include "Registration_MR.hpp"

#endif // Registration_MI_h__
