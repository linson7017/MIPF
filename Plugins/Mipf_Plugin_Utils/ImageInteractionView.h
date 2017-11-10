/********************************************************************
	FileName:    ImageInteractionView.h
	Author:        Ling Song
	Date:           Month 10 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef ImageInteractionView_h__
#define ImageInteractionView_h__

#include "MitkPluginView.h"
#include "ui_ImageNavigationView.h"

#include "mitkDataInteractor.h"

#pragma once
class ImageInteractionView : public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    ImageInteractionView();
    ~ImageInteractionView();
    void CreateView() override;
    WndHandle GetPluginHandle() override
    {
        return this;
    }
protected slots:
    void Start(bool start);
    void Reset();
private:
    void ImageMatrixChanged();

    mitk::DataInteractor::Pointer m_Interactor;
    Ui::ImageNavigationView m_ui;
};

#endif // ImageInteractionView_h__