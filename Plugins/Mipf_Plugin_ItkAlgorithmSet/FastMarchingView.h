/********************************************************************
	FileName:    FastMarchingView.h
	Author:        Ling Song
	Date:           Month 10 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef FastMarchingView_h__
#define FastMarchingView_h__

#include "MitkPluginView.h"
#include "ui_FastMarchingView.h"

#include "mitkPointSet.h"

class FastMarchingView  :public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    FastMarchingView();
    ~FastMarchingView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;


protected slots:
    void SeedPointsChanged();
    void OnImageSelectionChanged(const mitk::DataNode *node);
private:
    Ui::FastMarchingView m_ui;

    mitk::PointSet::Pointer m_pPointSet;
    mitk::DataNode::Pointer m_pPointSetNode;
};

#endif // FastMarchingView_h__