#ifndef Mipf_Plugin_BasicImageProcessActivator_h__
#define Mipf_Plugin_BasicImageProcessActivator_h__

#pragma once
#include "Activator_Base.h"

class BasicImageProcessView;

class Mipf_Plugin_BasicImageProcess_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_BasicImageProcess_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
private:
    BasicImageProcessView* m_pBasicImageProcessView;
};

#endif // Mipf_Plugin_BasicImageProcessActivator_h__