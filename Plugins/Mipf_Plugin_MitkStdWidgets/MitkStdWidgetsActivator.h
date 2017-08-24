#pragma once
#include "Activator_Base.h"

class MitkStdWidgets_Activator : public ActivatorBase
{
public:
    MitkStdWidgets_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
private:
};