/********************************************************************
	FileName:    RenderWindow3D.h
	Author:        Ling Song
	Date:           Month 9 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef RenderWindow3D_h__
#define RenderWindow3D_h__

#include "MitkPluginView.h"
#include <QObject>

#include "QmitkRenderWindow.h"
class vtkCornerAnnotation;

class RenderWindow:public QmitkRenderWindow,public MitkPluginView
{
    Q_OBJECT
public:
    RenderWindow();
    ~RenderWindow();

    void CreateView();

    WndHandle GetPluginHandle();
    
    void wheelEvent(QWheelEvent *e);

private:
    vtkSmartPointer<vtkCornerAnnotation> m_annotation;

};

#endif // RenderWindow3D_h__