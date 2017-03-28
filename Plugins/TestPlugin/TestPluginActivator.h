#ifndef TestPluginActivator_h__
#define TestPluginActivator_h__

#pragma once
#include "iqf_activator.h"

class TestView;

class TestPlugin_Activator : public QF::IQF_Activator
{
public:
    TestPlugin_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
private:
    TestView* m_pTestView;
    QF::IQF_Main* m_PMain;
};

#endif // TestPluginActivator_h__