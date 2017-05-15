#ifndef MitkSegmentationPluginActivator_h__
#define MitkSegmentationPluginActivator_h__

#pragma once
#include "Activator_Base.h"

class MitkSegmentation;

class MitkSegmentationPlugin_Activator : public ActivatorBase
{
public:
    MitkSegmentationPlugin_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
private:
    MitkSegmentation* m_pMitkSegmentation;
};

#endif // MitkSegmentationPluginActivator_h__