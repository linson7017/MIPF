/********************************************************************
	FileName:    SurfaceToImageView.h
	Author:        Ling Song
	Date:           Month 9 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef SurfaceToImageView_h__
#define SurfaceToImageView_h__

#include "MitkPluginView.h"
#include <QWidget>

#include "ui_SurfaceToImageView.h"
class SurfaceToImageView:public QWidget,public MitkPluginView
{
    Q_OBJECT
public:
    SurfaceToImageView();
    ~SurfaceToImageView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;

    protected slots:
    void Apply();

private:
    vtkSmartPointer<vtkImageData> CreateWhiteImage(double* spacing,double* origin,int* extent,int* dim);
private:
    Ui::SurfaceToImageView m_ui;
};

#endif // SurfaceToImageView_h__