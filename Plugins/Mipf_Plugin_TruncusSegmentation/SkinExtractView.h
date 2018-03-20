#ifndef SkinExtractView_h__
#define SkinExtractView_h__

#pragma once

#include "MitkPluginView.h"
#include <QWidget>

#include "ui_SkinExtractView.h"

class SkinExtractView : public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    SkinExtractView();
    ~SkinExtractView();
    void CreateView();
    WndHandle GetPluginHandle() { return this; }
protected slots:
    void Extract();
    void SelectionChanged(const mitk::DataNode* node);
private:
    Ui::SkinExtractView m_ui;
};

#endif // SkinExtractView_h__
