#ifndef Mipf_Plugin_MapperTestActivator_h__
#define Mipf_Plugin_MapperTestActivator_h__

#pragma once
#include "Activator_Base.h"

class MapperTestView;

class Mipf_Plugin_MapperTest_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_MapperTest_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register();
private:

};

#endif // Mipf_Plugin_MapperTestActivator_h__