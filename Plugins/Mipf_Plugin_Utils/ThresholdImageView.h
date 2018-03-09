/********************************************************************
	FileName:    ThresholdImage.h
	Author:        Ling Song
	Date:           Month 2 ; Year 2018
	Purpose:	     
*********************************************************************/
#ifndef ThresholdImageView_h__
#define ThresholdImageView_h__

#include "MitkPluginView.h"
#include <QWidget>
#include "ui_ThresholdImageView.h"

class ThresholdImageView :public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    ThresholdImageView();
    ~ThresholdImageView();
    void CreateView();
    WndHandle GetPluginHandle();
protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
protected slots:
    void ThresholdChanged(double minValue, double maxValue);
    void Export();
    void SelectionChanged(const mitk::DataNode* node);
    void VisibleChanged(bool visible);

private:
    Ui::ThresholdImageView m_ui;
    mitk::DataNode::Pointer m_pResultNode;
};
#endif // ThresholdImageView_h__