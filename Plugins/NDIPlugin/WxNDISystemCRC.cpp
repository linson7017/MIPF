#include "WxNDISystemCRC.h"

#define CARRIAGE_RETURN	0xD
#define MAX_REPLY_MSG     4096

static unsigned int CrcTable[256];

WxNDISystemCRC *WxNDISystemCRC::m_instance=NULL;

WxNDISystemCRC::WxNDISystemCRC()
{
    int i,j;
    long lCrcTable;
    /*
         * Create the CRC lookup table
         */
    for( i=0; i<256; i++ )
    {
        lCrcTable = i;
        for( j=0; j<8; j++ )
            lCrcTable = ( lCrcTable >> 1 ) ^ (( lCrcTable & 1 ) ? 0xA001L : 0 );

        CrcTable[i] = (unsigned int) lCrcTable & 0xFFFF;
    } /* for */
}

WxNDISystemCRC *WxNDISystemCRC::GetInstance()
{
    if(m_instance==NULL)
    {
        m_instance=new WxNDISystemCRC;
    }
    return m_instance;
}

int WxNDISystemCRC::SystemCheckCRC(char *psz)
{
    unsigned int
            uCrc = 0,
            uReplyCrc = 0,
            uReplySize = 0;

    int
            m, n;
    /*
     * calculate CRC
     */
    uCrc = 0;

    /*
     * We need to check if the reply is for BX, in binary format.
     * The start byte shall be 0xA5C4
     */
    if ( ((psz[0] & 0xff) == 0xc4) &&
         ((psz[1] & 0xff) == 0xa5) )
    {
        uReplyCrc = (psz[4] & 0xff) | ((psz[5] & 0xff) << 8); //get the header CRC

        if (CalcCRCByLen(psz, 4) == uReplyCrc) //Check the header CRC
        {
            /*
                 *  Get the reply size.
                 *  = reply size at [2] and [3] + 6 header bytes + 2 CRC bytes.
                 */
            uReplySize = ((psz[2] & 0xff) | ((psz[3] & 0xff) << 8)) + 8;

            /* Get the body CRC */
            uReplyCrc = (psz[uReplySize-2] & 0xff) | ((psz[uReplySize-1] & 0xff) << 8);

            if (CalcCRCByLen(&psz[6], (uReplySize-8)) == uReplyCrc) // Check the CRC
            {
                return 1; /* CRC check OK */
            }
            else
            {
                return 0; /* Bad CRC */
            }/* else */
        }
        else
        {
            return 0; /* Bad CRC */
        }/* else */
    }
    else
    {
        for( n = 0; (psz[n]!= CARRIAGE_RETURN) && (n< MAX_REPLY_MSG); n++)
        {
            ; /* get total number of bytes n */
        }/* for */

        /*
             * if rolled over the buffer size then something is wrong and
             * we will say the CRC is bad
             */
        if(n>=MAX_REPLY_MSG)
            return 0;

        /*
             * NULL terminate the string to be tidy
             */
        psz[n+1] = 0;

        /*
             * determine 16 bit CRC
             */
        for(m=0;m<(n-4);m++)
            uCrc = CalcCrc16(uCrc,psz[m]);

        /*
             * read CRC from message
             */
        sscanf(&(psz[n-4]),"%04x",&uReplyCrc);

        /*
             * return the comparison of the calculated and received CRC values
             */
        return (uCrc == uReplyCrc);

    }/* else */
}

unsigned int WxNDISystemCRC::CalcCrc16(unsigned int crc, int data)
{
        crc = CrcTable[ (crc ^ data) & 0xFF] ^ (crc >> 8);
        return (crc & 0xFFFF);
}

unsigned WxNDISystemCRC::CalcCRCByLen(char *pszString, int nLen)
{
    static unsigned char
            oddparity[16] = { 0, 1, 1, 0, 1, 0, 0, 1,
                              1, 0, 0, 1, 0, 1, 1, 0 };
        unsigned
            data,
            uCrc = 0;
        unsigned char
            *puch = (unsigned char *)pszString;
        int
            nCnt = 0;

        while ( nCnt < nLen )
        {
            data = (*puch ^ (uCrc & 0xff)) & 0xff;
            uCrc >>= 8;

            if ( oddparity[data & 0x0f] ^ oddparity[data >> 4] )
            {
                uCrc ^= 0xc001;
            } /* if */

            data <<= 6;
            uCrc ^= data;
            data <<= 1;
            uCrc ^= data;
            puch++;
            nCnt++;
        } /* while */

        return uCrc;
}


