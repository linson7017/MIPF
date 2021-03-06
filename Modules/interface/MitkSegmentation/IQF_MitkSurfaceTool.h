#ifndef IQF_MitkSurfaceTool_h__
#define IQF_MitkSurfaceTool_h__

const char QF_MitkSurface_Tool[] = "QF_MitkSurface_Tool";

namespace mitk
{
    class Image;
    class DataNode;
    class Surface;
}

class vtkPolyData;


class IQF_MitkSurfaceTool
{
public:
    virtual bool ExtractSurface(mitk::Image* pImage, vtkPolyData* pOutput, int iSmooth = 0, bool bLargestConnected = false) = 0;
    virtual bool ExtractSurface(mitk::DataNode* pNode, vtkPolyData* pOutput, int iSmooth = 0, bool bLargestConnected = false) = 0;
    virtual void SmoothSurface(vtkPolyData* pInput, vtkPolyData* pOutput, int iSmooth = 30, bool bLargestConnected = false) = 0;
    virtual void SmoothTubeSurface(vtkPolyData* pInput, vtkPolyData* pOutput, int iSmooth = 15, bool bLargestConnected = false) = 0;
    virtual void ConvertSurfaceToImage(mitk::Surface* surface, mitk::Image* referenceImage, mitk::Image* output) = 0;
    
    /************************************************************************/
    /* 简化面片
    dRate: 0~1，简化程度，越高简化越多
    mode: 0-DecimatePro  1-QuadricDecimation
    */
    /************************************************************************/
    virtual void SimplifySurfaceMesh(vtkPolyData* pInput, vtkPolyData* pOutput, double dRate, int mode = 0) = 0;
};


#endif // IQF_MitkSurfaceTool_h__
