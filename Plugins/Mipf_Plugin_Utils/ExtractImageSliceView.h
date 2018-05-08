#ifndef ExtractImageSliceView_h__
#define ExtractImageSliceView_h__

#pragma once
#include "MitkPluginView.h"
#include "ITKImageTypeDef.h"
#include "ui_ExtractImageSliceView.h"

class QmitkDataStorageComboBox;
class ExtractImageSliceView :public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    ExtractImageSliceView();
    ~ExtractImageSliceView();
    void CreateView();
    WndHandle GetPluginHandle() { return this; }
protected slots:
    void Extract();
    void Register();

private:
    void ExtractBrain(Float2DImageType* input, Float2DImageType* output);

private:
    Ui::ExtractImageSliceView m_ui;
};

#endif // ExtractImageSliceView_h__
