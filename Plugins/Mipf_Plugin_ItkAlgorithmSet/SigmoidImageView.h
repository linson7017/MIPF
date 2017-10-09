/********************************************************************
	FileName:    SigmoidImageView.h
	Author:        Ling Song
	Date:           Month 10 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef SigmoidImageView_h__
#define SigmoidImageView_h__

#include "MitkPluginView.h"

#include "ui_SigmoidImageView.h"

class SigmoidImageView: public QWidget,public MitkPluginView
{
    Q_OBJECT
public:
    SigmoidImageView();
    ~SigmoidImageView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;

protected slots:
    void Apply();
    void OnImageSelectionChanged(const mitk::DataNode * node); 
private:
    Ui::SigmoidImageView m_ui;
};

#endif // SigmoidImageView_h__