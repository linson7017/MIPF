#ifndef CQF_MainCommand_h__
#define CQF_MainCommand_h__
#include "qf_config.h"
#include "iqf_command.h"

#include "mitkDataNode.h"
#include "mitkNodePredicateAnd.h"
#include "mitkNodePredicateOr.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateNot.h"

#pragma once

namespace QF {
	class IQF_Main;
}
class CQF_ManualSegmentation;

class CQF_MainCommand :public QF::IQF_Command
{
public:
	CQF_MainCommand(QF::IQF_Main* pMain);
	~CQF_MainCommand();
	void Release();
	virtual bool ExecuteCommand(const char* szCommandID, QF::IQF_Properties* pInParam, QF::IQF_Properties* pOutParam);
	virtual int GetCommandCount();
	virtual const char* GetCommandID(int iIndex);

    void SetSegmentationImp(CQF_ManualSegmentation* pSegmentation) { m_pSegmentation = pSegmentation; }
private:
	QF::IQF_Main* m_pMain;
    CQF_ManualSegmentation* m_pSegmentation;

    mitk::DataNode* m_refNode;
    mitk::DataNode* m_workingNode;
};

#endif // CQF_MainCommand_h__
