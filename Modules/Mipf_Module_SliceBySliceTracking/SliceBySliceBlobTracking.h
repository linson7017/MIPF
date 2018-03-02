#ifndef SliceBySliceBlobTracking_h__
#define SliceBySliceBlobTracking_h__

#pragma once
#include "IQF_SliceBySliceTracking.h"
#include "vtkImageData.h"
#include "vtkSmartPointer.h"
#include "XMarker.h"
#include "VTKImageProperties.h"

class SliceBySliceBlobTracking:public IQF_SliceBySliceTracking
{
public:
    SliceBySliceBlobTracking();
    ~SliceBySliceBlobTracking();
    virtual void SetInputImage(ImageType image);
    virtual void SetSeedPoints(XMarkerList& list);
    // virtual void SetParameterField(ParameterField& parameterField) = 0;
    virtual void Track(int trackMode = BLOB_TRACK);
    virtual void GetOutput(std::vector< std::vector<Vector3> >& result) { result = _trackedGraphs; }
private:
    bool filterPoint(Vector3& p1, Vector3& p2, double dis = 12.0);
    void fitOneSlice(int sliceIndex, int startSlice, int* extent, std::vector<Vector3> refPoints, VTKImageProperties& mp);
    void fitOneSliceByBrightestPixel(int sliceIndex, int startSlice, int* extent, std::vector<Vector3>& refPoints, VTKImageProperties& mp, std::vector<Vector3>& directs);

    vtkImageData* _inputImage;
    XMarkerList _initPointsXMarkerList;
    double _refDistance2;
    std::vector< std::vector<Vector3> > _trackedGraphs;
};
#endif // SliceBySliceBlobTracking_h__
