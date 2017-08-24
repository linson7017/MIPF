#ifndef CQF_VesselToolsCom_h__
#define CQF_VesselToolsCom_h__

#pragma once
#include "iqf_component.h"

class CQF_VesselToolsCom :public QF::IQF_Component
{
public:
    CQF_VesselToolsCom(QF::IQF_Main* pMain);
    ~CQF_VesselToolsCom();
    virtual void Release();
    virtual bool Init();
    virtual void* GetInterfacePtr(const char* szInterfaceID);
    const char* GetComponentID() { return "QF_Component_CQF_VesselToolsCom"; }
    int GetInterfaceCount();
    const char* GetInterfaceID(int iID);
private:
    
    

   QF::IQF_Main* m_pMain;
};

#endif // CQF_VesselToolsCom_h__