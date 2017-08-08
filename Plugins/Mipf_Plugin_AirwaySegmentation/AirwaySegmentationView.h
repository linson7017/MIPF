#ifndef AirwaySegmentationView_h__
#define AirwaySegmentationView_h__

#include "MitkPluginView.h"
#include "mitkPointSet.h"
#include <QObject>
#include <vector>
#include <QFutureWatcher>
#include <QFuture>
#include <QtConcurrent>

#include "ui_AirwaySegmentation.h"

class QmitkDataStorageComboBox;
class IQF_MitkPointList;

class AirwaySegmentationView : public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    AirwaySegmentationView();
    ~AirwaySegmentationView();
    void CreateView();
public slots:
    void Segment(); 
	void DoSomething();
	void AirwayFinished();
    void OnSelectSeed(bool bSelecting);
    void OnClearSeed();

private:
	QFutureWatcher<void> m_watcher;
	QFuture<void> m_future;

	mitk::DataNode::Pointer m_result;
	mitk::Image::Pointer m_labelimage;

    IQF_MitkPointList* m_pPointList;

    Ui::AirwaySegmentationView m_ui;

};

#endif // AirwaySegmentationView_h__