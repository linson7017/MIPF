#ifndef ItkAlgorithmSetViewActivator_h__
#define ItkAlgorithmSetViewActivator_h__

#pragma once
#include "Activator_Base.h"

class ItkAlgorithmSetView;

class ItkAlgorithmSetView_Activator: public ActivatorBase
{
public:
    ItkAlgorithmSetView_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
private:
    ItkAlgorithmSetView* m_pView;
};

#endif // ItkAlgorithmSetViewActivator_h__
