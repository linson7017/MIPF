#ifndef WireMouldingTestView_h__
#define WireMouldingTestView_h__
/********************************************************************
	FileName:    WireMouldingTestView
	Author:        Ling Song
	Date:           Month 4 ; Year 2018
	Purpose:	     
*********************************************************************/
#include "MitkPluginView.h"
#include <QWidget>
#include "ui_WireMouldingTestView.h"
#include "vtkPointLocator.h"
#include "vtkCellLocator.h"
#include "vtkMath.h"
#include <vtkSelectEnclosedPoints.h>
#include "CVA/IQF_GuideWireMoulding.h"


class WireMouldingTestView:public QWidget,public MitkPluginView
{                                           
    Q_OBJECT
public:
    WireMouldingTestView();
    ~WireMouldingTestView();
    void CreateView();
    WndHandle GetPluginHandle() { return this; }
protected slots:
    void Apply();
private:
    Ui::WireMoudlingTestView m_ui;

   // IQF_GuideWireMoulding* pGuideWireMoulding;
};

#endif // WireMouldingTestView_h__
