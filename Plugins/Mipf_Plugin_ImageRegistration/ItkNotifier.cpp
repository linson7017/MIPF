#include "ItkNotifier.h"
#include <QCoreApplication>

ItkNotifier::ItkNotifier()
{
}

void ItkNotifier::RegistrationIterationEnd(const itk::Matrix<double, 4, 4>& matrix)
{
    emit SignalRegistrationIterationEnd(matrix);
    QCoreApplication::processEvents();
}

void ItkNotifier::RegistrationFinished()
{
    emit SignalRegistrationFinsihed();
    QCoreApplication::processEvents();
}
