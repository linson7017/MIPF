/********************************************************************
	FileName:    GenerateTrainingSet.h
	Author:        Ling Song
	Date:           Month 9 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef GenerateTrainingSetView_h__
#define GenerateTrainingSetView_h__

#include <QWidget>

#include "ui_GenerateTrainingSetView.h"

namespace QF
{
    class IQF_Main;
}

class GenerateTrainingSetView :public QWidget
{
    Q_OBJECT
public:
    GenerateTrainingSetView(QF::IQF_Main* pMain, QWidget* parent = NULL);
    ~GenerateTrainingSetView();
    protected slots:
    void DataDirBrowseFile();
    void OutputDirBrowseFile();
    void Apply();
private:
    Ui::GenerateTrainingSetView m_ui;

    QF::IQF_Main* m_pMain;
};

#endif // GenerateTrainingSetView_h__