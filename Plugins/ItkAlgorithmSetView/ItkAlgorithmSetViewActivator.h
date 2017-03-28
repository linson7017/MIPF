#ifndef ItkAlgorithmSetViewActivator_h__
#define ItkAlgorithmSetViewActivator_h__

#pragma once
#include "iqf_activator.h"

class ItkAlgorithmSetView;

class ItkAlgorithmSetView_Activator: public QF::IQF_Activator
{
public:
    ItkAlgorithmSetView_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
private:
    ItkAlgorithmSetView* m_pView;
    QF::IQF_Main* m_PMain;
};

#endif // ItkAlgorithmSetViewActivator_h__
