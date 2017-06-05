#include "ItkNotifier.h"
#include <QCoreApplication>

ItkNotifier::ItkNotifier()
{
    m_currentRegistrationMatrix.SetIdentity();
}

void ItkNotifier::RegistrationIterationEnd(const itk::Matrix<double, 4, 4>& matrix)
{
    emit SignalRegistrationIterationEnd(matrix);
    SetCurrentRegistrationMatrix(matrix);
    QCoreApplication::processEvents();
}

void ItkNotifier::RegistrationFinished()
{
    emit SignalRegistrationFinsihed(RESULT);
    QCoreApplication::processEvents();
}

void ItkNotifier::SetCurrentRegistrationMatrix(const itk::Matrix<double, 4, 4>& matrix)
{
    m_currentRegistrationMatrix = matrix;
}

itk::Matrix<double, 4, 4> ItkNotifier::GetCurrentRegistrationMarix()
{
    return m_currentRegistrationMatrix;
}

void ItkNotifier::ResultImageGenerated(const ResultImagePointerType resultImage)
{
    emit SignalResultImageGenerated(resultImage);
}
