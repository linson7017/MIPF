#ifndef CQF_MainMessage_h__
#define CQF_MainMessage_h__

#include "qf_config.h"
#include "iqf_message.h"

namespace QF
{
    class IQF_Main;
}


class CQF_MainMessage : public QF::IQF_Message
{
public:
    CQF_MainMessage(QF::IQF_Main* pMain);

    ~CQF_MainMessage();
    void Release();

    virtual int GetMessageCount();

    virtual const char* GetMessageID(int iIndex);

    virtual void OnMessage(const char* szMessage, int iValue, void *pValue) ;

private:
    QF::IQF_Main*  m_pMain;
};



#endif // CQF_MainMessage_h__
