/********************************************************************
	FileName:    SurfaceInteractionView.h
	Author:        Ling Song
	Date:           Month 10 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef SurfaceInteractionView_h__
#define SurfaceInteractionView_h__

#include "MitkPluginView.h"
#include "ui_SurfaceInteractionView.h"

#include "mitkDataInteractor.h"

#pragma once
class SurfaceInteractionView : public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    SurfaceInteractionView();
    ~SurfaceInteractionView();
    void CreateView() override;
    WndHandle GetPluginHandle() override
    {
        return this;
    }
protected slots:
    void Start(bool start);
    void Reset();
private:
    void ImageMatrixChanged(vtkMatrix4x4* matrix);

    mitk::DataInteractor::Pointer m_Interactor;
    Ui::SurfaceInteractionView m_ui;
};

#endif // SurfaceInteractionView_h__