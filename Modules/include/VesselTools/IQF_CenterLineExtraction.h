#ifndef IQF_CenterLineExtraction_h__
#define IQF_CenterLineExtraction_h__

#include "iconfig.h"
#include "IQF_Object.h"

class vtkPolyData;
class vtkPoints;

/************************************************************************/
/* Object ID:  CenterLineExtraction
* Get and use object by the code below :
    IQF_ObjectFactory* pObjectFactory = (IQF_ObjectFactory*)m_pMain->GetInterfacePtr(QF_Core_ObjectFactory);
    if (pObjectFactory)
    {
        IQF_CenterLineExtraction* pCenterLine = (IQF_CenterLineExtraction*)pObjectFactory->CreateObject("CenterLineExtraction");
        pCenterLine->SegmentVessel(...);
    ...
    }
*/
/************************************************************************/
const char  Object_ID_CenterLineExtraction[] = "Object_ID_CenterLineExtraction";
class IQF_CenterLineExtraction :public IQF_Object
{
    BUILD_INTERFACE(IQF_CenterLineExtraction)
public:
    virtual void ExtractCenterLineNetwork(vtkPolyData* pInput, double* vStartPoint, vtkPolyData* pOutputNetwork, 
        vtkPoints* pOutputEndpoints = nullptr, vtkPolyData* pOutputVoronoi = nullptr) = 0;
    virtual void ReconstructTubularSurfaceByCenterLine(vtkPolyData* pCenterLine, vtkPolyData* pOutputData) = 0;
};



#endif // IQF_CenterLineExtraction_h__
