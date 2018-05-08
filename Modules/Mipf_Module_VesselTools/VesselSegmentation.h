#ifndef VesselSegmentation_h__
#define VesselSegmentation_h__

#pragma once
#include "VesselTools/IQF_VesselSegmentationTool.h"

#include <iostream>

class VesselSegmentation : public IQF_VesselSegmentationTool
{
public:
    VesselSegmentation();
    ~VesselSegmentation();
    virtual void SegmentVessel(vtkImageData* pInput, vtkPolyData* pOutput,
        double dLowerThreshold, double dHigherThreshold, vtkIdList*  pSourceSeedIds, vtkIdList* pTargetSeedIds, int iInitializeMode = 0);

    virtual void SegmentVessel(vtkImageData* pInput, vtkImageData* pOutput,
        double dLowerThreshold, double dHigherThreshold, vtkIdList*  pSourceSeedIds, vtkIdList* pTargetSeedIds, int iInitializeMode = 0);

    virtual void GenerateVesselSurface(vtkImageData* pInput, vtkPolyData* pOutput, double dThreshold);

    virtual void CapSurface(vtkPolyData* input, vtkPolyData*output);

    virtual void Release() { delete this; }
protected:
    void SegmentationInitialize(vtkImageData* pInput, vtkImageData* pOutput, 
        double dLowerThreshold, double dHigherThreshold, vtkIdList*  pSourceSeedIds, vtkIdList* pTargetSeedIds, int mode=0);
};

#endif // VesselSegmentation_h__
