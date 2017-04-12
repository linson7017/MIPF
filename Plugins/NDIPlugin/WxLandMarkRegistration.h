#pragma once

//QT
#include <QObject>
#include <QList>
#include <QVector3D>
#include <QMatrix4x4>
#include <QtCore/qmath.h>

//VTK
#include <vtkLandmarkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkMath.h>


class  WxLandMarkRegistration : public QObject
{
	Q_OBJECT
public:
    WxLandMarkRegistration(QObject *parent = 0);
    ~WxLandMarkRegistration();

    bool ComputeRegistration(const QList<QVector3D> &image,const QList<QVector3D> &patient);

	vtkMatrix4x4* GetRegistrationVtkMatrix(){return m_RegistrationVtkMatrix;};
	QMatrix4x4 GetRegistrationQtMatrix(){return m_RegistrationQtMatrix;};
	double GetRegistrationRMSError(){return m_RMSError;};

private:
	double distanceTwoPoint(const QVector3D& point1,const QVector3D& point2);

private:
	vtkMatrix4x4*	m_RegistrationVtkMatrix;
	QMatrix4x4      m_RegistrationQtMatrix;
	double				m_RMSError;
};
