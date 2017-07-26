#ifndef CQF_CQF_AirwaySegmentationCom_h__
#define CQF_CQF_AirwaySegmentationCom_h__

#pragma once
#include "iqf_component.h"

class IQF_SegmentationMethodFactory;

class CQF_SegmentationCom :public QF::IQF_Component
{
public:
	CQF_SegmentationCom(QF::IQF_Main* pMain);
    ~CQF_SegmentationCom();
    virtual void Release();
    virtual bool Init();
    virtual void* GetInterfacePtr(const char* szInterfaceID);
    const char* GetComponentID() { return "QF_Component_SegmentationCom"; }
    int GetInterfaceCount();
    const char* GetInterfaceID(int iID);
private:
    IQF_SegmentationMethodFactory* m_pSegmentationMethodFactory;

   QF::IQF_Main* m_pMain;
};

#endif // CQF_CQF_AirwaySegmentationCom_h__