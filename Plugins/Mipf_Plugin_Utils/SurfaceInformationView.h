/********************************************************************
	FileName:    SurfaceInformationView.h
	Author:        Ling Song
	Date:           Month 9 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef SurfaceInformationView_h__
#define SurfaceInformationView_h__
#include "MitkPluginView.h"
#include <QWidget>

#include "ui_SurfaceInformationView.h"

#pragma once
class SurfaceInformationView   :public QWidget,public MitkPluginView
{
public:
    SurfaceInformationView();
    ~SurfaceInformationView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;
protected:
    void Update(const char* szMessage, int iValue /* = 0 */, void* pValue /* = 0 */) override;
private:
    Ui::SurfaceInformationView m_ui;

};

#endif // SurfaceInformationView_h__