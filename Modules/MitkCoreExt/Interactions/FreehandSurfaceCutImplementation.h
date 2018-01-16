/********************************************************************                                                                                                     FreehandSurfaceCutImplementation
	FileName:    FreehandSurfaceCutImplementation.h
	Author:        Ling Song
	Date:           Month 1 ; Year 2018
	Purpose:	     
*********************************************************************/
#ifndef FreehandSurfaceCutImplementation_h__
#define FreehandSurfaceCutImplementation_h__

#include "CutImplementation.h"
#include "vtkPolyData.h"
#include <stack>

#include "qf_config.h"

class QF_API FreehandSurfaceCutImplementation :public CutImplementation
{
public:
    FreehandSurfaceCutImplementation();
    virtual ~FreehandSurfaceCutImplementation();
protected:
    virtual vtkSmartPointer<vtkDataObject> CutImpl(vtkObject* pCutData, mitk::InteractionEvent * interactionEvent);
    vtkDataObject* GetDataObject();
    vtkSmartPointer<vtkDataObject> GetCopyOfDataObject();
    void Refresh();
};

#endif // FreehandSurfaceCutImplementation_h__
