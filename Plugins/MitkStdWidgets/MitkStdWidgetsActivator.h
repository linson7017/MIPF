#pragma once
#include "iqf_activator.h"

class DataManagerWidget;
class MultiViewsWidget;
class VolumeVisualizationWidget;
class ImageNavigatorWidget;
class MeasurementWidget;
class ImageStatisticsWidget;

class MitkStdWidgets_Activator : public QF::IQF_Activator
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


    QF::IQF_Main* m_pMain;
};