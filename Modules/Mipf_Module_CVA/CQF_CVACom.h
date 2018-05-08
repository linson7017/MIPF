#ifndef CQF_CVACom_h__
#define CQF_CVACom_h__

#pragma once
#include "iqf_component.h"

class CQF_CVACommand;
class CQF_CVAMessage;
class CQF_DSATool;

class CQF_CVACom :public QF::IQF_Component
{
public:
    CQF_CVACom(QF::IQF_Main* pMain);
    ~CQF_CVACom();
    virtual void Release();
    virtual bool Init();
    virtual void* GetInterfacePtr(const char* szInterfaceID);
    const char* GetComponentID() { return "QF_Component_CVA"; }
    int GetInterfaceCount();
    const char* GetInterfaceID(int iID);
private:
    CQF_CVACommand* m_pMainCommand;
    CQF_CVAMessage* m_pMainMessage;
    CQF_DSATool* m_pDSATool;

   QF::IQF_Main* m_pMain;
};

#endif // CQF_CVACom_h__