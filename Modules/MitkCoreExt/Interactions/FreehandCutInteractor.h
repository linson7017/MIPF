/********************************************************************
FileName:    FreehandCutInteractor.h
Author:        Ling Song
Date:           Month 5 ; Year 2017
Purpose:
*********************************************************************/
#ifndef FreehandCutInteractor_h__
#define FreehandCutInteractor_h__

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
class CutImplementation;

class QF_API FreehandCutInteractor : public mitk::DataInteractor
{
public:
    mitkClassMacro(FreehandCutInteractor, mitk::DataInteractor);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)
    void SetDataNode(mitk::DataNode *dataNode);
    void SetDataStorage(mitk::DataStorage* dataStorage)
    {
        m_pDataStorage = dataStorage;
    }
    void SetRenderer(vtkRenderer* renderer);
    void Start();
    void End();
    void SetInsideOut(bool flag);

    //implementation
    void Undo();
    void Redo();
    void Reset();
    void Finished();
    void SetImplementation(CutImplementation* implementation) { m_implementation = implementation; }
protected:
    FreehandCutInteractor();
    ~FreehandCutInteractor();
    virtual void ConnectActionsAndFunctions();

private:
    virtual void InitMove(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);
    virtual void InitModify(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);
    virtual void Draw(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);
    virtual void Modify(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);
    virtual void Finished(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);
    virtual void Undo(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);
    virtual void Redo(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);

    void Init();

    void ProjectPointOnPlane(const mitk::Point3D& input, vtkCamera* camera, mitk::Point3D& output);
    void RefreshCurve();
    double DistanceBetweenPointAndPoints(mitk::Point3D&, vtkPoints*);

    mitk::DataStorage* m_pDataStorage;
    vtkRenderer* m_pRenderer;

    mitk::DataNode::Pointer m_pCurveNode;

    vtkSmartPointer<vtkPolyData> m_pCurveData;
    vtkSmartPointer<vtkPoints> m_pCurvePoints;
    vtkSmartPointer<vtkPoints> m_pCurvePointsBeforeModify;
    std::vector<double> m_vDistanceBetweenPointsAndPoint;

    mitk::Point3D m_LastPoint;
    mitk::Vector3D m_SumVec;
    mitk::Point3D m_originCenter;   

    bool m_bDrawing;
    bool m_bInitFlag;
    bool m_bModify;

    CutImplementation* m_implementation;
};

#endif // FreehandCutInteractor_h__
