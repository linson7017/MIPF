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
protected:
    void wheelEvent(QWheelEvent *e);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    mitk::DataNode::Pointer RenderWindow::DetectTopMostVisibleImage();
private:
    vtkSmartPointer<vtkCornerAnnotation> m_annotation;
    bool m_dragging;
    QPoint m_preMousePt;
};

#endif // RenderWindow3D_h__