#ifndef RegistrationWorkStation_h__
#define RegistrationWorkStation_h__

#include <QObject>
#include "ITKImageTypeDef.h"
#include <QMatrix4x4>
#include "Registration_Base.h"

class ItkNotifier;

#pragma once
class RegistrationWorkStation:public QObject
{
    Q_OBJECT
public:
    enum RegistrationType
    {
        MI,   //Mutual Information
        MMI //Multi Resolution Mutual Information
    };
    RegistrationWorkStation();
    ~RegistrationWorkStation();
    void SetUseMultiResolution(bool useMultiResolution) { m_useMultiResolution = useMultiResolution; }
    bool GetUseMultiResolution() { return m_useMultiResolution; }
    void SetRegistrationType(RegistrationType registrationType) 
    {
        m_registrationType = registrationType;
    }
    RegistrationType GetRegistrationType() { return m_registrationType; }
public slots:
    void SlotDoRegistration(const Float3DImagePointerType fixedImage, const Float3DImagePointerType movingImage, QMatrix4x4 initTransformMatrix);
    void SlotRegistrationIterationEnd(const itk::Matrix<double, 4, 4>& result);
    void SlotRegistrationFinished();
    void SlotStopRegistration();
signals:
    void SignalWorkFinished();
    void SignalRegistrationIterationEnd(const itk::Matrix<double, 4, 4>&);
    void SignalRegistrationFinished();
private:
    ItkNotifier* m_pItkNotifier;
    RegistrationType m_registrationType;
    bool m_useMultiResolution;

    RegistrationBase<Float3DImageType, Float3DImageType>* m_pCurrentRegistration;
};

#endif // RegistrationWorkStation_h__
