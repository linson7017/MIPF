#ifndef Mipf_Plugin_PCLActivator_h__
#define Mipf_Plugin_PCLActivator_h__

#pragma once
#include "Activator_Base.h"

class Mipf_Plugin_PCL_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_PCL_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register();
};

#endif // Mipf_Plugin_PCLActivator_h__