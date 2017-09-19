#ifndef Mipf_Plugin_SurfaceCutActivator_h__
#define Mipf_Plugin_SurfaceCutActivator_h__

#pragma once
#include "Activator_Base.h"

class Mipf_Plugin_SurfaceCut_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_SurfaceCut_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
private:

};

#endif // Mipf_Plugin_SurfaceCutActivator_h__