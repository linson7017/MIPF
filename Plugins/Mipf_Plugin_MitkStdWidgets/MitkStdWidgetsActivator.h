#pragma once
#include "Activator_Base.h"

class DataManagerWidget;
class MultiViewsWidget;
class VolumeVisualizationWidget;
class ImageNavigatorWidget;
class MeasurementWidget;
class ImageStatisticsWidget;
class StatusBarWidget;

class MitkStdWidgets_Activator : public ActivatorBase
{
public:
    MitkStdWidgets_Activator(QF::IQF_Main* pMain);

    bool Init();
    const char* GetID();
    void Register(R* pR);
private:
    DataManagerWidget* m_pDataManagerWidget;
    MultiViewsWidget* m_pMultiViewsWidget;
    VolumeVisualizationWidget* m_VolumeVisualizationWidget;
    ImageNavigatorWidget* m_ImageNavigatorWidget;
    MeasurementWidget* m_pMeasurementWidget;
    ImageStatisticsWidget* m_pImageStatisticsWidget;

    StatusBarWidget* m_StatusBarWidget;
};