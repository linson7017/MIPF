#pragma once
#include "iqf_activator.h"

class SliceBySliceTrackingView;

class SliceBySliceTrackingView_Activator: public QF::IQF_Activator
{
public:
    SliceBySliceTrackingView_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
private:
    SliceBySliceTrackingView* m_pView;
    QF::IQF_Main* m_PMain;
};

