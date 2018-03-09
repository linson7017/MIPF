#ifndef Mipf_Plugin_GraphcutSegmentationActivator_h__
#define Mipf_Plugin_GraphcutSegmentationActivator_h__

#pragma once
#include "iqf_activator.h"
#include "Activator_Base.h"

class GraphcutSegmentationView;

class Mipf_Plugin_GraphcutSegmentation_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_GraphcutSegmentation_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register();
    void Constructed();
private:
    GraphcutSegmentationView* m_pGraphcutSegmentationView;
};

#endif // Mipf_Plugin_GraphcutSegmentationActivator_h__