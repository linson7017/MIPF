#pragma once
#include "Activator_Base.h"

class TubularTrackingView;

class TubularTrackingView_Activator: public ActivatorBase
{
public:
    TubularTrackingView_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register();
private:
    TubularTrackingView* m_pView;
};

