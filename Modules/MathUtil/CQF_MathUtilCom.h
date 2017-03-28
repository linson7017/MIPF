#ifndef CQF_SliceBySliceTracking_h__
#define CQF_SliceBySliceTracking_h__

#pragma once
#include "iqf_component.h"

class MathUtil;

class CQF_MathUtil :public QF::IQF_Component
{
public:
    CQF_MathUtil();
    ~CQF_MathUtil();
    virtual void Release();
    virtual bool Init();
    virtual void* GetInterfacePtr(const char* szInterfaceID);
    const char* GetComponentID() { return "QF_Component_MathUtil"; }
    int GetInterfaceCount();
    const char* GetInterfaceID(int iID);
private:
    MathUtil* m_mathUtil;
};

#endif // CQF_SliceBySliceTracking_h__
