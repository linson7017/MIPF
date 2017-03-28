#ifndef MultiViews_H
#define MultiViews_H
#include "MitkPluginView.h"

#include <QWidget>
#include <mitkImage.h>
#include <mitkPointSet.h>
#include <mitkStandaloneDataStorage.h>
#include <itkImage.h>
#ifndef DOXYGEN_IGNORE

class QmitkStdMultiWidget;
class IQF_MitkDataManager;

class MultiViewsWidget : public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    MultiViewsWidget(QF::IQF_Main* pMain);
    ~MultiViewsWidget() {}
    virtual void Init(QWidget* parent);

    void ChangeLayout(int index);
    void ResetView();
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
protected:
    virtual void SetupWidgets();
    IQF_MitkDataManager* m_DataManager;
    mitk::Image::Pointer m_currentImage;
    mitk::PointSet::Pointer m_Seeds;
    mitk::Image::Pointer m_ResultImage;
    mitk::DataNode::Pointer m_ResultNode;

    QmitkStdMultiWidget* m_multiWidget;

    bool m_bInited;
};
#endif // DOXYGEN_IGNORE
#endif // MultiViews_H