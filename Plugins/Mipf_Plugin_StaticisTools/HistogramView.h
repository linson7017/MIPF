#ifndef HistogramView_h__
#define HistogramView_h__

#include "MitkPluginView.h"

#include <QWidget>
#include <QTimer>

#include "mitkPointSet.h"

#include "ui_HistogramView.h"

class Histogram;
class QwtPlotMarker;

class HistogramView : public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    HistogramView();
    ~HistogramView();
    void CreateView();
   
protected slots:
    void Plot();
    void OnSelectStartPoint();
    void OnSelectEndPoint();
    void ProfilePlot();

private:
    void DrawLine();
private:
    Ui::HistogramView m_ui;
    Histogram* m_histogram;

    Histogram* m_profileHistogram;

    mitk::Point3D m_startPoint;
    mitk::Point3D m_endPoint;
    mitk::DataNode::Pointer m_pLineNode;
    mitk::PointSet::Pointer m_linePoints;

    QTimer m_timer;
    mitk::DataNode::Pointer m_dataNode;
    mitk::Image::Pointer m_image;
    QImage* m_qImage;
};

#endif // HistogramView_h__