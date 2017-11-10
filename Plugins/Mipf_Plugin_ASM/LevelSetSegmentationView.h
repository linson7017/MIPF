/********************************************************************
	FileName:    LevelSetSegmentationView.h
	Author:        Ling Song
	Date:           Month 10 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef LevelSetSegmentationView_h__
#define LevelSetSegmentationView_h__

#include "MitkPluginView.h"
#include "ui_LevelSetSegmentationView.h"

#include "mitkPointSet.h"
#include <QThread>

class LSSegmentation;

class LevelSetSegmentationView :public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    LevelSetSegmentationView(QF::IQF_Main* pMain, QWidget* parent);
    ~LevelSetSegmentationView();
protected slots:
    void Apply();
    void Stop();
    void SlotInteractionEnd(const mitk::Image::Pointer& image);
    void SlotSegmentationFinished();
signals:
    void SignalDoSegmentation(const mitk::Image::Pointer& inputImage,const  mitk::PointSet::Pointer& pSeedPointSet);
    void SignalStopSegmentation();
private:
    mitk::DataNode::Pointer m_PointSetNode;
    mitk::PointSet::Pointer m_PointSet;

    mitk::DataNode::Pointer m_ObserveNode;
    LSSegmentation* m_pSegmentation;
    QThread* m_segmentationThread;

    Ui::LevelSetSegmentationView m_ui;
};

#endif // LevelSetSegmentationView_h__