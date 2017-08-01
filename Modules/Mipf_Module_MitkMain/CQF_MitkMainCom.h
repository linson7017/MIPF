#ifndef CQF_SliceBySliceTracking_h__
#define CQF_SliceBySliceTracking_h__

#pragma once
#include "iqf_component.h"

class CQF_MitkDataManager;
class CQF_MitkRenderWindow;
class CQF_MitkReference;
class CQF_MitkIO;
class CQF_MitkDisplayOption;

class CQF_MainCommand;

class CQF_MitkMain :public QF::IQF_Component
{
public:
    CQF_MitkMain(QF::IQF_Main* pMain);
    ~CQF_MitkMain();
    virtual void Release();
    virtual bool Init();
    virtual void* GetInterfacePtr(const char* szInterfaceID);
    const char* GetComponentID() { return "QF_Component_MitkMain"; }
    int GetInterfaceCount();
    const char* GetInterfaceID(int iID);
private:
    CQF_MitkDataManager* m_pMitkDataManager;
    CQF_MitkRenderWindow* m_pMitkRenderWindow;
    CQF_MitkReference* m_pMitkReference;
    CQF_MitkIO* m_pMitkIO;
    CQF_MitkDisplayOption* m_pDisplayOption;

	CQF_MainCommand* m_pMainCommand;

    QF::IQF_Main* m_pMain;
};

#endif // CQF_SliceBySliceTracking_h__
