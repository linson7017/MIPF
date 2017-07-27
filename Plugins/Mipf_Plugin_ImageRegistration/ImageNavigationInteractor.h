/********************************************************************
	FileName:    ImageNavigationInteractor.h
	Author:        Ling Song
	Date:           Month 5 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef ImageNavigationInteractor_h__
#define ImageNavigationInteractor_h__

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkSmartPointer.h"
#include "mitkCommon.h"
#include "mitkDataInteractor.h"
#include <mitkImage.h>
#include <QMatrix4x4>
#pragma once


class ImageNavigationInteractor : public QObject, public mitk::DataInteractor
{
    Q_OBJECT
public:
    mitkClassMacro(ImageNavigationInteractor, mitk::DataInteractor);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)
    void SetDataNode(mitk::DataNode *dataNode);

    void SetTransformMatrix(const QMatrix4x4& matrix);
    QMatrix4x4 GetTransformMatrix();

protected:
     ImageNavigationInteractor();
    ~ImageNavigationInteractor();
    virtual void ConnectActionsAndFunctions();
  //  virtual void ConfigurationChanged();
signals:
    void SignalTranslateImage(const QVector3D& vector);
    void SignalRotateImage(double angle, const QVector3D& normal);
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

#endif // ImageNavigationInteractor_h__
