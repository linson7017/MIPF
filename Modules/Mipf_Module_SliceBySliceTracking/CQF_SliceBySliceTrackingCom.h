#ifndef CQF_SliceBySliceTracking_h__
#define CQF_SliceBySliceTracking_h__

#pragma once
#include "iqf_component.h"
#include "SliceBySliceBlobTracking.h"

class CQF_SliceBySliceTracking :public QF::IQF_Component
{
public:
    CQF_SliceBySliceTracking();
    ~CQF_SliceBySliceTracking();
    virtual void Release();
    virtual bool Init();
    virtual void* GetInterfacePtr(const char* szInterfaceID);
    const char* GetComponentID() { return "QF_Component_TububarTracking"; }
    int GetInterfaceCount();
    const char* GetInterfaceID(int iID);
private:
    SliceBySliceBlobTracking* m_pSliceBySliceTracing;
};

#endif // CQF_SliceBySliceTracking_h__
