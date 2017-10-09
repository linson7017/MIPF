/********************************************************************
	FileName:    LevelSetASMSegmentationView.h
	Author:        Ling Song
	Date:           Month 9 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef LevelSetASMSegmentationView_h__
#define LevelSetASMSegmentationView_h__

#include <QWidget>

#include "mitkPointSet.h"
#include "mitkDataNode.h"

#include "ui_LevelSetASMSegmentationView.h"

namespace QF
{
    class IQF_Main;
}

template<class TFilter>
class CommandIterationUpdate;

class LevelSetASMSegmentationView : public QWidget
{
     Q_OBJECT
public:
    LevelSetASMSegmentationView(QF::IQF_Main* pMain, QWidget* parent = NULL);
    ~LevelSetASMSegmentationView();

    
protected slots:
    void AddPCAImage();
    void RemovePCAImage();
    void Apply();
private:
    template<class TImageType>
    void ImportITKImage(TImageType* itkImage, const char* name, mitk::DataNode* parentNode = nullptr);
private:
    Ui::LevelSetASMSegmentationView m_ui;

    mitk::DataNode::Pointer m_PointSetNode;
    mitk::PointSet::Pointer m_PointSet;

    QF::IQF_Main* m_pMain;
};

#endif // LevelSetASMSegmentationView_h__