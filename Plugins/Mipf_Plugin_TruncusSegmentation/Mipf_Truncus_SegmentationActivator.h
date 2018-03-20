#ifndef Mipf_Truncus_SegmentationActivator_h__
#define Mipf_Truncus_SegmentationActivator_h__

#pragma once
#include "Activator_Base.h"

class Mipf_Truncus_Segmentation_Activator : public ActivatorBase
{
public:
    Mipf_Truncus_Segmentation_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register();
};

#endif // Mipf_Truncus_SegmentationActivator_h__