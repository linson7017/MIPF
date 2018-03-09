#ifndef Mipf_Plugin_UtilsActivator_h__
#define Mipf_Plugin_UtilsActivator_h__

#pragma once
#include "Activator_Base.h"

class LargestConnectedComponentView;
class MaskImageView;
class ImageHoleFillingView;
class SurfaceExtractView;

class Mipf_Plugin_Utils_Activator : public ActivatorBase
{
public:
    Mipf_Plugin_Utils_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register();
private:
    LargestConnectedComponentView* m_pLargestConnectedComponentView;
    MaskImageView* m_pMaskImageView;
    ImageHoleFillingView* m_pImageHoleFillingView;
    SurfaceExtractView* m_surfaceExtractView;
   
};

#endif // Mipf_Plugin_UtilsActivator_h__