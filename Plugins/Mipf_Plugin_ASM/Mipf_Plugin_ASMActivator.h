#ifndef Mipf_Plugin_ASMActivator_h__
#define Mipf_Plugin_ASMActivator_h__

#pragma once
#include "Activator_Base.h"

class Mipf_Plugin_ASM_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_ASM_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register();
};

#endif // Mipf_Plugin_ASMActivator_h__