#ifndef Mipf_Plugin_DicomActivator_h__
#define Mipf_Plugin_DicomActivator_h__

#pragma once
#include "Activator_Base.h"

class Mipf_Plugin_Dicom_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_Dicom_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register();
};

#endif // Mipf_Plugin_DicomActivator_h__