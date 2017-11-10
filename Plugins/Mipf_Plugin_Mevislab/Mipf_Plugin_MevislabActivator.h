#ifndef Mipf_Plugin_MevislabActivator_h__
#define Mipf_Plugin_MevislabActivator_h__

#pragma once
#include "Activator_Base.h"

class Mipf_Plugin_Mevislab_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_Mevislab_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
};

#endif // Mipf_Plugin_MevislabActivator_h__