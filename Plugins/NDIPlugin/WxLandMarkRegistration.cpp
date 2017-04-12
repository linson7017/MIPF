#include "WxLandMarkRegistration.h"

WxLandMarkRegistration::WxLandMarkRegistration(QObject *parent) : QObject(parent)
{
	m_RegistrationVtkMatrix=vtkMatrix4x4::New();
	m_RegistrationVtkMatrix->Identity();
	m_RegistrationQtMatrix.setToIdentity();
	m_RMSError=0.0;
}

WxLandMarkRegistration::~WxLandMarkRegistration()
{

}

bool WxLandMarkRegistration::ComputeRegistration(const QList<QVector3D> &target,const QList<QVector3D> &source)
{
	if (target.size()!= source.size())  return false;

	m_RegistrationVtkMatrix->Identity();
	m_RegistrationQtMatrix.setToIdentity();

	//compute registration matrix(patient-->image)
	vtkPoints* vPatientPoints=vtkPoints::New();
	foreach(QVector3D iterPatient, source)
	{
		vPatientPoints->InsertNextPoint(iterPatient.x(),iterPatient.y(),iterPatient.z());
	}
	vtkPoints* vImagePoints=vtkPoints::New();
	foreach(QVector3D iterImage, target)
	{
		vImagePoints->InsertNextPoint(iterImage.x(),iterImage.y(),iterImage.z());
	}
	vtkLandmarkTransform* landmark=vtkLandmarkTransform::New();
	landmark->SetSourceLandmarks(vPatientPoints);
	landmark->SetTargetLandmarks(vImagePoints);
	landmark->SetModeToRigidBody();
	landmark->Update();
	m_RegistrationVtkMatrix=landmark->GetMatrix();

	//matrix type conversion
	for(int row=0;row<4;row++)
	{
		for(int column=0;column<4;column++)
		{
			m_RegistrationQtMatrix(row,column)=m_RegistrationVtkMatrix->GetElement(row,column);
		}
	}

	//compute RMS error
	double sum=0;
	int pointnum= target.size();
	for(int i=0;i<pointnum;i++)
	{
		sum+=qPow(distanceTwoPoint(target[i],m_RegistrationQtMatrix*source[i]),2);
	}
	m_RMSError=qSqrt(sum/pointnum);

	return true;
}

double WxLandMarkRegistration::distanceTwoPoint(const QVector3D& point1,const QVector3D& point2)
{
	return (point1-point2).length();
}