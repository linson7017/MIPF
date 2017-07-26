#ifndef AirwaySegmentationView_h__
#define AirwaySegmentationView_h__

#include "MitkPluginView.h"
#include "mitkPointSet.h"
#include <QObject>
#include <vector>
#include <QFutureWatcher>
#include <QFuture>
#include <QtConcurrent>

class QmitkDataStorageComboBox;

class AirwaySegmentationView : public QObject, public MitkPluginView
{
public:
    AirwaySegmentationView(QF::IQF_Main* pMain);
	void Contructed(R* pR);

protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);


public slots:
	void DoSomething();
	void AirwayFinished();

private:
	QFutureWatcher<void> m_watcher;
	QFuture<void> m_future;

	mitk::PointSet::Pointer m_PointSet;

	QmitkDataStorageComboBox* m_pSourceImageSelector;

	mitk::DataNode::Pointer m_result;
	mitk::Image::Pointer m_labelimage;
};

#endif // AirwaySegmentationView_h__