#ifndef PointListView_h__
#define PointListView_h__

#pragma once

#include <MitkPluginView.h>
#include "ui_PointListView.h"

#include <mitkDataNode.h>

class IQF_MitkPointList;

class PointListView : public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    PointListView();
    ~PointListView();
    void CreateView();
protected:
    void Update(const char* szMessage, int iValue /* = 0 */, void* pValue /* = 0 */);
    
protected slots:
    void OnAddPoint(bool add);
    void OnRemoveSelectedPoint();
    void OnSavePoints();
    void OnLoadPoints();
private:
    Ui::PointListView m_ui;

    IQF_MitkPointList* m_pPointList;

    mitk::DataNode::Pointer m_pPointSetNode;

};

#endif // PointListView_h__
