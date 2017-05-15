#ifndef CMitkSegmentation_h__
#define CMitkSegmentation_h__

#include "mitkToolManager.h"
#include <QObject>


class QmitkRenderWindow;
class QmitkStdMultiWidget;
namespace QF {
    class IQF_Main;
}

class IQF_MitkDataManager;
class IQF_MitkRenderWindow;
class IQF_MitkReference;

class CMitkSegmentation :public QObject,public QF::IQF_Observer
{
    Q_OBJECT
public:
    CMitkSegmentation(QF::IQF_Main* pMain);
    ~CMitkSegmentation();
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
    void CreateView();

    void SetWorkDataNode(mitk::DataNode* workNode) { workingNode = workNode; }
    void SetReferenceDataNode(mitk::DataNode* referenceNode) { refNode = referenceNode; }
public:

    typedef std::map<mitk::DataNode*, unsigned long> NodeTagMapType;

    /*!
    \brief Invoked when the DataManager selection changed
    */
    virtual void OnSelectionChanged(mitk::DataNode::Pointer node);
    virtual void OnSelectionChanged(std::vector<mitk::DataNode::Pointer> nodes);

    // reaction to new segmentations being created by segmentation tools
    void NewNodeObjectsGenerated(mitk::ToolManager::DataVectorType*);

    // QmitkFunctionality's activate/deactivate
    virtual void Activated();
    virtual void Deactivated();
    virtual void Visible();

    // QmitkFunctionality's changes regarding THE QmitkStdMultiWidget
    virtual void StdMultiWidgetAvailable(QmitkStdMultiWidget* stdMultiWidget);
    virtual void StdMultiWidgetNotAvailable();
    virtual void StdMultiWidgetClosed(QmitkStdMultiWidget* stdMultiWidget);

    // BlueBerry's notification about preference changes (e.g. from a dialog)
    // virtual void OnPreferencesChanged(const berry::IBerryPreferences* prefs) {};

    // observer to mitk::RenderingManager's RenderingManagerViewsInitializedEvent event
    void RenderingManagerReinitialized();

    // observer to mitk::SliceController's SliceRotation event
    void SliceRotation(const itk::EventObject&) {}

    //static const std::string VIEW_ID;

    protected slots:
    void OnPatientComboBoxSelectionChanged(const mitk::DataNode* node);
    void OnSegmentationComboBoxSelectionChanged(const mitk::DataNode* node);

    // reaction to the button "New segmentation"
    void CreateNewSegmentation();

    void OnManualTool2DSelected(int id);

    void OnWorkingNodeVisibilityChanged();

    // called if a node's binary property has changed
    void OnBinaryPropertyChanged();

    void OnShowMarkerNodes(bool);

    void OnTabWidgetChanged(int);

protected:

    // a type for handling lists of DataNodes
    typedef std::vector<mitk::DataNode*> NodeList;

    // set available multiwidget
    void SetMultiWidget(QmitkStdMultiWidget* multiWidget);

    // actively query the current selection of data manager
    //void PullCurrentDataManagerSelection();

    // reactions to selection events from data manager (and potential other senders)
    //void BlueBerrySelectionChanged(berry::IWorkbenchPart::Pointer sourcepart, berry::ISelection::ConstPointer selection);
    mitk::DataNode::Pointer FindFirstRegularImage(std::vector<mitk::DataNode*> nodes) {}
    mitk::DataNode::Pointer FindFirstSegmentation(std::vector<mitk::DataNode*> nodes) {}

    // propagate BlueBerry selection to ToolManager for manual segmentation
    void SetToolManagerSelection(const mitk::DataNode* referenceData, const mitk::DataNode* workingData);

    // checks if given render window aligns with the slices of given image
    bool IsRenderWindowAligned(QmitkRenderWindow* renderWindow, mitk::Image* image) {}

    // make sure all images/segmentations look as selected by the users in this view's preferences
    void ForceDisplayPreferencesUponAllImages();

    // decorates a DataNode according to the user preference settings
    void ApplyDisplayOptions(mitk::DataNode* node);

    void ResetMouseCursor();

    void SetMouseCursor(const us::ModuleResource&, int hotspotX, int hotspotY);

    void SetToolSelectionBoxesEnabled(bool);

    bool m_MouseCursorSet;

    // handling of a list of known (organ name, organ color) combination
    // ATTENTION these methods are defined in QCMitkSegmentationOrganNamesHandling.cpp
    QStringList GetDefaultOrganColorString() {}
    void UpdateOrganList(QStringList& organColors, const QString& organname, mitk::Color colorname) {}
    void AppendToOrganList(QStringList& organColors, const QString& organname, int r, int g, int b) {}

    // If a contourmarker is selected, the plane in the related widget will be reoriented according to the marker`s geometry
    void OnContourMarkerSelected(const mitk::DataNode* node);

    void NodeRemoved(const mitk::DataNode* node);

    void NodeAdded(const mitk::DataNode *node);

    bool CheckForSameGeometry(const mitk::DataNode*, const mitk::DataNode*) const;

    void UpdateWarningLabel(QString text/*, bool overwriteExistingText = true*/);

    // THE currently existing QmitkStdMultiWidget
    QmitkStdMultiWidget * m_MultiWidget;

    unsigned long m_VisibilityChangedObserverTag;

    bool m_DataSelectionChanged;

    NodeTagMapType  m_WorkingDataObserverTags;

    NodeTagMapType  m_BinaryPropertyObserverTags;

    unsigned int m_RenderingManagerObserverTag;

    bool m_AutoSelectionEnabled;

    mitk::NodePredicateOr::Pointer m_IsOfTypeImagePredicate;
    mitk::NodePredicateProperty::Pointer m_IsBinaryPredicate;
    mitk::NodePredicateNot::Pointer m_IsNotBinaryPredicate;
    mitk::NodePredicateAnd::Pointer m_IsNotABinaryImagePredicate;
    mitk::NodePredicateAnd::Pointer m_IsABinaryImagePredicate;

    mitk::NodePredicateOr::Pointer m_IsASegmentationImagePredicate;
    mitk::NodePredicateAnd::Pointer m_IsAPatientImagePredicate;


    mitk::DataNode* workingNode;
    mitk::DataNode* refNode;

private:
        IQF_MitkDataManager* m_pMitkDataManager;
        IQF_MitkRenderWindow* m_pMitkRenderWindow;
        IQF_MitkReference* m_pMitkReferences;
        QF::IQF_Main* m_pMain;
};

#endif // CMitkSegmentation_h__