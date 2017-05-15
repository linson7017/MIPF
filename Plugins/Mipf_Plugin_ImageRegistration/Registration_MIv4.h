#ifndef Registration_MIv4_h__
#define Registration_MIv4_h__

#include <itkImage.h>

class MIRegistrationv4Parameters
{
    int m_numberOfIterations;
    int m_numberOfHistogramBins;
    double m_rateOfSpatialSamples;
    double m_minimumStepLength;
    double m_maximumStepLength;
public:
    MIRegistrationv4Parameters() :
        m_numberOfIterations(500),
        m_numberOfHistogramBins(128),
        m_rateOfSpatialSamples(0.01),
        m_minimumStepLength(0.0001),
        m_maximumStepLength(1.0)
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

};


template<class FixedImageType, class MovingImageType>
class RegistrationMIv4
{
public:
    RegistrationMIv4();
    ~RegistrationMIv4();
    void Start(const FixedImageType* fixedImage, const MovingImageType* movingImage, FixedImageType* resultImage, itk::Matrix<double, 4, 4>& initTransformMatrix = itk::Matrix<double, 4, 4>());

    MIRegistrationv4Parameters RegistrationParameters;
};

#include "Registration_MIv4.hpp"

#endif // Registration_MI_h__

