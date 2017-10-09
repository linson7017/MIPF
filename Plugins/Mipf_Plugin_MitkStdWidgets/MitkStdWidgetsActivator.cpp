#include "MitkStdWidgetsActivator.h"
#include "DataManagerWidget/DataManagerWidget.h"
#include "MultiViewsWidget/MultiViewsWidget.h"
#include "VolumeVisualizationWidget/VolumeVisualizationWidget.h"
#include "ImageNavigatorWidget/ImageNavigatorWidget.h"
#include "MeasurementWidget/MeasurementWidget.h"
#include "StatusWidget/StatusBarWidget.h"
#include "RenderWindow/RenderWindow.h"
#include "Properties/QmitkPropertyTreeView.h"
#include "StatusWidget/ProgressBarWidget.h"
//#include "MeasurementWidget/ImageStatisticsWidget.h"
#include "Res/R.h"
#include "Utils/QObjectFactory.h"
#include "Utils/PluginFactory.h"
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
    //���ݹ�����
    REGISTER_QOBJECT("DataManagerWidget", DataManagerWidget);

    //����ͼ��ʾ���
    REGISTER_QOBJECT("MultiViewsWidget", MultiViewsWidget);

    //����Ʋ��
    REGISTER_QOBJECT("VolumeVisualizationWidget", VolumeVisualizationWidget);

    //ͼ��������
    REGISTER_QOBJECT("ImageNavigatorWidget", ImageNavigatorWidget);

    //�������
    REGISTER_QOBJECT("MeasurementWidget", MeasurementWidget);

    //״̬��
    REGISTER_QOBJECT("StatusBarWidget", StatusBarWidget);

    REGISTER_QOBJECT("ProgressBarWidget", ProgressBarWidget);



    REGISTER_PLUGIN("RenderWindowWidget", RenderWindow);

    REGISTER_QOBJECT("PropertiesWidget", QmitkPropertyTreeView);



  //  m_pImageStatisticsWidget->InitResource(pR);
  //  pR->registerCustomWidget("ImageStatisticsWidget", m_pImageStatisticsWidget);
}
