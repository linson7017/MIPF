/********************************************************************
	FileName:    plot.h
	Author:        Ling Song
	Date:           Month 9 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef plot_h__
#define plot_h__

#include <qwt_plot.h>

#include <QColor>

class ColorBar;
class QwtWheel;
class QwtPlotCurve;

class Plot: public QwtPlot
{
    Q_OBJECT
public:
    Plot( QWidget *parent = NULL );
    virtual bool eventFilter( QObject *, QEvent * );

    void RefreshNumberOfPoint(int num);
    QColor GetSelectColor() { return m_color; }

public Q_SLOTS:
    void setCanvasColor( const QColor & );
    void insertCurve( int axis, double base );

private Q_SLOTS:
    void scrollLeftAxis( double );

signals:
    void colorChanged(const QColor&);

private:
    void insertCurve( Qt::Orientation, const QColor &, double base );

    ColorBar *d_colorBar;
    QwtWheel *d_wheel;

    QwtPlotCurve *d_curve;

    QColor m_color;
};
#endif // plot_h__