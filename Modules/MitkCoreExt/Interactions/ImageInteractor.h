/********************************************************************
	FileName:    ImageInteractor.h
	Author:        Ling Song
	Date:           Month 10 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef ImageInteractor_h__
#define ImageInteractor_h__

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkSmartPointer.h"
#include "mitkCommon.h"
#include "mitkDataInteractor.h"
#include <mitkImage.h>

#include <vtkSmartPointer.h>
#include <vtkVector.h>

#include "qf_config.h"


class QF_API ImageInteractor : public mitk::DataInteractor
{
public:
    mitkClassMacro(ImageInteractor, mitk::DataInteractor);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)
    void SetDataNode(mitk::DataNode *dataNode);
    void Reset();
    vtkMatrix4x4* GetTransform();
    void SetTransform(vtkMatrix4x4* data);

    //event
    typedef mitk::Message1<vtkMatrix4x4*> MatrixChangeEventType;
    MatrixChangeEventType TransformChangedEvent;
protected:
    ImageInteractor();
    ~ImageInteractor();
    virtual void ConnectActionsAndFunctions();
    //  virtual void ConfigurationChanged();
private:
    virtual void SelectImage(mitk::StateMachineAction *, mitk::InteractionEvent *);
    virtual void InitMove(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);
    virtual void MoveImage(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);
    virtual void RotateImage(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);
    virtual void FinishMove(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);

    void Init();
    void RefreshDataGeometry();
    void Translate(const vtkVector3d& translate);
    void Rotate(double angle, const vtkVector3d& normal);


    mitk::Image::Pointer m_imageData;

    bool m_bDragging;
    mitk::Point3D m_LastPoint;
    mitk::Vector3D m_SumVec;

    vtkSmartPointer<vtkMatrix4x4> m_originMatrix;
    mitk::Point3D m_originCenter;
    vtkSmartPointer<vtkTransform> m_transform;
    bool m_bInitFlag;
};

#endif // ImageInteractor_h__