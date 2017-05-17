#ifndef CMitkSegmentation_h__
#define CMitkSegmentation_h__

#include "mitkToolManager.h"
#include <QObject>
#include <iqf_observer.h>

class QmitkRenderWindow;
class QmitkStdMultiWidget;
class QmitkSlicesInterpolator;
class QmitkToolSelectionBox;

namespace QF {
    class IQF_Main;
}

class IQF_MitkDataManager;
class IQF_MitkRenderWindow;
class IQF_MitkReference;

class CMitkSegmentation:public QF::IQF_Observer
{
public:
    typedef std::map<mitk::DataNode*, unsigned long> NodeTagMapType;
    CMitkSegmentation(QF::IQF_Main* pMain);
    ~CMitkSegmentation();
public:
    void Init();
    mitk::DataNode* CreateSegmentationNode();
    void SetWorkingNode(mitk::DataNode* node);
    void SetReferenceNode(mitk::DataNode* node);
    bool SelectTool(int id);


    virtual void OnSelectionChanged(mitk::DataNode::Pointer node);
    virtual void OnSelectionChanged(std::vector<mitk::DataNode::Pointer> nodes);
    void RenderingManagerReinitialized();

    void SetToolManagerSelection(const mitk::DataNode* referenceData, const mitk::DataNode* workingData);
    void ForceDisplayPreferencesUponAllImages() {}
    void ApplyDisplayOptions(mitk::DataNode* node) {}
    void ResetMouseCursor();
    void SetMouseCursor(const us::ModuleResource&, int hotspotX, int hotspotY);
    void SetToolSelectionBoxesEnabled(bool enabled) { m_toolSelectionEnabled = enabled; }
    void UpdateWarningLabel(QString text);
    void OnContourMarkerSelected(const mitk::DataNode *node);
    bool CheckForSameGeometry(const mitk::DataNode *node1, const mitk::DataNode *node2) const;

    void OnManualTool2DSelected(int id);
    void OnWorkingNodeVisibilityChanged();
    void OnBinaryPropertyChanged();

    void OnShowMarkerNodes(bool);

    void NodeAdded(const mitk::DataNode *node);
    void NodeRemoved(const mitk::DataNode* node);

    void SetMultiWidget(QmitkStdMultiWidget* multiWidget);
    void NewNodeObjectsGenerated(mitk::ToolManager::DataVectorType* nodes);
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
    mitk::NodePredicateOr::Pointer m_IsOfTypeImagePredicate;
    mitk::NodePredicateProperty::Pointer m_IsBinaryPredicate;
    mitk::NodePredicateNot::Pointer m_IsNotBinaryPredicate;
    mitk::NodePredicateAnd::Pointer m_IsNotABinaryImagePredicate;
    mitk::NodePredicateAnd::Pointer m_IsABinaryImagePredicate;

    mitk::NodePredicateOr::Pointer m_IsASegmentationImagePredicate;
    mitk::NodePredicateAnd::Pointer m_IsAPatientImagePredicate;


    mitk::DataNode* workingNode;
    mitk::DataNode* refNode;

    QmitkStdMultiWidget * m_MultiWidget;

    bool m_MouseCursorSet;
    unsigned int m_RenderingManagerObserverTag;

    NodeTagMapType  m_WorkingDataObserverTags;

    NodeTagMapType  m_BinaryPropertyObserverTags;

    QString m_warningLabel;
    bool m_toolSelectionEnabled;

    QmitkSlicesInterpolator* m_interpolator;

    QmitkToolSelectionBox *m_ManualToolSelectionBox;

private:
        IQF_MitkDataManager* m_pMitkDataManager;
        IQF_MitkRenderWindow* m_pMitkRenderWindow;
        IQF_MitkReference* m_pMitkReferences;
        QF::IQF_Main* m_pMain;

        bool m_inited;
};

#endif // CMitkSegmentation_h__