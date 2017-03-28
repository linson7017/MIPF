#ifndef ImageRegistrationViewActivator_h__
#define ImageRegistrationViewActivator_h__

#pragma once
#include "iqf_activator.h"

class ImageRegistrationView;

class ImageRegistrationView_Activator: public QF::IQF_Activator
{
public:
    ImageRegistrationView_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
private:
    ImageRegistrationView* m_pView;
    QF::IQF_Main* m_PMain;
};

#endif // ImageRegistrationViewActivator_h__
