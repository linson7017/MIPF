#ifndef CQF_MitkStdCom_h__
#define CQF_MitkStdCom_h__

#pragma once
#include "iqf_component.h"

class CQF_MitkStdCommand;
class CQF_MitkStdMessage;
class PointListFactory;

class CQF_MitkStdCom :public QF::IQF_Component
{
public:
    CQF_MitkStdCom(QF::IQF_Main* pMain);
    ~CQF_MitkStdCom();
    virtual void Release();
    virtual bool Init();
    virtual void* GetInterfacePtr(const char* szInterfaceID);
    const char* GetComponentID() { return "QF_Component_CQF_MitkStdCom"; }
    int GetInterfaceCount();
    const char* GetInterfaceID(int iID);
private:
    CQF_MitkStdCommand* m_pMainCommand;

    PointListFactory* m_pPointListFactory;
    QF::IQF_Main* m_pMain;
};

#endif // CQF_MitkStdCom_h__