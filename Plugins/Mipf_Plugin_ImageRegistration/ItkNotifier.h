#ifndef ItkNotifier_h__
#define ItkNotifier_h__

#include <QObject>
#include <itkMatrix.h>
#include "QfResult.h"

typedef void* ResultImagePointerType;

#pragma once
class ItkNotifier:public QObject
{
    Q_OBJECT
public:
    ItkNotifier();
signals:
    void SignalRegistrationIterationEnd(const itk::Matrix<double, 4, 4>&);
    void SignalRegistrationFinsihed(const QfResult& result);
    void SignalResultImageGenerated(const ResultImagePointerType resultImage);
public:
    void RegistrationIterationEnd(const itk::Matrix<double, 4, 4>& matrix);
    void ResultImageGenerated(const ResultImagePointerType resultImage);
    void RegistrationFinished();
    void SetCurrentRegistrationMatrix(const itk::Matrix<double, 4, 4>& matrix);
    itk::Matrix<double, 4, 4> GetCurrentRegistrationMarix();
    
    QfResult RESULT;
    itk::Matrix<double, 4, 4> m_currentRegistrationMatrix;
};

#endif