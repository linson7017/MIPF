#ifndef TransformNodeView_h__
#define TransformNodeView_h__


#include "MitkPluginView.h"
#include <QWidget>

#include "ui_TransfromNodeView.h"

#pragma once
class TransformNodeView:public QWidget,public MitkPluginView
{
    Q_OBJECT
public:
    TransformNodeView();
    ~TransformNodeView();
    void CreateView();

protected slots:
    void Transform();

private:
    Ui::TransformNodeView m_ui;
};

#endif // TransformNodeView_h__
