#ifndef Mipf_Plugin_ManualSegmentationTestActivator_h__
#define Mipf_Plugin_ManualSegmentationTestActivator_h__

#pragma once
#include "Activator_Base.h"

class ManualSegmentationTestView;

class Mipf_Plugin_ManualSegmentationTest_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_ManualSegmentationTest_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
    void Constructed(R* pR);
private:
    ManualSegmentationTestView* m_pManualSegmentationTestView;
};

#endif // Mipf_Plugin_ManualSegmentationTestActivator_h__