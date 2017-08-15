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

  //  m_pDataManagerWidget = new DataManagerWidget(m_pMain);
  //  m_pDataManagerWidget->Init(NULL);

    m_pMultiViewsWidget = new MultiViewsWidget(m_pMain);
    m_pMultiViewsWidget->Init(NULL);

    m_VolumeVisualizationWidget = new VolumeVisualizationWidget(m_pMain);
    m_VolumeVisualizationWidget->Init(NULL);

    m_ImageNavigatorWidget = new ImageNavigatorWidget(m_pMain);
    m_ImageNavigatorWidget->Init(NULL);
  

    m_pMeasurementWidget = new MeasurementWidget(m_pMain);
    m_pMeasurementWidget->Init(NULL);

    m_StatusBarWidget = new StatusBarWidget();
    m_StatusBarWidget->Init();

 //  m_pImageStatisticsWidget = new ImageStatisticsWidget(m_pMain);
 //   m_pImageStatisticsWidget->Init(NULL);

    return true;
}

const char* MitkStdWidgets_Activator::GetID()
{
    return MitkStdWidgets_Activator_ID;
}

void MitkStdWidgets_Activator::Register(R* pR)
{
    //数据管理插件
   // m_pDataManagerWidget->InitResource(pR);
   // pR->registerCustomWidget("DataManagerWidget", m_pDataManagerWidget);
    REGISTER_CLASS("DataManagerWidget", DataManagerWidget);

    //多视图显示插件
    m_pMultiViewsWidget->InitResource(pR);
    pR->registerCustomWidget("MultiViewsWidget", m_pMultiViewsWidget);

    //体绘制插件
    m_VolumeVisualizationWidget->InitResource(pR);
    pR->registerCustomWidget("VolumeVisualizationWidget", m_VolumeVisualizationWidget);

    //图像浏览插件
    m_ImageNavigatorWidget->InitResource(pR);
    pR->registerCustomWidget("ImageNavigatorWidget", m_ImageNavigatorWidget);

    //测量插件
    m_pMeasurementWidget->InitResource(pR);
    pR->registerCustomWidget("MeasurementWidget", m_pMeasurementWidget);

    pR->registerCustomWidget("StatusBarWidget", m_StatusBarWidget);

  //  m_pImageStatisticsWidget->InitResource(pR);
  //  pR->registerCustomWidget("ImageStatisticsWidget", m_pImageStatisticsWidget);
}
