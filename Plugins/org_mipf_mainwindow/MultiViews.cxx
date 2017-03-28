#include "MultiViews.h"

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

#include <mitkIOUtil.h>


#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QPushButton>
#include <QVBoxLayout>
//##Documentation
//## @brief As MultiViews, but with QmitkStdMultiWidget as widget

MultiViews::MultiViews(QWidget *parent):m_bInited(false)
{
}

void MultiViews::SetupWidgets()
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

    if (m_bInited)
    {
        // Tell the multiWidget which DataStorage to render
        m_multiWidget->SetDataStorage(m_DataStorage);
        // Initialize views as axial, sagittal, coronar (from
        // top-left to bottom)
        mitk::TimeGeometry::Pointer geo = m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll());
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

void MultiViews::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
    m_DataStorage = dataStorage;
    if (m_DataStorage)
    {
        m_bInited = true;
    }
}

void MultiViews::Initialize()
{
    // setup the widgets as in the previous steps, but with an additional
    // QVBox for a button to start the segmentation
    this->SetupWidgets();
}

void MultiViews::ChangeLayout(int index)
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

void MultiViews::ResetView()
{
    m_multiWidget->ResetCrosshair();
}