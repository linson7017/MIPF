#pragma once
#include "Activator_Base.h"

class SliceBySliceTrackingView;

class SliceBySliceTrackingView_Activator: public ActivatorBase
{
public:
    SliceBySliceTrackingView_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
    void Constructed(R* pR);
private:
    SliceBySliceTrackingView* m_pView;
};

