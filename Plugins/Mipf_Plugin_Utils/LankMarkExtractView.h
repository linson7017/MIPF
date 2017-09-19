#ifndef LankMarkExtractView_h__
#define LankMarkExtractView_h__

#pragma once

#include "MitkPluginView.h"
#include "ui_LandMarkExtractView.h"


#include <QWidget>
namespace QF
{
    class IQF_Main;
}

class LankMarkExtractView:public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    LankMarkExtractView();
    ~LankMarkExtractView();
    void CreateView();

protected slots:
    void Extract();
    void OnImageSelectionChanged(const mitk::DataNode *node);
private:
    Ui::LandMarkExtractView m_ui;

    mitk::DataNode::Pointer m_pPointSetNode;
    mitk::PointSet::Pointer m_pPointSet;

};

#endif // LankMarkExtractView_h__
