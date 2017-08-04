#ifndef ProfileGrayScaleDistributionView_h__
#define ProfileGrayScaleDistributionView_h__

#include "MitkPluginView.h"
#include <QWidget>
#include "ui_ProfileGrayScaleDistributionView.h"

#include "mitkPointSet.h"

class QwtPlotCurve;
class QwtPlotMarker;

#pragma once
class ProfileGrayScaleDistributionView : public QWidget,public MitkPluginView
{
    Q_OBJECT
public:
    ProfileGrayScaleDistributionView();
    ~ProfileGrayScaleDistributionView();
    void CreateView();
protected slots:
    void Clear();
    void Plot();
    void OnSelectStartPoint();
    void OnSelectEndPoint();
private:
    void DrawLine();
private:
    Ui::ProfileGrayScaleDistributionView m_ui;
    mitk::DataNode::Pointer m_lineNode;

    QwtPlotCurve* m_curve;
    mitk::Point3D m_startPoint;
    mitk::Point3D m_endPoint;

    mitk::PointSet::Pointer m_linePoints;

    QwtPlotMarker *m_maxX;
    QwtPlotMarker *m_maxY;
    QwtPlotMarker *m_minX;
    QwtPlotMarker *m_minY;
    QwtPlotMarker *m_maxPos;
    QwtPlotMarker *m_minPos;
};

#endif // ProfileGrayScaleDistributionView_h__
