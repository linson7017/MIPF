#ifndef Mipf_Plugin_VMTKToolsActivator_h__
#define Mipf_Plugin_VMTKToolsActivator_h__

#pragma once
#include "Activator_Base.h"

class CenterLineExtractView;

class Mipf_Plugin_VMTKTools_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_VMTKTools_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register();
private:
};

#endif // Mipf_Plugin_VMTKToolsActivator_h__