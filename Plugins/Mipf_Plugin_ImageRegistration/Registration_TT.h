#ifndef Registration_TT_h__
#define Registration_TT_h__
#include "Registration_Base.h"

#include <itkTranslationTransform.h>
#include <itkRegularStepGradientDescentOptimizer.h>
#include <itkMattesMutualInformationImageToImageMetric.h>
#include <itkCenteredTransformInitializer.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkImageRegistrationMethod.h>
//#include <itkMultiResolutionImageRegistrationMethod.h>
#include <itkResampleImageFilter.h>
#include "itkImage.h" 

#include "ItkNotifier.h"

class TTRegistrationParameters
{
	int m_numberOfIterations;
	int m_numberOfHistogramBins;
	double m_rateOfSpatialSamples;
	double m_minimumStepLength;
	double m_maximumStepLength;
public:
    TTRegistrationParameters() :
        m_numberOfIterations(200),
		m_numberOfHistogramBins(128),
		//m_numberOfHistogramBins(24),
		m_rateOfSpatialSamples(0.01),
		//m_rateOfSpatialSamples(10000),
		m_minimumStepLength(0.2),
		m_maximumStepLength(10)
	{}
	void SetNumberOfIterations(int numberOfIterations) { m_numberOfIterations = numberOfIterations; }
	int GetNumberOfIterations() { return m_numberOfIterations; }
	void SetNumberOfHistogramBins(int numberOfHistogramBins) { m_numberOfHistogramBins = numberOfHistogramBins; }
	int GetNumberOfHistogramBins() { return m_numberOfHistogramBins; }
	void SetRateOfSpatialSamplesSamples(double rateOfSpatialSamples) { m_rateOfSpatialSamples = rateOfSpatialSamples; }
	double GetRateOfSpatialSamples() { return m_rateOfSpatialSamples; }
	void SetMinimumStepLength(double minimumStepLength) { m_minimumStepLength = minimumStepLength; }
	double GetMinimumStepLength() { return m_minimumStepLength; }
	void SetMaximumStepLength(double maximumStepLength) { m_maximumStepLength = maximumStepLength; }
	double GetMaximumStepLength() { return m_maximumStepLength; }

};

class ItkNotifier;

template<class FixedImageType, class MovingImageType>
class RegistrationTT : public RegistrationBase<FixedImageType, MovingImageType>
{
public:
	RegistrationTT();
	~RegistrationTT();
	void Start(const FixedImageType* fixedImage, const MovingImageType* movingImage, FixedImageType* resultImage, itk::Matrix<double, 4, 4>& initTransformMatrix = itk::Matrix<double, 4, 4>());
	void Stop();
	TTRegistrationParameters RegistrationParameters;

	itk::Matrix<double, 4, 4> GetFinalMatrix();

private:
	itk::RegularStepGradientDescentBaseOptimizer* m_Optimizer;
	itk::Matrix<double, 4, 4> finalMatrix;
};

#include "Registration_TT.hpp"




#endif // Registration_MI_h__
