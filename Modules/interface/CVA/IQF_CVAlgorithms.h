#ifndef IQF_CVAlgorithms_h__
#define IQF_CVAlgorithms_h__
/********************************************************************
	FileName:    IQF_CVAlgorithms
	Author:        Ling Song
	Date:           Month 3 ; Year 2018
	Purpose:	     
*********************************************************************/


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
const char  Object_ID_CVAlgorithms[] = "Object_ID_CVAlgorithms";

class vtkImageData;
class vtkPolyData;
class IQF_CVAlgorithms :public IQF_Object
{
public:
    virtual void GenerateVesselSurface(vtkImageData* pInputImage, vtkPoints* pSeeds, double* dvThreshold, vtkPolyData* pOutput, int iImageType = 0, bool bNeedRecalculateSegmentation = true)=0;
};



#endif // IQF_CVAlgorithms_h__
