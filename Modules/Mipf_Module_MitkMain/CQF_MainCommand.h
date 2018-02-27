#ifndef CQF_MainCommand_h__
#define CQF_MainCommand_h__
#include "qf_config.h"
#include "iqf_command.h"

#pragma once

namespace QF{
	class IQF_Main;
}

class CQF_MainCommand:public QF::IQF_Command
{
public:
	CQF_MainCommand(QF::IQF_Main* pMain);
	~CQF_MainCommand();
	void Release();
	virtual bool ExecuteCommand(const char* szCommandID, QF::IQF_Properties* pInParam, QF::IQF_Properties* pOutParam) ;
	virtual int GetCommandCount() ;
	virtual const char* GetCommandID(int iIndex) ;
private:

	QF::IQF_Main* m_pMain;
};

#endif // CQF_MainCommand_h__
