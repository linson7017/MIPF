#ifndef ItkNotifier_h__
#define ItkNotifier_h__

#include <QObject>
#include <itkMatrix.h>

#pragma once
class ItkNotifier:public QObject
{
    Q_OBJECT
public:
    ItkNotifier();
signals:
    void SignalRegistrationIterationEnd(const itk::Matrix<double, 4, 4>&);
    void SignalRegistrationFinsihed();
public:
    void RegistrationIterationEnd(const itk::Matrix<double, 4, 4>& matrix);
    void RegistrationFinished();
};

#endif