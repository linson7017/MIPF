#ifndef Activator_Base_H
#define Activator_Base_H

#include "iqf_activator.h"

class ActivatorBase :public QF::IQF_Activator
{
public:
    ActivatorBase(QF::IQF_Main* pMain) :m_pMain(pMain){}
    virtual bool Init() { return true; }
    virtual const char* GetID() { return ""; }
    virtual void Register() {}
    virtual void Constructed() {}
protected:
    QF::IQF_Main* m_pMain;
};

#endif // Activator_Base