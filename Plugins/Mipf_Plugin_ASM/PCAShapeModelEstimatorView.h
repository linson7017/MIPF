/********************************************************************
	FileName:    PCAShapeModelEstimatorView.h
	Author:        Ling Song
	Date:           Month 9 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef PCAShapeModelEstimatorView_h__
#define PCAShapeModelEstimatorView_h__

#include <QWidget>

#include "ui_PCAShapeModelEstimatorView.h"

namespace QF
{
    class IQF_Main;
}

#pragma once
class PCAShapeModelEstimatorView :public QWidget
{
    Q_OBJECT
public:
    PCAShapeModelEstimatorView(QF::IQF_Main* pMain,QWidget* parent=NULL);
    ~PCAShapeModelEstimatorView();
    template<class TImage>
    void AddITKImageNode(TImage* itkImage,const char* name);
protected slots:
    void BrowseFile();
    void Apply();
private:
    Ui::PCAShapeModelEstimatorView m_ui;

    QF::IQF_Main* m_pMain;
};

#endif // PCAShapeModelEstimatorView_h__