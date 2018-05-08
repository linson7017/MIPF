#ifndef CenterLineExtraction_h__
#define CenterLineExtraction_h__

#include "VesselTools/IQF_CenterLineExtraction.h"

#include "vtkSmartPointer.h"
#include <vector>

class vtkPoints;
class vtkIdList;

#pragma once
class CenterLineExtraction : public IQF_CenterLineExtraction
{
public:
    CenterLineExtraction();
    ~CenterLineExtraction();
    virtual void ExtractCenterLineNetwork(vtkPolyData* pInput, double* vStartPoint,vtkPolyData* pOutputNetwork,vtkPoints* pOutputEndpoints=NULL,vtkPolyData* pOutputVoronoi=NULL);
    virtual void ExtractCenterLineNetwork(vtkPolyData* pInput, vtkPolyData* pNetWork);
    virtual void ExtractCenterLine(vtkPolyData* pInput, vtkPoints* pSourcePoints, vtkPoints* pTargetPoints,vtkPolyData* pOutputNetwork);
    virtual void ReconstructTubularSurfaceByCenterLine(vtkPolyData* pCenterLine, vtkPolyData* pOutputData);
    virtual void Release() { delete this; }
protected:
    void PrepareModel(vtkPolyData* polyData, vtkPolyData* outputPolyData);
    void DecimateSurface(vtkPolyData* polyData, vtkPolyData* outputPolyData);
    void OpenSurfaceAtPoint(vtkPolyData* polyData, vtkPolyData* outputPolyData, double* seed);
    void ExtractNetwork(vtkPolyData* polyData, vtkPolyData* outputPolyData);
    void ClipSurfaceAtEndPoints(vtkPolyData* networkPolyData, vtkPolyData* surfacePolyData, vtkPolyData* outputPolyData, vtkPoints* endPoints);
    void ComputeCenterlines(vtkPolyData* polyData, vtkIdList* inletSeedIds, vtkIdList* outletSeedIds,
        vtkPolyData* outPolyData, vtkPolyData* outPolyData2);
    static int FoundMinimumIndex(std::vector<double>& v);
};

#endif // CenterLineExtraction_h__
