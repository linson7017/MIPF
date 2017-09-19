/********************************************************************
	FileName:    SurfaceConnectedView.h
	Author:        Ling Song
	Date:           Month 9 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef SurfaceConnectedView_h__
#define SurfaceConnectedView_h__

#include "MitkPluginView.h"
#include <QWidget>
#include "ui_SurfaceConnectedView.h"

#pragma once
class CanvasPicker;

class SurfaceConnectedView : public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    SurfaceConnectedView();
    ~SurfaceConnectedView();
    void CreateView() override;
protected slots:
    void Apply();
    void Save();
    void Import();
    void OpacityChanged(int regionID, float value);
    void ColorChanged(const QColor& c);

private:
    Ui::SurfaceConnectedView m_ui;
    int m_NumberOfRegion;

    CanvasPicker* m_picker;
};
#endif // SurfaceConnectedView_h__

