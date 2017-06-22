#include "Mipf_Plugin_UtilsActivator.h"
#include "LargestConnectedComponentView.h"
#include "MaskImageView.h"
#include "ImageHoleFillingView.h"
#include "SurfaceExtractView.h"
#include "Res/R.h"
#include "Utils/QObjectFactory.h"
#include "QmitkLabelSetWidget.h"

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Plugin_Utils_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Plugin_Utils_Activator_ID[] = "Mipf_Plugin_Utils_Activator_ID";

Mipf_Plugin_Utils_Activator::Mipf_Plugin_Utils_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
   
}

bool Mipf_Plugin_Utils_Activator::Init()
{
    m_pLargestConnectedComponentView = new LargestConnectedComponentView(m_pMain);
    m_pMaskImageView = new MaskImageView(m_pMain);
    m_pImageHoleFillingView = new ImageHoleFillingView(m_pMain);
    m_surfaceExtractView = new SurfaceExtractView(m_pMain);


    return true; 
}

const char* Mipf_Plugin_Utils_Activator::GetID()
{
    return Mipf_Plugin_Utils_Activator_ID;
}

void Mipf_Plugin_Utils_Activator::Register(R* pR)
{
    m_pLargestConnectedComponentView->InitResource(pR);
    m_pMaskImageView->InitResource(pR);
    m_pImageHoleFillingView->InitResource(pR);
    m_surfaceExtractView->InitResource(pR);
   // pR->registerCustomWidget("LargestConnectedComponentWidget", m_pLargestConnectedComponentView); 
}

void Mipf_Plugin_Utils_Activator::Constructed(R* pR)
{
    m_pLargestConnectedComponentView->Constructed(pR);
    m_pMaskImageView->Constructed(pR);
    m_pImageHoleFillingView->Constructed(pR);
    m_surfaceExtractView->Constructed(pR);
}