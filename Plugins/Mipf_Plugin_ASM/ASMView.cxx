#include "ASMView.h" 
#include "iqf_main.h"  
#include "PCAShapeModelEstimatorView.h"
#include "GenerateTrainingSetView.h"
#include "LevelSetASMSegmentationView.h"
#include "ShapeDrawerView.h"
#include "LevelSetSegmentationView.h"
  
ASMView::ASMView() :MitkPluginView() 
{
}
 
ASMView::~ASMView() 
{
}
 
void ASMView::CreateView()
{
    m_ui.setupUi(this);
    
    PCAShapeModelEstimatorView* pcaView = new PCAShapeModelEstimatorView(m_pMain,this);
    m_ui.stackedWidget->insertWidget(0,pcaView);

    GenerateTrainingSetView* gtView = new GenerateTrainingSetView(m_pMain, this);
    m_ui.stackedWidget->insertWidget(1, gtView);

    LevelSetASMSegmentationView* lsaView = new LevelSetASMSegmentationView(m_pMain, this);
    m_ui.stackedWidget->insertWidget(2, lsaView);

    ShapeDrawerView* sdView = new ShapeDrawerView(m_pMain, this);
    m_ui.stackedWidget->insertWidget(3, sdView);

    LevelSetSegmentationView* levelsetView = new LevelSetSegmentationView(m_pMain, this);
    m_ui.stackedWidget->insertWidget(4, levelsetView);

    m_ui.stackedWidget->setCurrentIndex(0);

    connect(m_ui.Algorithm, SIGNAL(currentIndexChanged(int)), this, SLOT(AlgorithmChanged(int)));
} 
 
WndHandle ASMView::GetPluginHandle() 
{
    return m_ui.verticalLayoutWidget;
}

void  ASMView::AlgorithmChanged(int i)
{
    if (i<=m_ui.stackedWidget->count() -1)
    {
        m_ui.stackedWidget->setCurrentIndex(i);
    }
}