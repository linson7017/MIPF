/********************************************************************
	FileName:    SurfaceCombineView.h
	Author:        Ling Song
	Date:           Month 9 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef SurfaceCombineView_h__
#define SurfaceCombineView_h__
#include "MitkPluginView.h"
#include <QWidget>

#include "ui_SurfaceCombineView.h"

class vtkPolyData;

#pragma once
class SurfaceCombineView :public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    SurfaceCombineView();
    ~SurfaceCombineView();

    void CreateView() override;

protected slots:
    void Add();
    void Remove();
    void Combine();
protected:
    //void Update(const char* szMessage, int iValue /* = 0 */, void* pValue /* = 0 */);   

private:
    Ui::SurfaceCombineView m_ui;

    std::map<std::string, vtkPolyData*> m_surfaces;
};

#endif // SurfaceCombineView_h__