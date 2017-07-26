#ifndef Mipf_Plugin_GrowcutActivator_h__
#define Mipf_Plugin_GrowcutActivator_h__

#pragma once
#include "Activator_Base.h"

class GrowcutView;

class Mipf_Plugin_Growcut_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_Growcut_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
	void Constructed(R* pR);
private:
    GrowcutView* m_pGrowcutView;
};

#endif // Mipf_Plugin_GrowcutActivator_h__