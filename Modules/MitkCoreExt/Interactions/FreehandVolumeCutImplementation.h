/********************************************************************
FileName:    FreehandVolumeCutImplementation.h
Author:        Ling Song
Date:           Month 1 ; Year 2018
Purpose:
*********************************************************************/
#ifndef FreehandVolumeCutImplementation_h__
#define FreehandVolumeCutImplementation_h__

#include "CutImplementation.h"
#include "vtkImageData.h"
#include <stack>


#include "qf_config.h"

class QF_API FreehandVolumeCutImplementation :public CutImplementation
{
public:
    FreehandVolumeCutImplementation();
    virtual ~FreehandVolumeCutImplementation();
    void Cut(vtkObject* pCutData, mitk::InteractionEvent * interactionEvent);
    vtkDataObject* GetDataObject();
    vtkSmartPointer<vtkDataObject> GetCopyOfDataObject();
    void Refresh();
};

#endif // FreehandVolumeCutImplementation_h__
