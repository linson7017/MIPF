#include "MitkStdWidgetsActivator.h"
#include "DataManagerWidget/DataManagerWidget.h"
#include "MultiViewsWidget/MultiViewsWidget.h"
#include "VolumeVisualizationWidget/VolumeVisualizationWidget.h"
#include "ImageNavigatorWidget/ImageNavigatorWidget.h"
#include "MeasurementWidget/MeasurementWidget.h"
#include "StatusWidget/StatusBarWidget.h"
//#include "MeasurementWidget/ImageStatisticsWidget.h"
#include "Res/R.h"
#include "Utils/QObjectFactory.h"
#include "QmitkWidgetsRegister.h"

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new MitkStdWidgets_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}


const char MitkStdWidgets_Activator_ID[] = "MitkStdWidgets_Activator_ID";

MitkStdWidgets_Activator::MitkStdWidgets_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
}

bool MitkStdWidgets_Activator::Init()
{
    RegisterQmitkWidgets();

    return true;
}

const char* MitkStdWidgets_Activator::GetID()
{
    return MitkStdWidgets_Activator_ID;
}

void MitkStdWidgets_Activator::Register(R* pR)
{
    //数据管理插件
    REGISTER_CLASS("DataManagerWidget", DataManagerWidget);

    //多视图显示插件
    REGISTER_CLASS("MultiViewsWidget", MultiViewsWidget);

    //体绘制插件
    REGISTER_CLASS("VolumeVisualizationWidget", VolumeVisualizationWidget);

    //图像浏览插件
    REGISTER_CLASS("ImageNavigatorWidget", ImageNavigatorWidget);

    //测量插件
    REGISTER_CLASS("MeasurementWidget", MeasurementWidget);

    //状态条
    REGISTER_CLASS("StatusBarWidget", StatusBarWidget);

  //  m_pImageStatisticsWidget->InitResource(pR);
  //  pR->registerCustomWidget("ImageStatisticsWidget", m_pImageStatisticsWidget);
}
