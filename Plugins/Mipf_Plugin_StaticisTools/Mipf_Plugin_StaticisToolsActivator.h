#ifndef Mipf_Plugin_StaticisToolsActivator_h__
#define Mipf_Plugin_StaticisToolsActivator_h__

#pragma once
#include "Activator_Base.h"

class Mipf_Plugin_StaticisTools_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_StaticisTools_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
private:

};

#endif // Mipf_Plugin_StaticisToolsActivator_h__