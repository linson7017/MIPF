#include "RegistrationWorkStation.h"
#include "Registration_MI.h"
#include "ItkNotifier.h"
#include <QMatrix4x4>
#include <itkImageRegistrationMethod.h>
#include <itkMultiResolutionImageRegistrationMethod.h>

RegistrationWorkStation::RegistrationWorkStation():m_registrationType(MI), m_pCurrentRegistration(NULL)
{
    m_pItkNotifier = new ItkNotifier;
    connect(m_pItkNotifier, &ItkNotifier::SignalRegistrationIterationEnd, this, &RegistrationWorkStation::SlotRegistrationIterationEnd);
    connect(m_pItkNotifier, &ItkNotifier::SignalRegistrationFinsihed, this, &RegistrationWorkStation::SlotRegistrationFinished);
}


RegistrationWorkStation::~RegistrationWorkStation()
{
}

void RegistrationWorkStation::SlotDoRegistration(const Float3DImagePointerType fixedImage, const Float3DImagePointerType movingImage, QMatrix4x4 initTransformMatrix)
{
    itk::Matrix<double, 4, 4> itkMatirx;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0;j < 4;j++)
        {
            itkMatirx[i][j] = initTransformMatrix(i,j);
        }
    }
    switch (m_registrationType)
    {
    case MI:
    {
            RegistrationMI<Float3DImageType, Float3DImageType> rmi;
            m_pCurrentRegistration = &rmi;
            rmi.SetNotifier(m_pItkNotifier);
            rmi.SetUseMultiResolution(m_useMultiResolution);
            rmi.Start(fixedImage.GetPointer(), movingImage.GetPointer(), NULL, itkMatirx);
    }
        break;
    default:
        break;
    }
    
}

void RegistrationWorkStation::SlotRegistrationIterationEnd(const itk::Matrix<double, 4, 4>& result)
{
    emit SignalRegistrationIterationEnd(result);
}

void RegistrationWorkStation::SlotRegistrationFinished()
{
    emit SignalRegistrationFinished();
}

void RegistrationWorkStation::SlotStopRegistration()
{
    if (m_pCurrentRegistration)
    {
        m_pCurrentRegistration->Stop();
    }
}
