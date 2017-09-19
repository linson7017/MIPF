/********************************************************************
	FileName:    FreehandSurfaceCutInteractor.h
	Author:        Ling Song
	Date:           Month 5 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef FreehandSurfaceCutInteractor_h__
#define FreehandSurfaceCutInteractor_h__

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkSmartPointer.h"
#include "mitkCommon.h"
#include "mitkDataInteractor.h"
#include <mitkSurface.h>
#include "mitkDataStorage.h"
#include "mitkDataNode.h"

//vtk
#include <vtkPoints.h>
#include <vtkPolyData.h>

#include "qf_config.h"

class vtkCamera;
class vtkRenderer;

#pragma once

class QF_API FreehandSurfaceCutInteractor : public mitk::DataInteractor
{
public:
    mitkClassMacro(FreehandSurfaceCutInteractor, mitk::DataInteractor);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)
    void SetDataNode(mitk::DataNode *dataNode);
    void SetDataStorage(mitk::DataStorage* dataStorage)
    {
        m_pDataStorage = dataStorage;
    }
    void SetRenderer(vtkRenderer* renderer);

    void SetInsideOut(bool flag);
    void Undo();
    void Redo();
    void Reset();
    void Finished();

    void Start();
    void End();

protected:
     FreehandSurfaceCutInteractor();
    ~FreehandSurfaceCutInteractor();
    virtual void ConnectActionsAndFunctions();

private:
    virtual void InitMove(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);
    virtual void Draw(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);
    virtual void Modify(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);
    virtual void Finished(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);
    virtual void Undo(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);
    virtual void Redo(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);


    void Init();

    void ProjectPointOnPlane(const mitk::Point3D& input, vtkCamera* camera,mitk::Point3D& output);

    void RefreshCurrentSurface();
    void RefreshCurve();


    mitk::Surface* m_pSurfaceData;
    mitk::DataStorage* m_pDataStorage;
    vtkRenderer* m_pRenderer;

    mitk::DataNode::Pointer m_pCurveNode;

    vtkSmartPointer<vtkPolyData> m_pCurveData;
    vtkSmartPointer<vtkPoints> m_pCurvePoints;

  
    mitk::Point3D m_LastPoint;
    mitk::Vector3D m_SumVec;
    mitk::Point3D m_originCenter;

    std::vector< vtkSmartPointer<vtkPolyData> >  m_vSurface;
    int m_currentSurfaceIndex;

    bool m_bDrawing;
    bool m_bInitFlag;
    bool m_bInsideOut;
};

#endif // FreehandSurfaceCutInteractor_h__
