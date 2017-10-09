#include "DataManagerWidget.h"
//## mitk
//#include "mitkDataStorageEditorInput.h"
//#include "mitkIDataStorageReference.h"
#include "mitkNodePredicateDataType.h"
#include "mitkCoreObjectFactory.h"
#include "mitkColorProperty.h"
#include "mitkCommon.h"
#include "mitkNodePredicateData.h"
#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateOr.h"
#include "mitkNodePredicateProperty.h"
#include "mitkEnumerationProperty.h"
#include "mitkLookupTableProperty.h"
#include "mitkProperties.h"
#include <mitkNodePredicateAnd.h>
#include <mitkITKImageImport.h>
#include "mitkRenderWindow.h"
#include "mitkRenderingManager.h"
#include <mitkRenderingModeProperty.h>
//#include <QmitkDataManagerItemDelegate.h>
//#include <mitkIDataStorageService.h>
//#include <mitkIRenderingManager.h>
#include <mitkImageCast.h>
//## Qmitk
//#include <QmitkDnDFrameWidget.h>
#include <QmitkIOUtil.h>
#include <QmitkDataStorageTreeModel.h>
#include <QmitkCustomVariants.h>
//#include <QmitkFileSaveAction.h>
#include <QmitkDataStorageFilterProxyModel.h>
#include <QmitkNumberPropertySlider.h>
#include <QmitkStdMultiWidget.h>
//#include "src/internal/QmitkNodeTableViewKeyFilter.h"
//#include "src/internal/QmitkInfoDialog.h"
//#include "src/internal/QmitkDataManagerItemDelegate.h"

//# Toolkit Includes
#include <QTableView>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QListView>
#include <QMenu>
#include <QAction>
#include <QComboBox>
#include <QApplication>
#include <QCursor>
#include <QHeaderView>
#include <QTreeView>
#include <QWidgetAction>
#include <QSplitter>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QKeyEvent>
#include <QColor>
#include <QColorDialog>
#include <QSizePolicy>
#include <QSortFilterProxyModel>
#include <QSignalMapper>
#include <QItemSelection>

//qf
#include <MitkMain/IQF_MitkDataManager.h>
#include <MitkMain/IQF_MitkRenderWindow.h>
#include <MitkMain/IQF_MitkReference.h>
#include "DataManagerItemDelegate.h"
#include "iqf_main.h"
#include <Res/R.h>


DataManagerWidget::DataManagerWidget() :MitkPluginView(),
m_CurrentRowCount(0)
{
}

DataManagerWidget::~DataManagerWidget()
{
}

void DataManagerWidget::SetDataStorage(IQF_MitkDataManager* dataStorage)
{
    m_DataManager = dataStorage;
}

void DataManagerWidget::CreateView()
{
    m_Parent = this;
    m_pMain->Attach(this);
    m_DataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr("QF_MitkMain_DataManager");

    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);
    
    //# GUI
    m_NodeTreeModel = new QmitkDataStorageTreeModel(m_DataManager->GetDataStorage());
    //m_NodeTreeModel->setParent(m_Parent);
    m_NodeTreeModel->SetPlaceNewNodesOnTop(true);
    m_NodeTreeModel->SetAllowHierarchyChange(false);

    m_SurfaceDecimation = false;
    // Prepare filters
    m_HelperObjectFilterPredicate = mitk::NodePredicateOr::New(
        mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)),
        mitk::NodePredicateProperty::New("hidden object", mitk::BoolProperty::New(true)));
    m_NodeWithNoDataFilterPredicate = mitk::NodePredicateData::New(0);

    m_FilterModel = new QmitkDataStorageFilterProxyModel();
    m_FilterModel->setSourceModel(m_NodeTreeModel);
    m_FilterModel->AddFilterPredicate(m_HelperObjectFilterPredicate);
    m_FilterModel->AddFilterPredicate(m_NodeWithNoDataFilterPredicate);

    //# Tree View (experimental)
    m_NodeTreeView = new QTreeView;
    m_NodeTreeView->setHeaderHidden(true);
    m_NodeTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_NodeTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    //  m_NodeTreeView->setAlternatingRowColors(true);
    m_NodeTreeView->setDragEnabled(true);
    m_NodeTreeView->setDropIndicatorShown(true);
    m_NodeTreeView->setAcceptDrops(true);
    m_NodeTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_NodeTreeView->setModel(m_FilterModel);
    m_NodeTreeView->setTextElideMode(Qt::ElideMiddle);
    //m_NodeTreeView->installEventFilter(new QmitkNodeTableViewKeyFilter(this));
    layout->addWidget(m_NodeTreeView);


    m_ItemDelegate = new DataManagerItemDelegate(m_NodeTreeView);
    m_NodeTreeView->setItemDelegate(m_ItemDelegate);

    QObject::connect(m_NodeTreeView, SIGNAL(customContextMenuRequested(const QPoint&))
        , this, SLOT(NodeTableViewContextMenuRequested(const QPoint&)));
    QObject::connect(m_NodeTreeModel, SIGNAL(rowsInserted(const QModelIndex&, int, int))
        , this, SLOT(NodeTreeViewRowsInserted(const QModelIndex&, int, int)));
    QObject::connect(m_NodeTreeModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int))
        , this, SLOT(NodeTreeViewRowsRemoved(const QModelIndex&, int, int)));
    QObject::connect(m_NodeTreeModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &))
        , this, SLOT(NodeTreeViewDataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)));
    connect(m_NodeTreeView->selectionModel()
        , SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &))
        , this
        , SLOT(NodeSelectionChanged(const QItemSelection &, const QItemSelection &)));

    //# m_NodeMenu
    m_NodeMenu = new QMenu(m_NodeTreeView);

    QmitkNodeDescriptor* unknownDataNodeDescriptor =
        QmitkNodeDescriptorManager::GetInstance()->GetUnknownDataNodeDescriptor();

    QmitkNodeDescriptor* imageDataNodeDescriptor =
        QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Image");

    auto multiComponentImageDataNodeDescriptor =
        QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("MultiComponentImage");

    QmitkNodeDescriptor* diffusionImageDataNodeDescriptor =
        QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("DiffusionImage");

    QmitkNodeDescriptor* surfaceDataNodeDescriptor =
        QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Surface");

    QAction* globalReinitAction = new QAction(QIcon(R::Instance()->getImageResourceUrl("@icon/Refresh_48.png")), "Global Reinit", this);
    QObject::connect(globalReinitAction, SIGNAL(triggered(bool)), this, SLOT(GlobalReinit(bool)));
    unknownDataNodeDescriptor->AddAction(globalReinitAction);
    m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor, globalReinitAction));

    QAction* saveAction = new QAction(QIcon(R::Instance()->getImageResourceUrl("@icon/Save_48.png")), "Save File", this); 
    QObject::connect(saveAction, SIGNAL(triggered(bool)), this, SLOT(SaveSelectedNodes(bool)));
    unknownDataNodeDescriptor->AddAction(saveAction);
    m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor, saveAction));

    QAction* removeAction = new QAction(QIcon(R::Instance()->getImageResourceUrl("@icon/Remove_48.png")), "Remove", this);
    QObject::connect(removeAction, SIGNAL(triggered(bool)), this, SLOT(RemoveSelectedNodes(bool)));
    unknownDataNodeDescriptor->AddAction(removeAction);
    m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor, removeAction));

    QAction* reinitAction = new QAction(QIcon(R::Instance()->getImageResourceUrl("@icon/Refresh_48.png")), "Reinit", this);
    QObject::connect(reinitAction, SIGNAL(triggered(bool)), this, SLOT(ReinitSelectedNodes(bool)));
    unknownDataNodeDescriptor->AddAction(reinitAction);
    m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor, reinitAction));


    //opacity
    m_OpacitySlider = new QSlider;
    m_OpacitySlider->setMinimum(0);
    m_OpacitySlider->setMaximum(100);
    m_OpacitySlider->setOrientation(Qt::Horizontal);
    QObject::connect(m_OpacitySlider, SIGNAL(valueChanged(int))
        , this, SLOT(OpacityChanged(int)));

    QLabel* _OpacityLabel = new QLabel("Opacity: ");
    QHBoxLayout* _OpacityWidgetLayout = new QHBoxLayout;
   // _OpacityWidgetLayout->setContentsMargins(4, 4, 4, 4);
    _OpacityWidgetLayout->addWidget(_OpacityLabel);
    _OpacityWidgetLayout->addWidget(m_OpacitySlider);
    QWidget* _OpacityWidget = new QWidget;
    _OpacityWidget->setLayout(_OpacityWidgetLayout);
    _OpacityWidget->setStyleSheet(m_pR->getStyleResource("mainstyle")+QString("QLabel {font-size:12px;font-weight:100}" ) );

    QWidgetAction* opacityAction = new QWidgetAction(this);
    opacityAction->setDefaultWidget(_OpacityWidget);
    QObject::connect(opacityAction, SIGNAL(changed())
        , this, SLOT(OpacityActionChanged()));
    unknownDataNodeDescriptor->AddAction(opacityAction, false);
    m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor, opacityAction));


    m_ColorButton = new QPushButton;
    m_ColorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    //m_ColorButton->setText("Change color");
    QObject::connect(m_ColorButton, SIGNAL(clicked())
        , this, SLOT(ColorChanged()));

    QLabel* _ColorLabel = new QLabel("Color: ");
    _ColorLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    QHBoxLayout* _ColorWidgetLayout = new QHBoxLayout;
   // _ColorWidgetLayout->setContentsMargins(4, 4, 4, 4);
    _ColorWidgetLayout->addWidget(_ColorLabel);
    _ColorWidgetLayout->addWidget(m_ColorButton);
    QWidget* _ColorWidget = new QWidget;
    _ColorWidget->setLayout(_ColorWidgetLayout);
    _ColorWidget->setStyleSheet(m_pR->getStyleResource("mainstyle")+ QString("QLabel {font-size:12px;font-weight:100}"));

    QWidgetAction* colorAction = new QWidgetAction(this);
    colorAction->setDefaultWidget(_ColorWidget);
    QObject::connect(colorAction, SIGNAL(changed())
        , this, SLOT(ColorActionChanged()));
    unknownDataNodeDescriptor->AddAction(colorAction, false);
    m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor, colorAction));

    m_ComponentSlider = new QmitkNumberPropertySlider;
    m_ComponentSlider->setOrientation(Qt::Horizontal);

    QLabel* _ComponentLabel = new QLabel("Component: ");
    QHBoxLayout* _ComponentWidgetLayout = new QHBoxLayout;
   // _ComponentWidgetLayout->setContentsMargins(4, 4, 4, 4);
    _ComponentWidgetLayout->addWidget(_ComponentLabel);
    _ComponentWidgetLayout->addWidget(m_ComponentSlider);
    QLabel* _ComponentValueLabel = new QLabel();
    _ComponentWidgetLayout->addWidget(_ComponentValueLabel);
    connect(m_ComponentSlider, SIGNAL(valueChanged(int)), _ComponentValueLabel, SLOT(setNum(int)));
    QWidget* _ComponentWidget = new QWidget;
    _ComponentWidget->setLayout(_ComponentWidgetLayout);

    QWidgetAction* componentAction = new QWidgetAction(this);
    componentAction->setDefaultWidget(_ComponentWidget);
    QObject::connect(componentAction, SIGNAL(changed())
        , this, SLOT(ComponentActionChanged()));
    multiComponentImageDataNodeDescriptor->AddAction(componentAction, false);
    m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(multiComponentImageDataNodeDescriptor, componentAction));
    if (diffusionImageDataNodeDescriptor != NULL)
    {
        diffusionImageDataNodeDescriptor->AddAction(componentAction, false);
        m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(diffusionImageDataNodeDescriptor, componentAction));
    }

    m_TextureInterpolation = new QAction("Texture Interpolation", this);
    m_TextureInterpolation->setCheckable(true);
    QObject::connect(m_TextureInterpolation, SIGNAL(changed())
        , this, SLOT(TextureInterpolationChanged()));
    QObject::connect(m_TextureInterpolation, SIGNAL(toggled(bool))
        , this, SLOT(TextureInterpolationToggled(bool)));
    imageDataNodeDescriptor->AddAction(m_TextureInterpolation, false);
    m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(imageDataNodeDescriptor, m_TextureInterpolation));
    if (diffusionImageDataNodeDescriptor != NULL)
    {
        diffusionImageDataNodeDescriptor->AddAction(m_TextureInterpolation, false);
        m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(diffusionImageDataNodeDescriptor, m_TextureInterpolation));
    }

    m_ColormapAction = new QAction("Colormap", this);
    m_ColormapAction->setMenu(new QMenu);
    QObject::connect(m_ColormapAction->menu(), SIGNAL(aboutToShow())
        , this, SLOT(ColormapMenuAboutToShow()));
    imageDataNodeDescriptor->AddAction(m_ColormapAction, false);
    m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(imageDataNodeDescriptor, m_ColormapAction));
    if (diffusionImageDataNodeDescriptor != NULL)
    {
        diffusionImageDataNodeDescriptor->AddAction(m_ColormapAction, false);
        m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(diffusionImageDataNodeDescriptor, m_ColormapAction));
    }

    m_SurfaceRepresentation = new QAction("Surface Representation", this);
    m_SurfaceRepresentation->setMenu(new QMenu(m_NodeTreeView));
    QObject::connect(m_SurfaceRepresentation->menu(), SIGNAL(aboutToShow())
        , this, SLOT(SurfaceRepresentationMenuAboutToShow()));
    surfaceDataNodeDescriptor->AddAction(m_SurfaceRepresentation, false);
    m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(surfaceDataNodeDescriptor, m_SurfaceRepresentation));

    QAction* showOnlySelectedNodes
        = new QAction(QIcon(R::Instance()->getImageResourceUrl("@icon/ShowSelectedNode_48.png"))
            , "Show only selected nodes", this);
    QObject::connect(showOnlySelectedNodes, SIGNAL(triggered(bool))
        , this, SLOT(ShowOnlySelectedNodes(bool)));
    unknownDataNodeDescriptor->AddAction(showOnlySelectedNodes);
    m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor, showOnlySelectedNodes));

    QAction* toggleSelectedVisibility
        = new QAction(QIcon(R::Instance()->getImageResourceUrl("@icon/InvertShowSelectedNode_48.png"))
            , "Toggle visibility", this);
    QObject::connect(toggleSelectedVisibility, SIGNAL(triggered(bool))
        , this, SLOT(ToggleVisibilityOfSelectedNodes(bool)));
    unknownDataNodeDescriptor->AddAction(toggleSelectedVisibility);
    m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor, toggleSelectedVisibility));

    QAction* actionShowInfoDialog
        = new QAction(QIcon(R::Instance()->getImageResourceUrl("@icon/ShowDataInfo_48.png"))
            , "Details...", this);
    QObject::connect(actionShowInfoDialog, SIGNAL(triggered(bool))
        , this, SLOT(ShowInfoDialogForSelectedNodes(bool)));
    unknownDataNodeDescriptor->AddAction(actionShowInfoDialog);
    m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor, actionShowInfoDialog));



}


void DataManagerWidget::Init(QWidget* parent)
{
    m_Parent = NULL;
}

void DataManagerWidget::NodeSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    QList<mitk::DataNode::Pointer> nodes = m_NodeTreeModel->GetNodeSet();

    foreach(mitk::DataNode::Pointer node, nodes)
    {
        if (node.IsNotNull())
            node->SetBoolProperty("selected", false);
    }
    nodes.clear();

    QModelIndexList indexesOfSelectedRowsFiltered = m_NodeTreeView->selectionModel()->selectedRows();
    QModelIndexList indexesOfSelectedRows;
    for (int i = 0; i < indexesOfSelectedRowsFiltered.size(); ++i)
    {
        indexesOfSelectedRows.push_back(m_FilterModel->mapToSource(indexesOfSelectedRowsFiltered[i]));
    }
    if (indexesOfSelectedRows.size() < 1)
    {
        return;
    }
    std::vector<mitk::DataNode::Pointer> selectedNodes;

    mitk::DataNode::Pointer node = 0;
    for (QModelIndexList::iterator it = indexesOfSelectedRows.begin()
        ; it != indexesOfSelectedRows.end(); it++)
    {
        node = m_NodeTreeModel->GetNode(*it);
        if (node.IsNotNull())
        {
            node->SetBoolProperty("selected", true);
            selectedNodes.push_back(node);
        }
    }

    m_DataManager->SetSelectedNode(selectedNodes);

    if (selectedNodes.size()>0)
    {
        m_pMain->SendMessageQf(MITK_MESSAGE_SELECTION_CHANGED, selectedNodes.size(), m_DataManager);
    }
}

void DataManagerWidget::NodeTreeViewDataChanged(const QModelIndex &/*topLeft*/, const QModelIndex &/*bottomRight*/, const QVector<int> &/*roles*/)
{
    QList<mitk::DataNode::Pointer> nodes = m_NodeTreeModel->GetNodeSet();
    std::list<mitk::DataNode::Pointer> nodeSet;
    foreach(mitk::DataNode::Pointer node, nodes)
    {
        if (node.IsNotNull())
            nodeSet.push_back(node);
    }
    m_DataManager->ClearNodeSet();
    m_DataManager->SetNodeSet(nodeSet);
  
}


void DataManagerWidget::NodeTreeViewRowsRemoved(
    const QModelIndex & /*parent*/, int /*start*/, int /*end*/)
{
    m_CurrentRowCount = m_NodeTreeModel->rowCount();
}

void DataManagerWidget::NodeTreeViewRowsInserted(const QModelIndex & parent, int start, int end)
{
    QModelIndex viewIndex = m_FilterModel->mapFromSource(parent);
    m_NodeTreeView->setExpanded(viewIndex, true);
    // a new row was inserted
    if (m_CurrentRowCount == 0 && m_NodeTreeModel->rowCount() == 1)
    {
        //this->OpenRenderWindowPart();
        m_CurrentRowCount = m_NodeTreeModel->rowCount();
    }
}



void DataManagerWidget::NodeTableViewContextMenuRequested(const QPoint & pos)
{
    int x = 0;
    QModelIndex selectedProxy = m_NodeTreeView->indexAt(pos);
    QModelIndex selected = m_FilterModel->mapToSource(selectedProxy);
    mitk::DataNode::Pointer node = m_NodeTreeModel->GetNode(selected);


    QModelIndexList indexesOfSelectedRowsFiltered = m_NodeTreeView->selectionModel()->selectedRows();
    QList<mitk::DataNode::Pointer> selectedNodes;
    for (int i = 0; i < indexesOfSelectedRowsFiltered.size(); ++i)
    {
        selectedNodes.append(m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(indexesOfSelectedRowsFiltered[i])));
    }

    if (!selectedNodes.isEmpty())
    {
        m_NodeMenu->clear();
        QList<QAction*> actions;
        if (selectedNodes.size() == 1)
        {
            actions = QmitkNodeDescriptorManager::GetInstance()->GetActions(node);

            for (QList<QAction*>::iterator it = actions.begin(); it != actions.end(); ++it)
            {
                (*it)->setData(QVariant::fromValue(node.GetPointer()));
            }
        }
        else
            actions = QmitkNodeDescriptorManager::GetInstance()->GetActions(selectedNodes);

        if (!m_ShowInActions.isEmpty())
        {
            QMenu* showInMenu = m_NodeMenu->addMenu("Show In");
            showInMenu->addActions(m_ShowInActions);
        }
        m_NodeMenu->addActions(actions);
        m_NodeMenu->popup(QCursor::pos());
    }
}

QList<mitk::DataNode::Pointer> DataManagerWidget::GetCurrentSelectionQList()
{
    QModelIndexList indexesOfSelectedRowsFiltered = m_NodeTreeView->selectionModel()->selectedRows();
    QList<mitk::DataNode::Pointer> selectedNodes;
    for (int i = 0; i < indexesOfSelectedRowsFiltered.size(); ++i)
    {
        selectedNodes.append(m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(indexesOfSelectedRowsFiltered[i])));
    }
    return selectedNodes;
}

std::list<mitk::DataNode::Pointer> DataManagerWidget::GetCurrentSelection()
{
    QModelIndexList indexesOfSelectedRowsFiltered = m_NodeTreeView->selectionModel()->selectedRows();
    std::list<mitk::DataNode::Pointer> selectedNodes;
    for (int i = 0; i < indexesOfSelectedRowsFiltered.size(); ++i)
    {
        selectedNodes.push_back(m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(indexesOfSelectedRowsFiltered[i])));
    }
    return selectedNodes;
}

void DataManagerWidget::ReinitSelectedNodes(bool)
{
    IQF_MitkRenderWindow* pMitkRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
    QList<mitk::DataNode::Pointer> selectedNodes = this->GetCurrentSelectionQList();
    foreach(mitk::DataNode::Pointer node, selectedNodes)
    {
        mitk::BaseData::Pointer basedata = node->GetData();
        if (basedata.IsNotNull() &&
            basedata->GetTimeGeometry()->IsValid())
        {
            pMitkRenderWindow->GetRenderingManager()->InitializeViews(
                basedata->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
        }
    }
}

void DataManagerWidget::RemoveSelectedNodes(bool)
{
    QModelIndexList indexesOfSelectedRowsFiltered = m_NodeTreeView->selectionModel()->selectedRows();
    QModelIndexList indexesOfSelectedRows;
    for (int i = 0; i < indexesOfSelectedRowsFiltered.size(); ++i)
    {
        indexesOfSelectedRows.push_back(m_FilterModel->mapToSource(indexesOfSelectedRowsFiltered[i]));
    }
    if (indexesOfSelectedRows.size() < 1)
    {
        return;
    }
    std::vector<mitk::DataNode::Pointer> selectedNodes;

    mitk::DataNode::Pointer node = 0;
    QString question = tr("Do you really want to remove ");

    for (QModelIndexList::iterator it = indexesOfSelectedRows.begin()
        ; it != indexesOfSelectedRows.end(); it++)
    {
        node = m_NodeTreeModel->GetNode(*it);
        // if node is not defined or if the node contains geometry data do not remove it
        if (node.IsNotNull() /*& strcmp(node->GetData()->GetNameOfClass(), "PlaneGeometryData") != 0*/)
        {
            selectedNodes.push_back(node);
            question.append(QString::fromStdString(node->GetName()));
            question.append(", ");
        }
    }
    // remove the last two characters = ", "
    question = question.remove(question.size() - 2, 2);
    question.append(" from data storage?");

    QMessageBox::StandardButton answerButton = QMessageBox::question(m_Parent
        , tr("DataManager")
        , question
        , QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    IQF_MitkDataManager* pMiktDataStorage = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    if (answerButton == QMessageBox::Yes)
    {
        for (std::vector<mitk::DataNode::Pointer>::iterator it = selectedNodes.begin()
            ; it != selectedNodes.end(); it++)
        {
            node = *it;
            pMiktDataStorage->GetDataStorage()->Remove(node);
            if (m_GlobalReinitOnNodeDelete)
                this->GlobalReinit(false);
        }
    }
}


void DataManagerWidget::SaveSelectedNodes(bool checked)
{
    std::list<mitk::DataNode::Pointer> dataNodes = GetCurrentSelection();

    std::vector<const mitk::BaseData*> data;
    QStringList names;
    for (std::list<mitk::DataNode::Pointer>::const_iterator nodeIter = dataNodes.begin(),
        nodeIterEnd = dataNodes.end(); nodeIter != nodeIterEnd; ++nodeIter)
    {
        data.push_back((*nodeIter)->GetData());
        std::string name;
        (*nodeIter)->GetStringProperty("name", name);
        names.push_back(QString::fromStdString(name));
    }

    try
    {
        IQF_MitkReference* pReference = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);

        QStringList fileNames = QmitkIOUtil::Save(data, names, pReference? pReference->GetString("LastFileSavePath"):"",
            this);
        if (!fileNames.empty()&& pReference)
        {
            pReference->SetString("LastFileSavePath", QFileInfo(fileNames.back()).absolutePath().toStdString().c_str());
        }
    }
    catch (const mitk::Exception& e)
    {
        MITK_INFO << e;
        return;
    }
    
}

void DataManagerWidget::OpacityChanged(int value)
{
    mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
    if (node)
    {
        float opacity = static_cast<float>(value) / 100.0f;
        node->SetFloatProperty("opacity", opacity);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void DataManagerWidget::OpacityActionChanged()
{
    mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
    if (node)
    {
        float opacity = 0.0;
        if (node->GetFloatProperty("opacity", opacity))
        {
            m_OpacitySlider->setValue(static_cast<int>(opacity * 100));
        }
    }
}

void DataManagerWidget::ColormapMenuAboutToShow()
{
    mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
    if (!node)
        return;

    mitk::LookupTableProperty::Pointer lookupTableProperty =
        dynamic_cast<mitk::LookupTableProperty*>(node->GetProperty("LookupTable"));
    if (!lookupTableProperty)
    {
        mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
        lookupTableProperty = mitk::LookupTableProperty::New();
        lookupTableProperty->SetLookupTable(mitkLut);
        node->SetProperty("LookupTable", lookupTableProperty);
    }

    mitk::LookupTable::Pointer lookupTable = lookupTableProperty->GetValue();
    if (!lookupTable)
        return;

    m_ColormapAction->menu()->clear();
    QAction* tmp;

    int i = 0;
    std::string lutType = lookupTable->typenameList[i];

    while (lutType != "END_OF_ARRAY")
    {
        m_ColormapAction->menu()->setStyleSheet(m_pR->getStyleResource("mainstyle"));
        tmp = m_ColormapAction->menu()->addAction(QString::fromStdString(lutType));
        tmp->setCheckable(true);

        if (lutType == lookupTable->GetActiveTypeAsString())
        {
            tmp->setChecked(true);
        }

        QObject::connect(tmp, SIGNAL(triggered(bool)), this, SLOT(ColormapActionToggled(bool)));

        lutType = lookupTable->typenameList[++i];
    }
}

void DataManagerWidget::ColormapActionToggled(bool /*checked*/)
{
    mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
    if (!node)
        return;

    mitk::LookupTableProperty::Pointer lookupTableProperty =
        dynamic_cast<mitk::LookupTableProperty*>(node->GetProperty("LookupTable"));
    if (!lookupTableProperty)
        return;

    QAction* senderAction = qobject_cast<QAction*>(QObject::sender());
    if (!senderAction)
        return;

    std::string activatedItem = senderAction->text().toStdString();

    mitk::LookupTable::Pointer lookupTable = lookupTableProperty->GetValue();
    if (!lookupTable)
        return;

    lookupTable->SetType(activatedItem);
    lookupTableProperty->SetValue(lookupTable);
    mitk::RenderingModeProperty::Pointer renderingMode =
        dynamic_cast<mitk::RenderingModeProperty*>(node->GetProperty("Image Rendering.Mode"));
    renderingMode->SetValue(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void DataManagerWidget::ColorChanged()
{
    mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
    if (node)
    {
        mitk::Color color;
        mitk::ColorProperty::Pointer colorProp;
        node->GetProperty(colorProp, "color");
        if (colorProp.IsNull())
            return;
        color = colorProp->GetValue();
        QColor initial(color.GetRed() * 255, color.GetGreen() * 255, color.GetBlue() * 255);
        QColor qcolor = QColorDialog::getColor(initial, 0, QString("Change color"));
        if (!qcolor.isValid())
            return;
        m_ColorButton->setAutoFillBackground(true);
        node->SetProperty("color", mitk::ColorProperty::New(qcolor.red() / 255.0, qcolor.green() / 255.0, qcolor.blue() / 255.0));
        if (node->GetProperty("binaryimage.selectedcolor"))
        {
            node->SetProperty("binaryimage.selectedcolor", mitk::ColorProperty::New(qcolor.red() / 255.0, qcolor.green() / 255.0, qcolor.blue() / 255.0));
        }
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void DataManagerWidget::ColorActionChanged()
{
    mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
    if (node)
    {
        mitk::Color color;
        mitk::ColorProperty::Pointer colorProp;
        node->GetProperty(colorProp, "color");
        if (colorProp.IsNull())
            return;
        color = colorProp->GetValue();

        QString styleSheet = "background-color:rgb(";
        styleSheet.append(QString::number(color[0] * 255));
        styleSheet.append(",");
        styleSheet.append(QString::number(color[1] * 255));
        styleSheet.append(",");
        styleSheet.append(QString::number(color[2] * 255));
        styleSheet.append(")");
        m_ColorButton->setStyleSheet(styleSheet);
    }
}

void DataManagerWidget::GlobalReinit(bool checked)
{
    mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());
}

void DataManagerWidget::SurfaceRepresentationMenuAboutToShow()
{
    mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
    if (!node)
        return;

    mitk::EnumerationProperty* representationProp =
        dynamic_cast<mitk::EnumerationProperty*> (node->GetProperty("material.representation"));
    if (!representationProp)
        return;

    // clear menu
    m_SurfaceRepresentation->menu()->clear();
    QAction* tmp;

    // create menu entries
    for (mitk::EnumerationProperty::EnumConstIterator it = representationProp->Begin(); it != representationProp->End()
        ; it++)
    {
        tmp = m_SurfaceRepresentation->menu()->addAction(QString::fromStdString(it->second));
        tmp->setCheckable(true);

        if (it->second == representationProp->GetValueAsString())
        {
            tmp->setChecked(true);
        }

        QObject::connect(tmp, SIGNAL(triggered(bool))
            , this, SLOT(SurfaceRepresentationActionToggled(bool)));
    }
}

void DataManagerWidget::SurfaceRepresentationActionToggled(bool /*checked*/)
{
    mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
    if (!node)
        return;

    mitk::EnumerationProperty* representationProp =
        dynamic_cast<mitk::EnumerationProperty*> (node->GetProperty("material.representation"));
    if (!representationProp)
        return;

    QAction* senderAction = qobject_cast<QAction*> (QObject::sender());

    if (!senderAction)
        return;

    std::string activatedItem = senderAction->text().toStdString();

    if (activatedItem != representationProp->GetValueAsString())
    {
        if (representationProp->IsValidEnumerationValue(activatedItem))
        {
            representationProp->SetValue(activatedItem);
            representationProp->InvokeEvent(itk::ModifiedEvent());
            representationProp->Modified();

            mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
    }

}

void DataManagerWidget::TextureInterpolationChanged()
{
    mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
    if (node)
    {
        bool textureInterpolation = false;
        node->GetBoolProperty("texture interpolation", textureInterpolation);
        m_TextureInterpolation->setChecked(textureInterpolation);
    }
}

void DataManagerWidget::TextureInterpolationToggled(bool checked)
{
    mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
    if (node)
    {
        node->SetBoolProperty("texture interpolation", checked);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}