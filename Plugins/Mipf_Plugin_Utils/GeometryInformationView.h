/********************************************************************
	FileName:    GeometryInformationView.h
	Author:        Ling Song
	Date:           Month 9 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef GeometryInformationView_h__
#define GeometryInformationView_h__
#include "MitkPluginView.h"
#include <QWidget>

#include "ui_GeometryInformationView.h"

#pragma once
class GeometryInformationView   :public QWidget,public MitkPluginView
{
public:
    GeometryInformationView();
    ~GeometryInformationView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;
protected:
    void Update(const char* szMessage, int iValue /* = 0 */, void* pValue /* = 0 */) override;
private:
    Ui::GeometryInformationView m_ui;

};

#endif // GeometryInformationView_h__