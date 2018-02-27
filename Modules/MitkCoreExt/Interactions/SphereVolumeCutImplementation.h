#ifndef SphereVolumeCutImplementation_h__
#define SphereVolumeCutImplementation_h__

#include "CutImplementation.h"
#include "qf_config.h"

class QF_API SphereVolumeCutImplementation :public CutImplementation
{
public:
	SphereVolumeCutImplementation();
	virtual ~SphereVolumeCutImplementation();

    typedef mitk::Message1<vtkObject*> ImplementEventType;
    ImplementEventType CutFinishedEvent;
protected:
	vtkSmartPointer<vtkDataObject> CutImpl(vtkObject* pCutData, mitk::InteractionEvent * interactionEvent);
};

#endif // SphereVolumeCutImplementation_h__
