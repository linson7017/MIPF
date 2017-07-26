#ifndef ManualSegmentationTestView_h__
#define ManualSegmentationTestView_h__

#include "MitkPluginView.h"

class R;
class QmitkDataStorageComboBox;

class IQF_MitkSegmentationTool;

class ManualSegmentationTestView :public QObject, public MitkPluginView
{
    Q_OBJECT
public:
    ManualSegmentationTestView(QF::IQF_Main* pMain);

    void Constructed(R* pR);
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);

public slots:
    void OnImageSelectionChanged(const mitk::DataNode* node);
    void OnSegmentSelectionChanged(const mitk::DataNode* node);
private:
    QmitkDataStorageComboBox* m_pImageSelector;
    QmitkDataStorageComboBox* m_pSegmentSelector;
    IQF_MitkSegmentationTool* m_pMitkSegmentationTool;

    typedef    std::map<std::string, mitk::DataNode::Pointer>  DataNodeMapType;
    DataNodeMapType m_surfaceNodes;

    bool m_bToolInited;

    QString m_currentToolName;
};

#endif // ManualSegmentationTestView_h__