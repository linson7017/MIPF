#ifndef BoneExtract_h__
#define BoneExtract_h__

#include "MitkPluginView.h"

#include "ui_boneextract.h"


#pragma once
class BoneExtract: public QWidget,public MitkPluginView
{
    Q_OBJECT
public:
    BoneExtract();
    ~BoneExtract();
    void CreateView();
protected slots:
    void Extract();
    void AdaptiveThresholdExtract();
    void SelectionChanged(const mitk::DataNode* node);
private:
    Ui::BoneExtractView m_ui;
};

#endif // BoneExtract_h__
