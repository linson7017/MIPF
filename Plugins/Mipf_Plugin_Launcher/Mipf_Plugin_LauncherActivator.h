#ifndef Mipf_Plugin_LauncherActivator_h__
#define Mipf_Plugin_LauncherActivator_h__

#pragma once
#include "Activator_Base.h"

class LauncherView;

class Mipf_Plugin_Launcher_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_Launcher_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register();
private:
    LauncherView* m_pLauncherView;
};

#endif // Mipf_Plugin_LauncherActivator_h__