#ifndef GeometryInteractor_h__
#define GeometryInteractor_h__

#include "mitkAffineBaseDataInteractor3D.h"
#include "qf_config.h"

#pragma once
class QF_API GeometryInteractor  : public mitk::AffineBaseDataInteractor3D
{
public:
    mitkClassMacro(GeometryInteractor, AffineBaseDataInteractor3D);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self);

protected:
    virtual void ScaleObject(mitk::StateMachineAction *, mitk::InteractionEvent *) override;
    virtual void RotateObject(mitk::StateMachineAction *, mitk::InteractionEvent *) override;
private:
    GeometryInteractor();
    ~GeometryInteractor();
};

#endif // GeometryInteractor_h__
