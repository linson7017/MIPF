#ifndef IQF_VesselSegmentationTool_h__
#define IQF_VesselSegmentationTool_h__

#include "IQF_Object.h"

class vtkImageData;
class vtkIdList;
class vtkPolyData;
/************************************************************************/
/* Object ID:  VesselSegmentation
* Get and use object by the code below :
    IQF_ObjectFactory* pObjectFactory = (IQF_ObjectFactory*)m_pMain->GetInterfacePtr(QF_Core_ObjectFactory);
    if (pObjectFactory)
    {
    IQF_VesselSegmentationTool* pVesselSegTool = (IQF_VesselSegmentationTool*)pObjectFactory->CreateObject("VesselSegmentation");
    pVesselSegTool->SegmentVessel(...);
    ...
    }
*/
/************************************************************************/
const char Object_ID_VesselSegmentationTool[] = "Object_ID_VesselSegmentationTool";
class IQF_VesselSegmentationTool  : public IQF_Object
{
public:
    virtual void SegmentVessel(vtkImageData* pInput, vtkPolyData* pOutput,
        double dLowerThreshold, double dHigherThreshold, vtkIdList*  pSourceSeedIds, vtkIdList* pTargetSeedIds, int iInitializeMode = 0) = 0;

    virtual void SegmentVessel(vtkImageData* pInput, vtkImageData* pOutput,
        double dLowerThreshold, double dHigherThreshold, vtkIdList*  pSourceSeedIds, vtkIdList* pTargetSeedIds, int iInitializeMode = 0) = 0;

    virtual void GenerateVesselSurface(vtkImageData* pInput, vtkPolyData* pOutput, double dThreshold) = 0;

    virtual void CapSurface(vtkPolyData* input, vtkPolyData*output) = 0;
};


#endif // IQF_VesselSegmentationTool_h__
