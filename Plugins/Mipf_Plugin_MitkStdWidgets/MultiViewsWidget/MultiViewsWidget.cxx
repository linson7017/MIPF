#include "MultiViewsWidget.h"

#include "QmitkStdMultiWidget.h"
#include "mitkRenderingManager.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkPointSet.h"
#include "mitkPointSetDataInteractor.h"
#include "mitkImageAccessByItk.h"
#include "mitkRenderingManager.h"
#include "mitkDataStorage.h"

#include <mitkIOUtil.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QPushButton>
#include <QVBoxLayout>

#include <MitkMain/IQF_MitkDataManager.h>
#include <MitkMain/IQF_MitkRenderWindow.h>

#include "iqf_main.h"
#include "Utils/variant.h"
#include "Res/R.h"

#include "vtkAxesActor.h"
#include "vtkOrientationMarkerWidget.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"

//##Documentation
//## @brief As MultiViews, but with QmitkStdMultiWidget as widget

MultiViewsWidget::MultiViewsWidget():MitkPluginView(),m_bInited(false), m_multiWidget(nullptr)
{
}

void MultiViewsWidget::CreateView()
{
    Init(this);
}

void MultiViewsWidget::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "MITK_MESSAGE_MULTIVIEWS_CHANGELAYOUT") == 0)
    {
        ChangeLayout(variant::GetVariant(*(VarientMap*)pValue, "parameterIndex").getInt());
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_MULTIVIEWS_RESET") == 0)
    {
        ResetView();
    }
}

void MultiViewsWidget::SetupWidgets()
{
    //*************************************************************************
    // Part I: Create windows and pass the tree to it
    //*************************************************************************
    // Create toplevel widget with vertical layout
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setMargin(0);
    vlayout->setSpacing(0);
    // Create viewParent widget with horizontal layout
    QWidget *viewParent = new QWidget(this);
    vlayout->addWidget(viewParent);
    QHBoxLayout *hlayout = new QHBoxLayout(viewParent);
    hlayout->setMargin(0);
    //*************************************************************************
    // Part Ia: create and initialize QmitkStdMultiWidget
    //*************************************************************************
    m_multiWidget = new QmitkStdMultiWidget(viewParent);
    hlayout->addWidget(m_multiWidget);

    if (GetDataStorage())
    {
        // Tell the multiWidget which DataStorage to render
        m_multiWidget->SetDataStorage(GetDataStorage());
        // Initialize views as axial, sagittal, coronar (from
        // top-left to bottom)
        mitk::TimeGeometry::Pointer geo = GetDataStorage()->ComputeBoundingGeometry3D(GetDataStorage()->GetAll());
        mitk::RenderingManager::GetInstance()->InitializeViews(geo);
    }

    // Initialize bottom-right view as 3D view
    m_multiWidget->GetRenderWindow4()->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

    // Enable standard handler for levelwindow-slider
    m_multiWidget->EnableStandardLevelWindow();

    // Add the displayed views to the DataStorage to see their positions in 2D and 3D
    m_multiWidget->AddDisplayPlaneSubTree();
    m_multiWidget->AddPlanesToDataStorage();
    bool widgetPlanesVisible = true;
    if (HasAttribute("widgetsVisible"))
    {
        widgetPlanesVisible = QString(GetAttribute("widgetsVisible")).compare("true", Qt::CaseInsensitive) == 0;
    }
    m_multiWidget->SetWidgetPlanesVisibility(widgetPlanesVisible);

    m_multiWidget->DisableDepartmentLogo();
    if (HasAttribute("logo"))
    {
        MITK_INFO << R::Instance()->getImageResourceUrl(GetAttribute("logo"));
        m_multiWidget->SetDepartmentLogoPath(R::Instance()->getImageResourceUrl(GetAttribute("logo")).c_str());
        m_multiWidget->EnableDepartmentLogo();
    }
    

    //set cross hair
    int crosshairgapsize = GetMitkReferenceInterface()->GetInt("Crosshair-Gap-Size", 1);
    if (HasAttribute("Crosshair-Gap-Size"))
    {
        crosshairgapsize = QString(GetAttribute("Crosshair-Gap-Size")).toInt();
    }
    m_multiWidget->GetWidgetPlane1()->SetIntProperty("Crosshair.Gap Size", crosshairgapsize);
    m_multiWidget->GetWidgetPlane2()->SetIntProperty("Crosshair.Gap Size", crosshairgapsize);
    m_multiWidget->GetWidgetPlane3()->SetIntProperty("Crosshair.Gap Size", crosshairgapsize);

}


void MultiViewsWidget::Init(QWidget* parent)
{
    m_pMain->Attach(this);
    // setup the widgets as in the previous steps, but with an additional
    // QVBox for a button to start the segmentation
    SetupWidgets();
    IQF_MitkRenderWindow* pRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
    if (pRenderWindow)
    {
        pRenderWindow->SetMitkStdMultiWidget(m_multiWidget,GetAttribute("id"));
        m_pMain->SendMessageQf(MITK_MESSAGE_MULTIWIDGET_INITIALIZED, 0, pRenderWindow);
    }
}

void MultiViewsWidget::ChangeLayout(int index)
{
    switch (index)
    {
        case 0:
        {
            m_multiWidget->changeLayoutToDefault();
            break;
        }
        case 1:
        {
            m_multiWidget->changeLayoutToWidget1();
            break;
        }
        case 2:
        {
            m_multiWidget->changeLayoutToWidget2();
            break;
        }
        case 3:
        {
            m_multiWidget->changeLayoutToWidget3();
            break;
        }
        case 4:
        {
            m_multiWidget->changeLayoutToBig3D();
            break;
        }
        default:
            break;
    }
}

void MultiViewsWidget::ResetView()
{
    m_multiWidget->ResetCrosshair();
}

void MultiViewsWidget::hideEvent(QHideEvent *e)
{
	m_pMain->SendMessageQf(MITK_MESSAGE_MULTIWIDGET_HIDE, 0, m_multiWidget);
	QWidget::hideEvent(e);
}

void MultiViewsWidget::showEvent(QShowEvent *e)
{
	m_pMain->SendMessageQf(MITK_MESSAGE_MULTIWIDGET_SHOW, 0, m_multiWidget);
	QWidget::showEvent(e);
}

void MultiViewsWidget::closeEvent(QCloseEvent *e)
{
	m_pMain->SendMessageQf(MITK_MESSAGE_MULTIWIDGET_CLOSE, 0, m_multiWidget);
	QWidget::closeEvent(e);
}