/********************************************************************
	FileName:    ShapeDrawerView.h
	Author:        Ling Song
	Date:           Month 10 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef ShapeDrawerView_h__
#define ShapeDrawerView_h__

#include "MitkPluginView.h"

#include "ui_ShapeDrawerView.h"

#include "mitkDataNode.h"

#pragma once
class ShapeDrawerView :public QWidget,public MitkPluginView
{
    Q_OBJECT
public:
    ShapeDrawerView(QF::IQF_Main* pMain, QWidget* parent);
    ~ShapeDrawerView();
protected slots:
    void Apply();
private:
    Ui::ShapeDrawerView m_ui;

    mitk::DataNode::Pointer m_resultNode;
};

#endif // ShapeDrawerView_h__