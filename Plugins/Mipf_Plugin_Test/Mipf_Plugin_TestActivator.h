#ifndef Mipf_Plugin_TestActivator_h__
#define Mipf_Plugin_TestActivator_h__

#pragma once
#include "Activator_Base.h"

class Mipf_Plugin_Test_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_Test_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register();
};

#endif // Mipf_Plugin_TestActivator_h__