#include "WxNetworkComm.h"

WxNetworkComm::WxNetworkComm(QObject *parent) : QObject(parent), m_socket(NULL)
{
    m_socket=new QTcpSocket;
}

WxNetworkComm::~WxNetworkComm()
{
    DisconnectDevice();
	delete m_socket;
}

bool WxNetworkComm::ConnectDevice(QString deviceIP,int devicePort)
{    
	if(m_socket!=NULL)
	{
		m_socket->connectToHost(deviceIP,devicePort);
		return m_socket->waitForConnected(5000);
	}
	else
	{
		return false;
	}
}

void WxNetworkComm::DisconnectDevice()
{
    if(m_socket!=NULL)
    {
        m_socket->disconnectFromHost();
        m_socket->close();
    }
}

QByteArray WxNetworkComm::SendCommandRecieveReply(const QByteArray &command,bool isBinaryResponse)
{
    QByteArray reply;
    if(m_socket!=NULL)
    {
        qint64 length=m_socket->write(command);
        if(length==command.length())
        {
            while (true) {
                if(m_socket->waitForReadyRead())
                {
                    reply.append(m_socket->readAll());
                }
                if(reply.endsWith("\r") || isBinaryResponse)
                {
                    break;
                }
            }
        }
    }
    return reply;
}
