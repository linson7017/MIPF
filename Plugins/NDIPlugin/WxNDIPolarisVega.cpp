#include "WxNDIPolarisVega.h"

WxNDIPolarisVega::WxNDIPolarisVega(QObject *parent) : QObject(parent),
                                                                            m_ntcomm(NULL)
{
}

WxNDIPolarisVega::~WxNDIPolarisVega()
{
}

bool WxNDIPolarisVega::InitalizePolarisVega(QString strIP, int nPort)
{
	m_ntcomm=new WxNetworkComm;
    if(m_ntcomm->ConnectDevice(strIP,nPort))
    {
        qDebug() << "Connect to " << strIP<<":"<< nPort << " successfully!";
		m_QuatTransformations.clear();
		m_HandlePortName.clear();
		
		RESET_Command();
		
		APIREV_Command();
		VER_Command(QString("4"));
		VER_Command(QString("5"));
        INIT_Command();
		PHSR_Command();
		VER_Command(QString("4"));
		SFLIST_Command();

        TSTART_Command();
        BX_Command();
        TSTOP_Command();
        
		return true;
    }
	else
	{
        qDebug() << "Connect to " << strIP << ":" << nPort << " failed!";
		if(m_ntcomm)
			delete m_ntcomm;

		return false;
	}
}

void WxNDIPolarisVega::AddTool(const QString &romFileName,QString tooltype)
{
    QByteArray handleport=PHRQ_Command();
	m_HandlePortName.append(handleport);

    PVWR_Command(handleport,romFileName);
    PINIT_Command(handleport);
	PHINF_Command(handleport);

	TSTART_Command();
	BX_Command();
	TSTOP_Command();
    
	PENA_Command(handleport,tooltype);
}

void WxNDIPolarisVega::StartTrack()
{
    TSTART_Command();
}

void WxNDIPolarisVega::StopTrack()
{
    TSTOP_Command();
	//for(int i=0;i<m_HandlePortName.size();i++)
	//{
	//	PHF_Command(m_HandlePortName.at(i));
	//}
	BEEP_Command();

	m_ntcomm->DisconnectDevice();

	m_HandlePortName.clear();
	m_QuatTransformations.clear();

	if(m_ntcomm)
		delete m_ntcomm;
}

void  WxNDIPolarisVega::GetQMatrix4x4(int port,QMatrix4x4& matrix4x4)
{
	BX_Command(true);
	
	QuatTransformation tempqt;
	if(port<m_HandlePortName.size())
	{
		QByteArray handleport=m_HandlePortName.at(port);
		tempqt=m_QuatTransformations[handleport];
		if(tempqt.rotation.q0==0 && tempqt.rotation.qx==0 && tempqt.rotation.qy==0 &&
			tempqt.rotation.qz==0 && tempqt.translation.x==0 && tempqt.translation.y==0 &&
			tempqt.translation.z==0)
		{
			matrix4x4.setToIdentity();
		}
		else
		{
			RotationMatrix matrix;
			QuaternionToRotationMatrix(&tempqt,matrix);

			for(int row=0;row<4;row++)
			{
				for(int column=0;column<4;column++)
				{
					matrix4x4(row,column)=matrix[row][column];
				}
			}

		}
	}
	else
	{
		matrix4x4.setToIdentity();
	}
}

void WxNDIPolarisVega::GetCurrentTips(int port, double& x, double& y, double& z)
{
    BX_Command(true);
    QByteArray handleport = m_HandlePortName.at(port);
    x = m_QuatTransformations[handleport].translation.x;
    y = m_QuatTransformations[handleport].translation.y;
    z = m_QuatTransformations[handleport].translation.z;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WxNDIPolarisVega::INIT_Command()
{
    QByteArray command;
    command.append(QString("INIT"));
    command.append(QString(" "));
    command.append(QString("\r"));
    qDebug()<<command;

    QByteArray reply;
    reply=m_ntcomm->SendCommandRecieveReply(command);
    qDebug()<<reply;

    bool valid=WxNDISystemCRC::GetInstance()->SystemCheckCRC(reply.data());
    if(valid)
    {
        if(reply.startsWith("OKAY"))
        {
            qDebug()<<"INIT OK!";
        }
        else
        {
            qDebug()<<"-----INIT ERROR"<<reply.mid(5,2).toInt()<<"-----";
        }
    }
}

void WxNDIPolarisVega::RESET_Command()
{
    QByteArray command;
    command.append(QString("RESET"));
    command.append(QString(" "));
    command.append(QString("0"));
    command.append(QString("\r"));
    qDebug()<<command;

    QByteArray reply;
    reply=m_ntcomm->SendCommandRecieveReply(command);
    qDebug()<<reply;

    bool valid=WxNDISystemCRC::GetInstance()->SystemCheckCRC(reply.data());
    if(valid)
    {
        if(reply.startsWith("RESET"))
        {
            qDebug()<<"RESET OK!";
        }
        else
        {
            qDebug()<<"-----RESET ERROR"<<reply.mid(5,2).toInt()<<"-----";
        }
    }
}

void WxNDIPolarisVega::TSTART_Command()
{
    QByteArray command;
    command.append(QString("TSTART"));
    command.append(QString(" "));
    command.append(QString("\r"));
    qDebug()<<command;

    QByteArray reply;
    reply=m_ntcomm->SendCommandRecieveReply(command);
    qDebug()<<reply;

    bool valid=WxNDISystemCRC::GetInstance()->SystemCheckCRC(reply.data());
    if(valid)
    {
        if(reply.startsWith("OKAY"))
        {
            qDebug()<<"TSTART OK!";
        }
        else
        {
            qDebug()<<"-----TSTART ERROR"<<reply.mid(5,2).toInt()<<"-----";
        }
    }
}

void WxNDIPolarisVega::TSTOP_Command()
{
    QByteArray command;
    command.append(QString("TSTOP"));
    command.append(QString(" "));
    command.append(QString("\r"));
    qDebug()<<command;

    QByteArray reply;
    reply=m_ntcomm->SendCommandRecieveReply(command);
    qDebug()<<reply;

    bool valid=WxNDISystemCRC::GetInstance()->SystemCheckCRC(reply.data());
    if(valid)
    {
        if(reply.startsWith("OKAY"))
        {
            qDebug()<<"TSTOP OK!";
        }
        else
        {
            qDebug()<<"-----TSTOP ERROR"<<reply.mid(5,2).toInt()<<"-----";
        }
    }
}

void WxNDIPolarisVega::BEEP_Command()
{
    QByteArray command;
    command.append(QString("BEEP"));
    command.append(QString(" "));
    command.append(QString("1"));
    command.append(QString("\r"));
    qDebug()<<command;

    QByteArray reply;
    reply=m_ntcomm->SendCommandRecieveReply(command);
    qDebug()<<reply;

    bool valid=WxNDISystemCRC::GetInstance()->SystemCheckCRC(reply.data());
    if(valid)
    {
        if(reply.startsWith("ERROR"))
        {
            qDebug()<<"-----BEEP ERROR"<<reply.mid(5,2).toInt()<<"-----";
        }
        else
        {
            qDebug()<<"BEEP OK!";
        }
    }
}

void WxNDIPolarisVega::APIREV_Command()
{
	QByteArray command;
	command.append(QString("APIREV"));
	command.append(QString(" "));
	command.append(QString("\r"));
	qDebug()<<command;

	QByteArray reply;
	reply=m_ntcomm->SendCommandRecieveReply(command);
	qDebug()<<reply;

	bool valid=WxNDISystemCRC::GetInstance()->SystemCheckCRC(reply.data());
	if(valid)
	{
		if(reply.startsWith("ERROR"))
		{
			qDebug()<<"-----APIREV ERROR"<<reply.mid(5,2).toInt()<<"-----";
		}
		else
		{
			qDebug()<<"APIREV OK!";
		}
	}
}

void WxNDIPolarisVega::VER_Command(QString strOption)
{
	QByteArray command;
	command.append(QString("VER"));
	command.append(QString(" "));
	command.append(strOption);
	command.append(QString("\r"));
	qDebug()<<command;

	QByteArray reply;
	reply=m_ntcomm->SendCommandRecieveReply(command);
	qDebug()<<reply;

	bool valid=WxNDISystemCRC::GetInstance()->SystemCheckCRC(reply.data());
	if(valid)
	{
		if(reply.startsWith("ERROR"))
		{
			qDebug()<<"-----VER ERROR"<<reply.mid(5,2).toInt()<<"-----";
		}
		else
		{
			qDebug()<<"VER OK!";
		}
	}
}

void WxNDIPolarisVega::PHSR_Command()
{
	QByteArray command;
	command.append(QString("PHSR"));
	command.append(QString(" "));
	command.append(QString("00"));
	command.append(QString("\r"));
	qDebug()<<command;

	QByteArray reply;
	reply=m_ntcomm->SendCommandRecieveReply(command);
	qDebug()<<reply;

	bool valid=WxNDISystemCRC::GetInstance()->SystemCheckCRC(reply.data());
	if(valid)
	{
		if(reply.startsWith("ERROR"))
		{
			qDebug()<<"-----PHSR ERROR"<<reply.mid(5,2).toInt()<<"-----";
		}
		else
		{
			qDebug()<<"PHSR OK!";
		}
	}
}

 void WxNDIPolarisVega::SFLIST_Command()
 {
	 QByteArray command;
	 command.append(QString("SFLIST"));
	 command.append(QString(" "));
	 command.append(QString("03"));
	 command.append(QString("\r"));
	 qDebug()<<command;

	 QByteArray reply;
	 reply=m_ntcomm->SendCommandRecieveReply(command);
	 qDebug()<<reply;

	 bool valid=WxNDISystemCRC::GetInstance()->SystemCheckCRC(reply.data());
	 if(valid)
	 {
		 if(reply.startsWith("ERROR"))
		 {
			 qDebug()<<"-----SFLIST ERROR"<<reply.mid(5,2).toInt()<<"-----";
		 }
		 else
		 {
			 qDebug()<<"SFLIST OK!";
		 }
	 }
 }

QByteArray WxNDIPolarisVega::PHRQ_Command()
{
    QByteArray command;
    //Command
    command.append(QString("PHRQ"));
    //SPACE
    command.append(QString(" "));
    //Hardware Device
    command.append(QString("********"));
    //System Type
    command.append(QString("*"));
    //Tool Type
    command.append(QString("1"));
    //Port Number
    command.append(QString("**"));
    //Reserved
    command.append(QString("**"));
    //CR
    command.append(QString("\r"));
    qDebug()<<command;

    QByteArray reply;
    reply=m_ntcomm->SendCommandRecieveReply(command);
    qDebug()<<reply;

    bool valid=WxNDISystemCRC::GetInstance()->SystemCheckCRC(reply.data());
    if(valid)
    {
        if(reply.startsWith("ERROR"))
        {
            qDebug()<<"-----PHRQ ERROR"<<reply.mid(5,2).toInt()<<"-----";
            return 0;
        }
        else
        {
            QByteArray m_handlePort=reply.left(reply.length()-5);
            qDebug()<<"PHRQ OK!"<<m_handlePort;
            return m_handlePort;
        }
    }
}

void WxNDIPolarisVega::PVWR_Command(const QByteArray handPort,const QString &RomFileName)
{
    QByteArray romFileData=ReadRomFile(RomFileName);
    qreal chunckCount=romFileData.length()/64.0;
    for(int i=0;i<qCeil(chunckCount);i++)
    {
        QByteArray chunckData=romFileData.mid(i*64,64);

        QByteArray command;
        //Command
        command.append(QString("PVWR"));
        //SPACE
        command.append(QString(" "));
        //Port Handle
        command.append(handPort);
        //Start Address
        QVariant startAddress;
        startAddress=i*64;
        QString number = QString("%1").arg(startAddress.toInt(),4,16,QChar('0'));
        command.append(number);
        //Tool Definition Data
		QByteArray tempRomData=chunckData.toHex();
		if(tempRomData.length()!=128)
		{
			int appendnum=128-tempRomData.length();
			for (int i=0;i<appendnum;i++)
			{
				tempRomData.append("0");
			}
		}
        command.append(tempRomData);
        //CR
        command.append(QString("\r"));
        qDebug()<<command;

        QByteArray reply;
        reply=m_ntcomm->SendCommandRecieveReply(command);
        qDebug()<<reply;

        bool valid=WxNDISystemCRC::GetInstance()->SystemCheckCRC(reply.data());
        if(valid)
        {
            if(reply.startsWith("OKAY"))
            {
                qDebug()<<"PVWR OK!";
            }
            else
            {
                qDebug()<<"-----PVWR ERROR"<<reply.mid(5,2).toInt()<<"-----";
            }
        }
    }
}

void WxNDIPolarisVega::PINIT_Command(const QByteArray handPort)
{
    QByteArray command;

    command.append(QString("PINIT"));
    command.append(QString(" "));
    command.append(handPort);
    command.append(QString("\r"));
    qDebug()<<command;

    QByteArray reply;
    reply=m_ntcomm->SendCommandRecieveReply(command);
    bool valid=WxNDISystemCRC::GetInstance()->SystemCheckCRC(reply.data());
    qDebug()<<reply;

    if(valid)
    {
        if(reply.startsWith("OKAY"))
        {
            qDebug()<<"PINIT OK!";
        }
        else
        {
            qDebug()<<"-----PINIT ERROR"<<reply.mid(5,2).toInt()<<"-----";
        }
    }
}

void WxNDIPolarisVega::PHINF_Command(const QByteArray handPort)
{
	QByteArray command;

	command.append(QString("PHINF"));
	command.append(QString(" "));
	command.append(handPort);
	command.append(QString("0001"));
	command.append(QString("\r"));
	qDebug()<<command;

	QByteArray reply;
	reply=m_ntcomm->SendCommandRecieveReply(command);
	bool valid=WxNDISystemCRC::GetInstance()->SystemCheckCRC(reply.data());
	qDebug()<<reply;

	if(valid)
	{
		if(reply.startsWith("ERROR"))
		{
			qDebug()<<"-----PHINF ERROR"<<reply.mid(5,2).toInt()<<"-----";
		}
		else
		{
			qDebug()<<"PHINF OK!";
		}
	}
}	

void WxNDIPolarisVega::PENA_Command(const QByteArray handPort,QString tooltype)
{
    QByteArray command;
    command.append(QString("PENA"));
    command.append(QString(" "));
    command.append(handPort);
    command.append(tooltype);
    command.append(QString("\r"));
    qDebug()<<command;

    QByteArray reply;
    reply=m_ntcomm->SendCommandRecieveReply(command);
    qDebug()<<reply;

    bool valid=WxNDISystemCRC::GetInstance()->SystemCheckCRC(reply.data());
    if(valid)
    {
        if(reply.startsWith("OKAY"))
        {
            qDebug()<<"PENA OK!";
        }
        else
        {
            qDebug()<<"-----PENA ERROR"<<reply.mid(5,2).toInt()<<"-----";
        }
    }
}

void WxNDIPolarisVega::PHF_Command(const QByteArray handPort)
{
    QByteArray command;
    //Command
    command.append(QString("PHF"));
    //SPACE
    command.append(QString(" "));
    //
    command.append(handPort);
    //CR
    command.append(QString("\r"));
    qDebug()<<command;

    QByteArray reply;
    reply=m_ntcomm->SendCommandRecieveReply(command);
    qDebug()<<reply;

    bool valid=WxNDISystemCRC::GetInstance()->SystemCheckCRC(reply.data());
    if(valid)
    {
        if(reply.startsWith("OKAY"))
        {
            qDebug()<<"PHF OK!";
        }
        else
        {
            qDebug()<<"-----PHF ERROR"<<reply.mid(5,2).toInt()<<"-----";
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WxNDIPolarisVega::BX_Command(bool isParseData)
{
    QByteArray command;
    //Command
    command.append(QString("BX"));
    //SPACE
    command.append(QString(" "));
    //Reply Option
    command.append(QString("0001"));
    //CR
    command.append(QString("\r"));
   // qDebug()<<command;

    QByteArray reply;
    reply=m_ntcomm->SendCommandRecieveReply(command,true);
  //  qDebug()<<reply.toHex();

    bool valid=WxNDISystemCRC::GetInstance()->SystemCheckCRC(reply.data());
    if(valid)
    {
        if(reply.startsWith("ERROR"))
        {
          //  qDebug()<<"-----BX ERROR"<<reply.mid(5,2).toInt()<<"-----";
        }
        else
        {
            //qDebug()<<"BX OK!";
            if(isParseData)
            {
                QDataStream replyds(reply);
                replyds.setByteOrder(QDataStream::LittleEndian);

                quint16 startSequence;
                replyds>>startSequence;
                if((startSequence & 0xFFFF)==0xA5C4)
                {
                    replyds.skipRawData(4);
                    ParseHandles(&replyds);

					for(int i=0;i<m_HandlePortName.size();i++)
					{
                        /*qDebug() << "Probe " << i << ":" << m_QuatTransformations[m_HandlePortName.at(i)].translation.x <<
                            "," << m_QuatTransformations[m_HandlePortName.at(i)].translation.y <<
                            "," << m_QuatTransformations[m_HandlePortName.at(i)].translation.z <<
                            "," << m_QuatTransformations[m_HandlePortName.at(i)].rotation.q0 <<
                            "," << m_QuatTransformations[m_HandlePortName.at(i)].rotation.qx <<
                            "," << m_QuatTransformations[m_HandlePortName.at(i)].rotation.qy <<
                            "," << m_QuatTransformations[m_HandlePortName.at(i)].rotation.qz;*/
					}

                }
                else
                {
                     //qDebug()<<"-----BX Reply StartSequence Error-----";
                }
            }
        }
    }
}

void WxNDIPolarisVega::ParseHandles(QDataStream *ds)
{
    qint8 numberOfHandles;
    *ds>>numberOfHandles;

    for(;numberOfHandles>0;numberOfHandles--)
    {
        ParseOneHandle(ds);
    }

    qint16 systemStatus;
    *ds>>systemStatus;
}

void WxNDIPolarisVega::ParseOneHandle(QDataStream *ds)
{
    qint8 handleN;
    *ds>>handleN;
    qint8 handleStatus;
    *ds>>handleStatus;

    QuatTransformation qtransform;
    if(handleStatus==0x01)
    {
        qtransform=ParseReplyOption0001(ds);
		QByteArray handleport=QString("%1").arg(handleN,2,16,QChar('0')).toUpper().toLocal8Bit();
		m_QuatTransformations[handleport]=qtransform;
    }
    quint32 portStatus,frameNumber;
    *ds>>portStatus;
    *ds>>frameNumber;
}

WxNDIPolarisVega::QuatTransformation WxNDIPolarisVega::ParseReplyOption0001(QDataStream *ds)
{
    quint32 binQ0,binQx,binQy,binQz,binTx,binTy,binTz,binError;
    *ds>>binQ0;
    *ds>>binQx;
    *ds>>binQy;
    *ds>>binQz;
    *ds>>binTx;
    *ds>>binTy;
    *ds>>binTz;
    *ds>>binError;

    float q0=MEMORY_TYPE_CAST(float,binQ0);
    float qx=MEMORY_TYPE_CAST(float,binQx);
    float qy=MEMORY_TYPE_CAST(float,binQy);
    float qz=MEMORY_TYPE_CAST(float,binQz);
    float tx=MEMORY_TYPE_CAST(float,binTx);
    float ty=MEMORY_TYPE_CAST(float,binTy);
    float tz=MEMORY_TYPE_CAST(float,binTz);
    float error=MEMORY_TYPE_CAST(float,binError);

    QuatTransformation qtransform;
    qtransform.rotation.q0=q0;
    qtransform.rotation.qx=qx;
    qtransform.rotation.qy=qy;
    qtransform.rotation.qz=qz;
    qtransform.translation.x=tx;
    qtransform.translation.y=ty;
    qtransform.translation.z=tz;

    return qtransform;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QByteArray WxNDIPolarisVega::ReadRomFile(const QString &RomFileName)
{
    QByteArray romData;
    QFile romFile(RomFileName);
    bool isOpened=romFile.open(QIODevice::ReadOnly);
    if (!isOpened)
    {
        qDebug()<<"-----Rom File Open Error-----";
    }
    else
    {
        romData=romFile.readAll();
    }
    return romData;
}

void WxNDIPolarisVega::QuaternionToRotationMatrix( QuatTransformation* pdtQuatRot,RotationMatrix dtRotMatrix )
{
    float
        fQ0Q0,
        fQxQx,
        fQyQy,
        fQzQz,
        fQ0Qx,
        fQ0Qy,
        fQ0Qz,
        fQxQy,
        fQxQz,
        fQyQz;

        //Determine some calculations done more than once
        fQ0Q0 = pdtQuatRot->rotation.q0 * pdtQuatRot->rotation.q0;
        fQxQx = pdtQuatRot->rotation.qx * pdtQuatRot->rotation.qx;
        fQyQy = pdtQuatRot->rotation.qy * pdtQuatRot->rotation.qy;
        fQzQz = pdtQuatRot->rotation.qz * pdtQuatRot->rotation.qz;
        fQ0Qx = pdtQuatRot->rotation.q0 * pdtQuatRot->rotation.qx;
        fQ0Qy = pdtQuatRot->rotation.q0 * pdtQuatRot->rotation.qy;
        fQ0Qz = pdtQuatRot->rotation.q0 * pdtQuatRot->rotation.qz;
        fQxQy = pdtQuatRot->rotation.qx * pdtQuatRot->rotation.qy;
        fQxQz = pdtQuatRot->rotation.qx * pdtQuatRot->rotation.qz;
        fQyQz = pdtQuatRot->rotation.qy * pdtQuatRot->rotation.qz;

        //Determine the rotation matrix elements.
        dtRotMatrix[0][0] = fQ0Q0 + fQxQx - fQyQy - fQzQz;
        dtRotMatrix[0][1] = 2.0 * (-fQ0Qz + fQxQy);
        dtRotMatrix[0][2] = 2.0 * (fQ0Qy + fQxQz);
        dtRotMatrix[0][3] =pdtQuatRot->translation.x;
        dtRotMatrix[1][0] = 2.0 * (fQ0Qz + fQxQy);
        dtRotMatrix[1][1] = fQ0Q0 - fQxQx + fQyQy - fQzQz;
        dtRotMatrix[1][2] = 2.0 * (-fQ0Qx + fQyQz);
        dtRotMatrix[1][3] =pdtQuatRot->translation.y;
        dtRotMatrix[2][0] = 2.0 * (-fQ0Qy + fQxQz);
        dtRotMatrix[2][1] = 2.0 * (fQ0Qx + fQyQz);
        dtRotMatrix[2][2] = fQ0Q0 - fQxQx - fQyQy + fQzQz;
        dtRotMatrix[2][3] =pdtQuatRot->translation.z;
        dtRotMatrix[3][0] =0;
        dtRotMatrix[3][1] =0;
        dtRotMatrix[3][2] =0;
        dtRotMatrix[3][3] =1;
}
