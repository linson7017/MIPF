#pragma once

#include <QObject>

class  WxNDISystemCRC
{
public:
    static WxNDISystemCRC *GetInstance();

public:
    int SystemCheckCRC(char *psz);
    unsigned int CalcCrc16( unsigned int crc, int data );
    unsigned CalcCRCByLen( char *pszString, int nLen );

private:
    WxNDISystemCRC();

private:

    static WxNDISystemCRC *m_instance;
};
