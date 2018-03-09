#include "Mipf_Plugin_ImageCropperActivator.h"
#include "ImageCropperView.h"
#include "Res/R.h"
#include <mitkBoundingShapeObjectFactory.h>

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Plugin_ImageCropper_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Plugin_ImageCropper_Activator_Activator_ID[] = "Mipf_Plugin_ImageCropper_Activator_Activator_ID";

Mipf_Plugin_ImageCropper_Activator::Mipf_Plugin_ImageCropper_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
}

bool Mipf_Plugin_ImageCropper_Activator::Init()
{
    mitk::RegisterBoundingShapeObjectFactory();
    m_pImageCropperView = new ImageCropperView(m_pMain); 
    return true; 
}

const char* Mipf_Plugin_ImageCropper_Activator::GetID()
{
    return Mipf_Plugin_ImageCropper_Activator_Activator_ID; 
}

void Mipf_Plugin_ImageCropper_Activator::Register()
{
    m_pImageCropperView->InitResource(); 
    R::Instance()->registerCustomWidget("ImageCropperWidget", m_pImageCropperView);
}