#ifndef CQF_CVACommand_h__
#define CQF_CVACommand_h__
#include "qf_config.h"
#include "iqf_command.h"

#pragma once

namespace QF {
    class IQF_Main;
}

class CQF_CVACommand :public QF::IQF_Command
{
public:
    CQF_CVACommand(QF::IQF_Main* pMain);
    ~CQF_CVACommand();
    void Release();
    virtual bool ExecuteCommand(const char* szCommandID, QF::IQF_Properties* pInParam, QF::IQF_Properties* pOutParam);
    virtual int GetCommandCount();
    virtual const char* GetCommandID(int iIndex);
private:

    QF::IQF_Main* m_pMain;
};

#endif // CQF_CVACommand_h__