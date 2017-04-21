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
#include <mitkMain/IQF_MitkRenderWindow.h>

#include "iqf_main.h"
#include "Utils/variant.h"

//##Documentation
//## @brief As MultiViews, but with QmitkStdMultiWidget as widget

MultiViewsWidget::MultiViewsWidget(QF::IQF_Main* pMain):MitkPluginView(pMain),m_bInited(false)
{
    m_pMain->Attach(this);
    m_DataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr("QF_MitkMain_DataManager");
    m_DataManager->Init();
}

void MultiViewsWidget::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "MITK_COMMAND_MULTIVIEWS_CHANGELAYOUT") == 0)
    {
        VarientMap vmp = *(VarientMap*)pValue;
        variant v = variant::GetVariant(vmp, "parameterIndex");
        int index = v.getInt();
        ChangeLayout(index);
    }
    else if (strcmp(szMessage, "MITK_COMMAND_MULTIVIEWS_RESET") == 0)
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
    vlayout->setSpacing(2);
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

    if (m_DataManager->GetDataStorage())
    {
        // Tell the multiWidget which DataStorage to render
        m_multiWidget->SetDataStorage(m_DataManager->GetDataStorage());
        // Initialize views as axial, sagittal, coronar (from
        // top-left to bottom)
        mitk::TimeGeometry::Pointer geo = m_DataManager->GetDataStorage()->ComputeBoundingGeometry3D(m_DataManager->GetDataStorage()->GetAll());
        mitk::RenderingManager::GetInstance()->InitializeViews(geo);
    }

    // Initialize bottom-right view as 3D view
    m_multiWidget->GetRenderWindow4()->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

    // Enable standard handler for levelwindow-slider
    m_multiWidget->EnableStandardLevelWindow();

    // Add the displayed views to the DataStorage to see their positions in 2D and 3D
    m_multiWidget->AddDisplayPlaneSubTree();
    m_multiWidget->AddPlanesToDataStorage();
    m_multiWidget->SetWidgetPlanesVisibility(true);

}


void MultiViewsWidget::Init(QWidget* parent)
{
    // setup the widgets as in the previous steps, but with an additional
    // QVBox for a button to start the segmentation
    SetupWidgets();
    IQF_MitkRenderWindow* pRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
    if (pRenderWindow)
    {
        pRenderWindow->SetMitkStdMultiWidget(m_multiWidget);
        m_pMain->SendMessageQf(MITK_MESSAGE_MULTIWIDGET_INIT, 0, pRenderWindow);
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