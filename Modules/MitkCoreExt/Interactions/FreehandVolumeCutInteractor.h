/********************************************************************
FileName:    FreehandVolumeCutInteractor.h
Author:        Ling Song
Date:           January 2 ; Year 2018
Purpose:
*********************************************************************/

#ifndef FreehandVolumeCutInteractor_h__
#define FreehandVolumeCutInteractor_h__

#include "mitkCommon.h"
#include "mitkDataInteractor.h"
#include <mitkSurface.h>
#include <mitkImage.h>
#include "mitkDataStorage.h"
#include "mitkDataNode.h"

#include "qf_config.h"


class vtkCamera;
class vtkRenderer;

class QF_API FreehandVolumeCutInteractor : public mitk::DataInteractor
{
public:
    mitkClassMacro(FreehandVolumeCutInteractor, mitk::DataInteractor);
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
    
private:
    FreehandVolumeCutInteractor();
    ~FreehandVolumeCutInteractor();
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
    //cut
    void PolygonCut(mitk::Image* data, vtkPoints* points,mitk::InteractionEvent* interactionEvent);
    void ImageStencilCut(mitk::Image* data, vtkPoints* points, mitk::InteractionEvent* interactionEvent);


    void ProjectPointOnPlane(const mitk::Point3D& input, vtkCamera* camera, mitk::Point3D& output);

    void RefreshCurrentImage();
    void RefreshCurve();
    double DistanceBetweenPointAndPoints(mitk::Point3D&, vtkPoints*);

    mitk::Image* m_pImageData;
    mitk::DataStorage* m_pDataStorage;
    vtkRenderer* m_pRenderer;

    mitk::DataNode::Pointer m_pCurveNode;

    vtkSmartPointer<vtkPolyData> m_pCurveData;
    vtkSmartPointer<vtkPoints> m_pCurvePoints;
    vtkSmartPointer<vtkPoints> m_pCurvePointsBeforeModify;

    mitk::Point3D m_LastPoint;
    mitk::Vector3D m_SumVec;
    mitk::Point3D m_originCenter;

    std::vector< vtkSmartPointer<vtkImageData> >  m_vImage;
    std::vector<double> m_vDistanceBetweenPointsAndPoint;
    int m_currentImageIndex;

    bool m_bDrawing;
    bool m_bInitFlag;
    bool m_bInsideOut;
    bool m_bModify;

};


#endif