#ifndef CQF_CoreCom_h__
#define CQF_CoreCom_h__

#pragma once
#include "iqf_component.h"


class CQF_ObjectFactory;


class CQF_CoreCom :public QF::IQF_Component
{
public:
    CQF_CoreCom(QF::IQF_Main* pMain);
    ~CQF_CoreCom();
    virtual void Release();
    virtual bool Init();
    virtual void* GetInterfacePtr(const char* szInterfaceID);
    const char* GetComponentID() { return "QF_Component_Core"; }
    int GetInterfaceCount();
    const char* GetInterfaceID(int iID);
private:

   QF::IQF_Main* m_pMain;

   CQF_ObjectFactory* m_pObjectFactory;
};

#endif // CQF_CoreCom_h__