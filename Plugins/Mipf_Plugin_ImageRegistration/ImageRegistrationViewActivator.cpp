#include "ImageRegistrationViewActivator.h"
#include "ImageRegistrationView.h"


QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new ImageRegistrationView_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}


const char ImageRegistrationView_Activator_ID[] = "ImageRegistrationView_Activator";

ImageRegistrationView_Activator::ImageRegistrationView_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
}

bool ImageRegistrationView_Activator::Init()
{
    m_pView = new ImageRegistrationView(m_pMain);
    return true;
}

const char* ImageRegistrationView_Activator::GetID()
{
    return ImageRegistrationView_Activator_ID;
}

void ImageRegistrationView_Activator::Register(R* pR)
{
    m_pView->InitResource(pR);
}

void ImageRegistrationView_Activator::Contructed(R* pR)
{
    m_pView->ResourceContructed(pR);
}