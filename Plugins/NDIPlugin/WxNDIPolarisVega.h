#pragma once

//QT
#include <QObject>
#include <QFile>
#include <QMatrix4x4>
#include <QQuaternion>
#include <qmath.h>

//VTK
#include <vtkMatrix4x4.h>

//WX
#include "WxNDISystemCRC.h"
#include "WxNetworkComm.h"

class  WxNDIPolarisVega : public QObject
{
    Q_OBJECT
	
	/* POSITION 3D Structure */
	typedef struct Position3dStruct
	{
		float   x;
		float   y;
		float   z;
	} Position3d;

	/* QUATERNION Structure */
	typedef struct QuatRotationStruct
	{
		float   q0;
		float   qx;
		float   qy;
		float   qz;
	} QuatRotation;

	/* QuatTansformation Structure */
	typedef struct QuatTransformationStruct
	{
		QuatRotation   rotation;
		Position3d       translation;
	} QuatTransformation;

	/* 4x4 Matrix Structure*/
	typedef float RotationMatrix[4][4];

	//tracking tool information//
	typedef struct ToolInformationStruct
	{
		QString						romfilename;	
		QString						trackpriority;
		QByteArray				handleport;
		QuatTransformation transform;
	} ToolInformation;

	/*type conversion */
	#define MEMORY_TYPE_CAST(targetType,value)  *((targetType *)&value)

public:
    explicit WxNDIPolarisVega(QObject *parent = 0);
    ~WxNDIPolarisVega();

    bool InitalizePolarisVega(QString, int);
    void AddTool(const QString &,QString);
    void StartTrack();
    void StopTrack();
	void GetQMatrix4x4(int,QMatrix4x4& matrix);
    void GetCurrentTips(int port, double& x, double& y, double& z);

private:
     void INIT_Command();       //Initializes the system
     void RESET_Command();  //Resets the system
     void TSTART_Command(); //Starts Tracking mode
     void TSTOP_Command();  //Stops tracking mode
     void BEEP_Command();    //Sounds the system beeper
	 void APIREV_Command(); //Returns the API revision number that functions with your system
	 void VER_Command(QString); //Returns the firmware revision number of critical processors installed in the system
	 void PHSR_Command();   //Returns the number of assigned port handles and the port status for each one
	 void SFLIST_Command(); //Returns information about the supported features of the system

     QByteArray PHRQ_Command();                                             //Assigns a port handle to a tool
     void PVWR_Command(const QByteArray,const QString &);   //Assigns a tool definition file to a wireless tool
     void PINIT_Command(const QByteArray);                              //Initializes a port handle
	 void PHINF_Command(const QByteArray);								//Returns port handle status, information about the tool associated with the port handle, and the physical location of a port handle
     void PENA_Command(const QByteArray,QString);                //Enables the reporting of transformations for a particular port handle
     void PHF_Command(const QByteArray);                               //Releases system resources from an unused port handle

     void BX_Command(bool isParseData=false);

private:
     QByteArray ReadRomFile(const QString &);

     void ParseHandles(QDataStream *ds);
     void ParseOneHandle(QDataStream *ds);
     QuatTransformation ParseReplyOption0001(QDataStream *ds);

     void QuaternionToRotationMatrix(QuatTransformation*,RotationMatrix);

private:
	QMap<QByteArray,QuatTransformation> m_QuatTransformations;
	QList<QByteArray> m_HandlePortName;

    WxNetworkComm *m_ntcomm;
};

