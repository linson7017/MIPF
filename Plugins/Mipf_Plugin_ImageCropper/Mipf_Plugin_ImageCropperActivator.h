#ifndef Mipf_Plugin_ImageCropperActivator_h__
#define Mipf_Plugin_ImageCropperActivator_h__

#pragma once
#include "iqf_activator.h"
#include "Activator_Base.h"

class ImageCropperView;

class Mipf_Plugin_ImageCropper_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_ImageCropper_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register();
private:
    ImageCropperView* m_pImageCropperView;
};

#endif // Mipf_Plugin_ImageCropperActivator_h__