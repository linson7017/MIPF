#ifndef Mipf_Plugin_ImageBatchProcessingActivator_h__
#define Mipf_Plugin_ImageBatchProcessingActivator_h__

#pragma once
#include "Activator_Base.h"

class Mipf_Plugin_ImageBatchProcessing_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_ImageBatchProcessing_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
};

#endif // Mipf_Plugin_ImageBatchProcessingActivator_h__