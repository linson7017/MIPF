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
#include <QMatrix4x4>

#include "qf_config.h"


class QF_API ImageInteractor : public mitk::DataInteractor
{
public:
    mitkClassMacro(ImageInteractor, mitk::DataInteractor);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)
        void SetDataNode(mitk::DataNode *dataNode);

    void SetTransformMatrix(const QMatrix4x4& matrix);
    QMatrix4x4 GetTransformMatrix();
    void Reset();
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
    void Translate(const QVector3D& translate);
    void Rotate(double angle, const QVector3D& normal);


    mitk::Image::Pointer m_imageData;

    bool m_bDragging;
    mitk::Point3D m_LastPoint;
    mitk::Vector3D m_SumVec;

    vtkMatrix4x4* m_originMatrix;
    mitk::Point3D m_originCenter;
    QMatrix4x4 m_transformMatrix;
    bool m_bInitFlag;
};

#endif // ImageInteractor_h__