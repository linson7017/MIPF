#ifndef Mipf_Plugin_VolumeCutActivator_h__
#define Mipf_Plugin_VolumeCutActivator_h__

#pragma once
#include "Activator_Base.h"

class Mipf_Plugin_VolumeCut_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_VolumeCut_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register();
};

#endif // Mipf_Plugin_VolumeCutActivator_h__