#ifndef Mipf_Plugin_DeepMedicActivator_h__
#define Mipf_Plugin_DeepMedicActivator_h__

#pragma once
#include "Activator_Base.h"

class Mipf_Plugin_DeepMedic_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_DeepMedic_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
};

#endif // Mipf_Plugin_DeepMedicActivator_h__