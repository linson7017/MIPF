#ifndef NDIPluginActivator_h__
#define NDIPluginActivator_h__

#pragma once
#include "iqf_activator.h"

class NDIView;

class NDIPlugin_Activator : public QF::IQF_Activator
{
public:
    NDIPlugin_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
private:
    NDIView* m_pNDIView;
    QF::IQF_Main* m_PMain;
};

#endif // NDIPluginActivator_h__