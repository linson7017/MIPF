#ifndef DSAHistogramView_h__ 
#define DSAHistogramView_h__ 
 
#include "MitkPluginView.h" 
#include <QWidget>
#include "ui_DSAHistogramView.h"
 
class QwtPlotCurve;
class QwtPlotMarker;
class Histogram;
class DSAHistogramView : public QWidget,public MitkPluginView  
{  
public:   
    DSAHistogramView(); 
    ~DSAHistogramView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;
protected slots:
void Apply();


private:
    Ui::DSAHistogramView m_ui;

    QwtPlotCurve* m_curve;
    Histogram* m_histogram;
    QwtPlotMarker *m_maxX;
    QwtPlotMarker *m_maxY;
    QwtPlotMarker *m_minX;
    QwtPlotMarker *m_minY;
    QwtPlotMarker *m_maxPos;
    QwtPlotMarker *m_minPos;
};
#endif // DSAHistogramView_h__ 