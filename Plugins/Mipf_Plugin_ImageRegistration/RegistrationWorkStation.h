#ifndef RegistrationWorkStation_h__
#define RegistrationWorkStation_h__

#include <QObject>
#include "ITKImageTypeDef.h"
#include <QMatrix4x4>
#include "Registration_Base.h"
#include "QfResult.h"
#include "ItkNotifier.h"

#pragma once
class RegistrationWorkStation:public QObject
{
    Q_OBJECT
public:
    enum RegistrationType
    {
        MI,   //Mutual Information
        MMI, //Multi Resolution Mutual Information
        SFD 
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
    void SetBeginWithTranslation(bool beginWithTranslation) { m_beginWithTranslation = beginWithTranslation; }
    bool GetBeginWithTranslation() { return m_beginWithTranslation; }
public slots:
    void SlotDoRegistration(const Float3DImagePointerType fixedImage, const Float3DImagePointerType movingImage, QMatrix4x4 initTransformMatrix);
    void SlotRegistrationIterationEnd(const itk::Matrix<double, 4, 4>& result);
    void SlotRegistrationFinished(const QfResult& result);
    void SlotStopRegistration();
    void SlotResultImageGenerated(const ResultImagePointerType resultImage);
signals:
    void SignalWorkFinished();
    void SignalRegistrationIterationEnd(const itk::Matrix<double, 4, 4>&);
    void SignalRegistrationFinished(const QfResult&);
    void SignalReslutImageGenerated(const Float3DImagePointerType resultImage);
private:
    ItkNotifier* m_pItkNotifier;
    RegistrationType m_registrationType;
    bool m_useMultiResolution;
    bool m_stopped;
    bool m_beginWithTranslation;

    RegistrationBase<Float3DImageType, Float3DImageType>* m_pCurrentRegistration;
};

#endif // RegistrationWorkStation_h__
