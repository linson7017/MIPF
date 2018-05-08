#ifndef LabelImageProcessView_h__
#define LabelImageProcessView_h__

#pragma once

#include "MitkPluginView.h"
#include <QWidget>
#include "ITKImageTypeDef.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrentRun>

#include "ui_LabelImageProcessView.h"

class QmitkDataStorageComboBox;


class LabelImageProcessView :public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    LabelImageProcessView();
    ~LabelImageProcessView();
    void CreateView();
    WndHandle GetPluginHandle() { return this; }
protected slots:
    void Resample();

private:
    void UpdateLabelConnectedComponentInformation(const mitk::DataNode * node);

private:
    Ui::LabelImageProcessView m_ui;
};

#endif // LabelImageProcessView_h__
