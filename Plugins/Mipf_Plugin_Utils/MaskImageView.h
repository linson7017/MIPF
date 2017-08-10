#ifndef MaskImageView_h__
#define MaskImageView_h__

#pragma once

#include "MitkPluginView.h"
#include <QWidget>
#include "ui_MaskImageView.h"

class QmitkDataStorageComboBox;

class MaskImageView :public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    MaskImageView();
    ~MaskImageView();
    void CreateView();
protected slots:
    void Mask();
private:
    Ui::MaskImageView m_ui;
};

#endif // MaskImageView_h__


