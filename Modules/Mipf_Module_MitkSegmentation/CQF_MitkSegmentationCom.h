#ifndef CQF_MitkSegmentationCom_h__
#define CQF_MitkSegmentationCom_h__

#pragma once
#include "iqf_component.h"

class CQF_MainCommand;
class CQF_MainMessage;
class CQF_ManualSegmentation;
class CQF_SurfaceTool;

class CQF_MitkSegmentationCom :public QF::IQF_Component
{
public:
	CQF_MitkSegmentationCom(QF::IQF_Main* pMain);
	~CQF_MitkSegmentationCom();
	virtual void Release();
	virtual bool Init();
	virtual void* GetInterfacePtr(const char* szInterfaceID);
	const char* GetComponentID() { return "QF_Component_MitkSegmentation"; }
	int GetInterfaceCount();
	const char* GetInterfaceID(int iID);
private:
    CQF_MainCommand* m_pMainCommand;
    CQF_MainMessage* m_pMainMessage;

    CQF_ManualSegmentation* m_pSegmentation;
    CQF_SurfaceTool* m_pSurfaceTool;

	QF::IQF_Main* m_pMain;
};

#endif // CQF_MitkSegmentationCom_h__
