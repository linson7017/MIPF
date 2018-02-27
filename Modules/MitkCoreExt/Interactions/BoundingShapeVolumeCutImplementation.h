#ifndef BoundingShapeVolumeCutImplementation_h__
#define BoundingShapeVolumeCutImplementation_h__

#include "CutImplementation.h"
#include "qf_config.h"

class QF_API BoundingShapeVolumeCutImplementation :public CutImplementation
{
public:
	BoundingShapeVolumeCutImplementation();
	virtual ~BoundingShapeVolumeCutImplementation();
protected:
	vtkSmartPointer<vtkDataObject> CutImpl(vtkObject* pCutData, mitk::InteractionEvent * interactionEvent);
};

#endif // BoundingShapeVolumeCutImplementation_h__
