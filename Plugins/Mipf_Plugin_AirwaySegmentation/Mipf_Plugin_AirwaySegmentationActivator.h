#ifndef Mipf_Plugin_AirwaySegmentationActivator_h__
#define Mipf_Plugin_AirwaySegmentationActivator_h__

#pragma once
#include "Activator_Base.h"

class AirwaySegmentationView;

class Mipf_Plugin_AirwaySegmentation_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_AirwaySegmentation_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
	void Constructed(R* pR);
private:
    AirwaySegmentationView* m_pAirwaySegmentationView;
};

#endif // Mipf_Plugin_AirwaySegmentationActivator_h__