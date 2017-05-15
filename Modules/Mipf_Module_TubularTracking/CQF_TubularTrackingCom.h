#ifndef CQF_TubularTrackingCom_h__
#define CQF_TubularTrackingCom_h__

#include "iqf_component.h"

class TubularTracking;


class CQF_TubularTrackingCom :public QF::IQF_Component
{
public:
    CQF_TubularTrackingCom();
    ~CQF_TubularTrackingCom();

    virtual void Release();
    virtual bool Init();
    virtual void* GetInterfacePtr(const char* szInterfaceID);
    const char* GetComponentID() { return "QF_Component_TububarTracking"; }
    int GetInterfaceCount();
    const char* GetInterfaceID(int iID);
private:
    TubularTracking* m_pTubularTracing;
};

#endif // CQF_TubularTrackingCom_h__
