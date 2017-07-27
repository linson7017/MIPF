#ifndef Registration_SFD_h__
#define Registration_SFD_h__
#include "Registration_Base.h"

#include "itkSymmetricForcesDemonsRegistrationFilter.h"
#include "itkHistogramMatchingImageFilter.h"
#include "itkWarpImageFilter.h"
#include "itkImage.h" 

//#include "ItkNotifier.h"

class SFDRegistrationParameters
{

	int m_numberOfHistogramLevels;
	int m_numberOfMatchPoints;
	int m_numberOfIterations;
	int m_standardDeviations;

public:
	SFDRegistrationParameters() :	

		m_numberOfHistogramLevels(1024),
		m_numberOfMatchPoints(7),
		m_numberOfIterations(50),
		m_standardDeviations(1.0)

	{}
	void SetNumberOfIterations(int numberOfIterations) { m_numberOfIterations = numberOfIterations; }
	int GetNumberOfIterations() { return m_numberOfIterations; }

	void SetNumberOfHistogramLevels(int numberOfHistogramLevels) 
	{ m_numberOfHistogramLevels = numberOfHistogramLevels; }
	int GetNumberOfHistogramLevels() 
	{ return m_numberOfHistogramLevels; }

	void SetNumberOfMatchPoints(int numberOfMatchPoints)
	{ m_numberOfMatchPoints = numberOfMatchPoints; }
	int GetNumberOfMatchPoints()
	{ return m_numberOfMatchPoints;	}

	void SetStandardDeviations(int StandardDeviations)
	{  m_standardDeviations = StandardDeviations;  }
	int GetStandardDeviations()
	{ return m_standardDeviations;	}

};

//class ItkNotifier;

template<class FixedImageType, class MovingImageType>
class RegistrationSFD : public RegistrationBase<FixedImageType, MovingImageType>
{

public:
	RegistrationSFD();
	~RegistrationSFD();
	void Start(const FixedImageType* fixedImage, const MovingImageType* movingImage, FixedImageType* resultImage, itk::Matrix<double, 4, 4>& initTransformMatrix = itk::Matrix<double, 4, 4>());
	void Stop();
	SFDRegistrationParameters RegistrationParameters;

//private:
//	itk::RegularStepGradientDescentBaseOptimizer* m_Optimizer;
};


#include "Registration_SFD.hpp"

#endif // Registration_MI_h__