#ifndef CQF_TestCom_h__
#define CQF_TestCom_h__

#pragma once
#include "iqf_component.h"

class CQF_TestCommand;
class CQF_TestMessage;
class Test;

class CQF_TestCom :public QF::IQF_Component
{
public:
    CQF_TestCom(QF::IQF_Main* pMain);
    ~CQF_TestCom();
    virtual void Release();
    virtual bool Init();
    virtual void* GetInterfacePtr(const char* szInterfaceID);
    const char* GetComponentID() { return "QF_Component_Test"; }
    int GetInterfaceCount();
    const char* GetInterfaceID(int iID);
private:
    CQF_TestCommand* m_pMainCommand;
    CQF_TestMessage* m_pMainMessage;

    Test* m_pTest;
   QF::IQF_Main* m_pMain;
};

#endif // CQF_TestCom_h__