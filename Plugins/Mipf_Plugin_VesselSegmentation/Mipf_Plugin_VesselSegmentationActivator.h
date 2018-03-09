#ifndef Mipf_Plugin_VesselSegmentationActivator_h__
#define Mipf_Plugin_VesselSegmentationActivator_h__

#pragma once
#include "Activator_Base.h"

class VesselSegmentationView;

class Mipf_Plugin_VesselSegmentation_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_VesselSegmentation_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register();
private:
    //VesselSegmentationView* m_pVesselSegmentationView;
};

#endif // Mipf_Plugin_VesselSegmentationActivator_h__