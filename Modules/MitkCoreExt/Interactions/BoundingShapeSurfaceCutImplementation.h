#ifndef BoundingShapeSurfaceCutImplementation_h__
#define BoundingShapeSurfaceCutImplementation_h__

#include "CutImplementation.h"
#include "vtkPolyData.h"
#include <stack>

#include "qf_config.h"

class QF_API BoundingShapeSurfaceCutImplementation :public CutImplementation
{
public:
	BoundingShapeSurfaceCutImplementation();
	virtual ~BoundingShapeSurfaceCutImplementation();
protected:
	virtual vtkSmartPointer<vtkDataObject> CutImpl(vtkObject* pCutData, mitk::InteractionEvent * interactionEvent);
};

#endif // BoundingShapeSurfaceCutImplementation_h__
