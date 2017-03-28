#ifndef IQF_TubularTracking_h__
#define IQF_TubularTracking_h__

#pragma once
#include "XMarker.h"
#include "vtkSmartPointer.h"
#include "vtkImageData.h"
class ParameterField;
typedef vtkSmartPointer<vtkImageData> ImageType;

const char QF_Algorithm_TubularTracking[] = "QF_Algorithm_TubularTracking";

class IQF_TubularTracking
{
public:
    virtual void SetInputImage(ImageType image)=0;
    virtual void SetSeedPoints(XMarkerList& list)=0;
    virtual void SetParameterField(ParameterField& parameterField)=0;
    virtual void Track()=0;
    virtual void GetOutput(std::vector< std::vector<Vector3> >& result) = 0;
};


class ParameterField
{
public:
    ParameterField() :_minRadius(0.5), _maxRadius(2.0), _initRadius(1.0), _useInitRadius(false), _maxAngle(0.01), _terminateThresold(0.0) {}
    void setMinRadius(double minRadius) { _minRadius = minRadius; }
    double minRadius() { return _minRadius; }

    void setMaxRadius(double maxRadius) { _maxRadius = maxRadius; }
    double maxRadius() { return _maxRadius; }

    void setInitRadius(double initRadius) { _initRadius = initRadius; }
    double initRadius() { return _initRadius; }

    void setUseInitRadius(bool useInitRadius) { _useInitRadius = useInitRadius; }
    bool useInitRadius() { return _useInitRadius; }

    void setMaxAngle(double maxAngle) { _maxAngle = maxAngle; }
    double maxAngle() { return _maxAngle; }

    void setTerminateThresold(double terminateThresold) { _terminateThresold = terminateThresold; }
    double terminateThresold() { return _terminateThresold; }

    void setWindowSizeFactor(double windowSizeFactor) { _windowSizeFactor = windowSizeFactor; }
    double windowSizeFactor() { return _windowSizeFactor; }

    void setStepLength(double stepLength) { _stepLength = stepLength; }
    double stepLength() { return _stepLength; }

    void setNeedInvertIntensity(bool needInvertIntensity) { _needInvertIntensity = needInvertIntensity; }
    bool needInvertIntensity() { return _needInvertIntensity; }
private:
    double _minRadius;
    double _maxRadius;
    double _initRadius;
    bool _useInitRadius;
    double _maxAngle;
    double _terminateThresold;
    double _windowSizeFactor;
    double _stepLength;
    bool _needInvertIntensity;
};


#endif // IQF_TubularTracking_h__
