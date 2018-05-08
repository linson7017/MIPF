/********************************************************************
	FileName:    SurfaceIntersectView.h
	Author:        Ling Song
	Date:           Month 9 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef SurfaceIntersectView_h__
#define SurfaceIntersectView_h__
#include "MitkPluginView.h"
#include <QWidget>

#include "ui_SurfaceIntersectView.h"

#pragma once
class SurfaceIntersectView   :public QWidget,public MitkPluginView
{
public:
    SurfaceIntersectView();
    ~SurfaceIntersectView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;
protected :
    void Apply();
private:
    Ui::SurfaceIntersectView m_ui;

};

#endif // SurfaceIntersectView_h__