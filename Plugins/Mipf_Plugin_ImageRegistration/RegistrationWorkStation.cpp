#include "RegistrationWorkStation.h"
#include "Registration_MI.h"
#include "Registration_MR.h"
#include "Registration_TT.h"
#include "Registration_SFD.h"

#include "ItkNotifier.h"
#include <vtkMatrix4x4.h>
#include <itkImageRegistrationMethod.h>
#include <itkMultiResolutionImageRegistrationMethod.h>

RegistrationWorkStation::RegistrationWorkStation():m_registrationType(MI), m_pCurrentRegistration(NULL), m_stopped(false), m_beginWithTranslation(false)
{
    m_pItkNotifier = new ItkNotifier;
    connect(m_pItkNotifier, &ItkNotifier::SignalRegistrationIterationEnd, this, &RegistrationWorkStation::SlotRegistrationIterationEnd);
    connect(m_pItkNotifier, &ItkNotifier::SignalRegistrationFinsihed, this, &RegistrationWorkStation::SlotRegistrationFinished);
    connect(m_pItkNotifier, &ItkNotifier::SignalResultImageGenerated, this, &RegistrationWorkStation::SlotResultImageGenerated);
}


RegistrationWorkStation::~RegistrationWorkStation()
{
}

void RegistrationWorkStation::SlotDoRegistration(const Float3DImagePointerType fixedImage, const Float3DImagePointerType movingImage, vtkMatrix4x4* initTransformMatrix)
{
    itk::Matrix<double, 4, 4> itkMatirx;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0;j < 4;j++)
        {
            itkMatirx[i][j] = initTransformMatrix->GetElement(i,j);
        }
    }
    switch (m_registrationType)
    {
    case MI:
    {
        m_pItkNotifier->RESULT.Init();
        if (m_beginWithTranslation)
        {
            RegistrationTT<Float3DImageType, Float3DImageType> rtt;
            m_pCurrentRegistration = &rtt;
            rtt.SetQuitAfterFinished(false);
            rtt.SetNotifier(m_pItkNotifier);
            rtt.SetUseMultiResolution(m_useMultiResolution);
            rtt.Start(fixedImage.GetPointer(), movingImage.GetPointer(), NULL, itkMatirx);
            if (m_stopped)
            {
                m_stopped = false;
                return;
            }
        }
        if (m_onlyTranslation)
        {
            return;
        }

        if (m_useMultiResolution)
        {
            m_pCurrentRegistration = new RegistrationMR<Float3DImageType, Float3DImageType>();
        }
        else
        {
            m_pCurrentRegistration = new RegistrationMI<Float3DImageType, Float3DImageType>();
        }
        /*if (!initTransformMatrix.isIdentity())
        {
            m_pCurrentRegistration->SetBeginStepLength(0.2);
        }*/
        m_pCurrentRegistration->SetNotifier(m_pItkNotifier);
        m_pCurrentRegistration->SetUseMultiResolution(m_useMultiResolution);
        m_pCurrentRegistration->Start(fixedImage.GetPointer(), movingImage.GetPointer(), NULL, 
            m_beginWithTranslation?m_pItkNotifier->GetCurrentRegistrationMarix(): itkMatirx);
        delete m_pCurrentRegistration;
        m_pCurrentRegistration = NULL;
    }
        break;
    case SFD:
    {
        itk::Matrix<double, 4, 4> itkMatirx;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0;j < 4;j++)
            {
                itkMatirx[i][j] = initTransformMatrix->GetElement(i, j);
            }
        }
        m_pItkNotifier->RESULT.Init();
        RegistrationSFD<Float3DImageType, Float3DImageType> rsfd;
        m_pCurrentRegistration = &rsfd;
        rsfd.SetNotifier(m_pItkNotifier);
        rsfd.SetUseMultiResolution(m_useMultiResolution);
        rsfd.Start(fixedImage.GetPointer(), movingImage.GetPointer(), NULL, itkMatirx);
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

void RegistrationWorkStation::SlotRegistrationFinished(const QfResult& result)
{
    emit SignalRegistrationFinished(result);
}

void RegistrationWorkStation::SlotStopRegistration()
{
    if (m_pCurrentRegistration)
    {
        m_pCurrentRegistration->Stop();
    }
    m_stopped = true;
}

void RegistrationWorkStation::SlotResultImageGenerated(const ResultImagePointerType resultImage)
{
    Float3DImagePointerType image = Float3DImageType::New();
    image->Graft((Float3DImageType*)resultImage);
    emit SignalReslutImageGenerated(image);
}