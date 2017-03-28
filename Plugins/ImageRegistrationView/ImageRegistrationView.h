#ifndef ImageRegistrationView_h__
#define ImageRegistrationView_h__

#include "PluginView.h"
#include "mitkPointSet.h"
#include "mitkDataNode.h"
#include "Vector3.h"
#include <QObject>

//class QmitkPointListWidget;
class QmitkDataStorageComboBox;

class ImageRegistrationView :public QObject,public PluginView
{
    Q_OBJECT
public:
    ImageRegistrationView(QF::IQF_Main* pMain);
    ~ImageRegistrationView();
    void InitResource(R* pR);
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
protected slots:
    void OnFixedImageSelectionChanged(const mitk::DataNode* node);
    void OnMovingImageSelectionChanged(const mitk::DataNode* node);
private:

    mitk::PointSet::Pointer m_PointSet;
    QmitkDataStorageComboBox* m_FixedDataStorageComboBox;
    QmitkDataStorageComboBox* m_MovingDataStorageComboBox;
    mitk::DataNode* m_FixedImageNode;
    mitk::DataNode* m_MovingImageNode;
};


#endif // SliceBySliceTrackingView_h__
