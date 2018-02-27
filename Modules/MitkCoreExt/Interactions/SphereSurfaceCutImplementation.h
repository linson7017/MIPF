#ifndef SphereSurfaceCutImplementation_h__
#define SphereSurfaceCutImplementation_h__

#include "CutImplementation.h"
#include "vtkPolyData.h"
#include <stack>

#include "qf_config.h"

class QF_API SphereSurfaceCutImplementation :public CutImplementation
{
public:
	SphereSurfaceCutImplementation();
	virtual ~SphereSurfaceCutImplementation();
protected:
	virtual vtkSmartPointer<vtkDataObject> CutImpl(vtkObject* pCutData, mitk::InteractionEvent * interactionEvent);
};

#endif // SphereSurfaceCutImplementation_h__
