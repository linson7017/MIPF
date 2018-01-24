#ifndef CQF_TestCommand_h__
#define CQF_TestCommand_h__
#include "qf_config.h"
#include "iqf_command.h"

#pragma once

namespace QF {
    class IQF_Main;
}

class CQF_TestCommand :public QF::IQF_Command
{
public:
    CQF_TestCommand(QF::IQF_Main* pMain);
    ~CQF_TestCommand();
    void Release();
    virtual bool ExecuteCommand(const char* szCommandID, QF::IQF_PropertySet* pInParam, QF::IQF_PropertySet* pOutParam);
    virtual int GetCommandCount();
    virtual const char* GetCommandID(int iIndex);
private:

    QF::IQF_Main* m_pMain;
};

#endif // CQF_TestCommand_h__