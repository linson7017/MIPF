#ifndef CQF_SurfaceTool_h__
#define CQF_SurfaceTool_h__

#include "MitkSegmentation/IQF_MitkSurfaceTool.h"

#pragma once
class CQF_SurfaceTool:public IQF_MitkSurfaceTool
{
public:
    CQF_SurfaceTool();
    ~CQF_SurfaceTool();

    virtual bool ExtractSurface(mitk::Image* pImage, vtkPolyData* pOutput, int iSmooth = 0, bool bLargestConnected = false);
    virtual bool ExtractSurface(mitk::DataNode* pNode, vtkPolyData* pOutput, int iSmooth = 0, bool bLargestConnected = false);
    virtual void SmoothSurface(vtkPolyData* pInput, vtkPolyData* pOutput, int iSmooth=15, bool bLargestConnected = false);
    virtual void SmoothTubeSurface(vtkPolyData* pInput, vtkPolyData* pOutput, int iSmooth = 30, bool bLargestConnected = false);
    virtual void SmoothTubeSurfaceUsingLaplace(vtkPolyData* pInput, vtkPolyData* pOutput, int iSmooth = 15, bool bLargestConnected = false);
    virtual void ConvertSurfaceToImage(mitk::Surface* surface, mitk::Image* referenceImage, mitk::Image* output);

};

#endif // CQF_SurfaceExtract_h__
