#ifndef Mipf_Plugin_SFLSSegmentationActivator_h__
#define Mipf_Plugin_SFLSSegmentationActivator_h__

#pragma once
#include "Activator_Base.h"

class SFLSSegmentationView;

class Mipf_Plugin_SFLSSegmentation_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_SFLSSegmentation_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
private:
    SFLSSegmentationView* m_pSFLSSegmentationView;
};

#endif // Mipf_Plugin_SFLSSegmentationActivator_h__