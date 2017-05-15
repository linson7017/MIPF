#ifndef ImageRegistrationViewActivator_h__
#define ImageRegistrationViewActivator_h__

#pragma once
#include "Activator_Base.h"

class ImageRegistrationView;

class ImageRegistrationView_Activator: public ActivatorBase
{
public:
    ImageRegistrationView_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
    void Contructed(R* pR);
private:
    ImageRegistrationView* m_pView;
};

#endif // ImageRegistrationViewActivator_h__
