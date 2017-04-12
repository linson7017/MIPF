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

class  WxPointSetAutomaticPairing : public QObject
{
	Q_OBJECT

public:
    WxPointSetAutomaticPairing();
    ~WxPointSetAutomaticPairing();

    bool PointSetAutomaticPariring(QList<QVector3D> &pointset1,QList<QVector3D> &pointset2);
	QList<QVector3D> GetPointSet1(){return m_AutoPairedPointSet1;};
	QList<QVector3D> GetPointSet2(){return m_AutoPairedPointSet2;};

private:
	double distanceTwoPoint(const QVector3D& point1,const QVector3D& point2);
	double coefficientTwoListPoint(const QList<double>& listpoint1,const QList<double>& listpoint2);

private:
	QList<QList<double>> m_PointSetDistance1; 
	QList<QList<double>> m_PointSetDistance2; 
    QList<QList<double>> m_PointSetCoefficient; 

	QList<QVector3D> m_AutoPairedPointSet1;
	QList<QVector3D> m_AutoPairedPointSet2;
};
