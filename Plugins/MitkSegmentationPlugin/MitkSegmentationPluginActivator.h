#ifndef MitkSegmentationPluginActivator_h__
#define MitkSegmentationPluginActivator_h__

#pragma once
#include "iqf_activator.h"

class MitkSegmentation;

class MitkSegmentationPlugin_Activator : public QF::IQF_Activator
{
public:
    MitkSegmentationPlugin_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
private:
    MitkSegmentation* m_pMitkSegmentation;
    QF::IQF_Main* m_PMain;
};

#endif // MitkSegmentationPluginActivator_h__