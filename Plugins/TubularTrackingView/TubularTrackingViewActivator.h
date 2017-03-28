#pragma once
#include "iqf_activator.h"

class TubularTrackingView;

class TubularTrackingView_Activator: public QF::IQF_Activator
{
public:
    TubularTrackingView_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
private:
    TubularTrackingView* m_pView;
    QF::IQF_Main* m_PMain;
};

