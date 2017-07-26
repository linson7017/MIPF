#ifndef CQF_MitkStdCommand_h__
#define CQF_MitkStdCommand_h__
#include "qf_config.h"
#include "iqf_command.h"

#pragma once

class PointList;
namespace QF {
    class IQF_Main;
}

class CQF_MitkStdCommand :public QF::IQF_Command
{
public:
    CQF_MitkStdCommand(QF::IQF_Main* pMain);
    ~CQF_MitkStdCommand();
    void Release();
    virtual bool ExecuteCommand(const char* szCommandID, QF::IQF_PropertySet* pInParam, QF::IQF_PropertySet* pOutParam);
    virtual int GetCommandCount();
    virtual const char* GetCommandID(int iIndex);
private:

    QF::IQF_Main* m_pMain;

    PointList* m_pPointList;
};

#endif // CQF_MitkStdCommand_h__