#pragma once

#include <QObject>
#include <QFile>
#include <QTcpSocket>


class  WxNetworkComm : public QObject
{
    Q_OBJECT
public:
    explicit WxNetworkComm(QObject *parent = 0);
    ~WxNetworkComm();

public:
    bool ConnectDevice(QString,int);
    void DisconnectDevice();

    QByteArray SendCommandRecieveReply(const QByteArray &,bool isBinaryResponse=false);

private:
    QTcpSocket *m_socket;
};
