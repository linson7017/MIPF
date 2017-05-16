#ifndef DataManager_h__
#define DataManager_h__

#pragma once

#include <QmitkNodeDescriptorManager.h>

/// Qt
#include <QItemSelection>
#include "MitkPluginView.h"

namespace mitk
{
    class DataStorage;
}

// Forward declarations
class QMenu;
class QAction;
class QComboBox;
class QWidgetAction;
class QSlider;
class QModelIndex;
class QTreeView;
class QPushButton;
class QToolBar;
class QMenu;
class QSignalMapper;

class QmitkDnDFrameWidget;
class QmitkDataStorageTreeModel;
class QmitkDataManagerItemDelegate;
class QmitkNumberPropertySlider;
class QmitkDataStorageFilterProxyModel;
class QWidget;
class IQF_MitkDataManager;
class DataManagerItemDelegate;

class DataManagerWidget : public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    DataManagerWidget(QF::IQF_Main* pMain);
    ~DataManagerWidget();
    void Init(QWidget* parent);
    void SetDataStorage(IQF_MitkDataManager* dataStorage);
public slots:
    void NodeSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
    void NodeTreeViewRowsInserted(const QModelIndex & parent, int start, int end);
    void NodeTreeViewRowsRemoved(const QModelIndex & parent, int start, int end);
    void NodeTreeViewDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void NodeTableViewContextMenuRequested(const QPoint & index);

    void OpacityChanged(int value);
    void OpacityActionChanged();
    void ComponentActionChanged() {}
    void ColorChanged();
    void ColorActionChanged();
    void TextureInterpolationChanged() {}
    void TextureInterpolationToggled(bool checked) {}
    void ColormapMenuAboutToShow();
    void ColormapActionToggled(bool);
    void SurfaceRepresentationMenuAboutToShow() {}
    void SurfaceRepresentationActionToggled(bool checked) {}
    void RemoveSelectedNodes(bool checked = false);
    void SaveSelectedNodes(bool checked = false);
    void ReinitSelectedNodes(bool checked = false);
    void MakeAllNodesInvisible(bool checked = false) {}
    void ShowOnlySelectedNodes(bool checked = false) {}
    void ToggleVisibilityOfSelectedNodes(bool checked = false) {}
    void ShowInfoDialogForSelectedNodes(bool checked = false) {}
    void GlobalReinit(bool checked = false);
    void ContextMenuActionTriggered(bool) {}
    void ShowIn(const QString& editorId) {}
protected:
    QList<mitk::DataNode::Pointer> GetCurrentSelectionQList();
    std::list<mitk::DataNode::Pointer> GetCurrentSelection();
    virtual void CreateView();
    


    QWidget* m_Parent;
    QmitkDnDFrameWidget* m_DndFrameWidget;
    ///
    /// \brief A plain widget as the base pane.
    ///
    QmitkDataStorageTreeModel* m_NodeTreeModel;
    QmitkDataStorageFilterProxyModel* m_FilterModel;
    mitk::NodePredicateBase::Pointer m_HelperObjectFilterPredicate;
    mitk::NodePredicateBase::Pointer m_NodeWithNoDataFilterPredicate;
    ///
    /// \brief The Table view to show the selected nodes.
    ///
    QTreeView* m_NodeTreeView;
    ///
    /// \brief The context menu that shows up when right clicking on a node.
    ///
    QMenu* m_NodeMenu;
    ///
    /// \brief flag indicating whether a surface created from a selected decimation is decimated with vtkQuadricDecimation or not
    ///
    bool m_SurfaceDecimation;


    ///# A list of ALL actions for the Context Menu
    std::vector< std::pair< QmitkNodeDescriptor*, QAction* > > m_DescriptorActionList;

    /// A Slider widget to change the opacity of a node
    QSlider* m_OpacitySlider;
    /// A Slider widget to change the rendered vector component of an image
    QmitkNumberPropertySlider* m_ComponentSlider;
    /// button to change the color of a node
    QPushButton* m_ColorButton;
    /// TextureInterpolation action
    QAction* m_TextureInterpolation;
    /// SurfaceRepresentation action
    QAction* m_SurfaceRepresentation;
    /// Lookuptable selection action
    QAction* m_ColormapAction;

    /// Maps "Show in" actions to editor ids
    QSignalMapper* m_ShowInMapper;

    /// A list of "Show in" actions
    QList<QAction*> m_ShowInActions;

    /// saves the current amount of rows shown in the datamanager
    size_t m_CurrentRowCount;

    /// if true, GlobalReinit() is called if a node is deleted
    bool  m_GlobalReinitOnNodeDelete;

    DataManagerItemDelegate* m_ItemDelegate;


    //
    IQF_MitkDataManager* m_DataManager;

private:

   // QItemSelectionModel* GetDataNodeSelectionModel() const override;
};

#endif // DataManager_h__
