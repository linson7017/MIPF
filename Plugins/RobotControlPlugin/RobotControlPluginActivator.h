#ifndef RobotControlPluginActivator_h__
#define RobotControlPluginActivator_h__

#pragma once
#include "iqf_activator.h"

class RobotControlView;

class RobotControlPlugin_Activator : public QF::IQF_Activator
{
public:
    RobotControlPlugin_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
private:
    RobotControlView* m_pRobotControlView;
    QF::IQF_Main* m_PMain;
};

#endif // RobotControlPluginActivator_h__