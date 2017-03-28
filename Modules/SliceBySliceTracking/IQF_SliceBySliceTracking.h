#ifndef IQF_SliceBySliceTracking_h__
#define IQF_SliceBySliceTracking_h__

#pragma once
#include "XMarker.h"
#include "vtkSmartPointer.h"
#include "vtkImageData.h"
class ParameterField;
typedef vtkSmartPointer<vtkImageData> ImageType;
const char QF_Algorithm_SliceBySliceTracking[] = "QF_Algorithm_SliceBySliceTracking";

class IQF_SliceBySliceTracking
{
public:
    virtual void SetInputImage(ImageType image) = 0;
    virtual void SetSeedPoints(XMarkerList& list) = 0;
   // virtual void SetParameterField(ParameterField& parameterField) = 0;
    virtual void Track() = 0;
    virtual void GetOutput(std::vector< std::vector<Vector3> >& result)=0;
};

#endif // IQF_SliceBySliceTracking_h__
