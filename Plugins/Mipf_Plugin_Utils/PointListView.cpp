#include "PointListView.h"
#include "MitkStd/IQF_MitkPointList.h"

#include "QmitkPointListModel.h"

PointListView::PointListView():m_pPointList(NULL)
{

}

PointListView::~PointListView()
{
    if (m_pPointList)
    {
        m_pPointList->Release();
        m_pPointList = NULL;
    }
    
}

void PointListView::CreateView()
{
    m_ui.setupUi(this);
    
    IQF_PointListFactory* pFactory = (IQF_PointListFactory*)m_pMain->GetInterfacePtr(QF_MitkStd_PointListFactory);
    if (!pFactory)  return;

    m_pPointList = pFactory->CreatePointList();
    m_pPointList->Initialize();
    m_pPointList->Attach(this);

    m_pPointSetNode = mitk::DataNode::New();
    m_pPointList->CreateNewPointSetNode(m_pPointSetNode);
    m_ui.AddPointBtn->setCheckable(true);

    m_ui.PointsTable->SetMultiWidget(m_pMitkRenderWindow->GetMitkStdMultiWidget());
    dynamic_cast<QmitkPointListModel *>(m_ui.PointsTable->model())->SetPointSetNode(m_pPointSetNode);
    connect(m_ui.AddPointBtn, SIGNAL(clicked(bool)),this,SLOT(OnAddPoint(bool)));
    connect(m_ui.RemovePointBtn, SIGNAL(clicked()), this, SLOT(OnRemoveSelectedPoint()));
    connect(m_ui.SaveBtn, SIGNAL(clicked()), this, SLOT(OnSavePoints()));
    connect(m_ui.LoadBtn, SIGNAL(clicked()), this, SLOT(OnLoadPoints()));
}

void PointListView::Update(const char* szMessage, int iValue /* = 0 */, void* pValue /* = 0 */)
{
       if (strcmp(szMessage,MITK_MESSAGE_POINTLIST_CHANGED)==0)
       {
           mitk::PointSet* ps = (mitk::PointSet*)pValue;
           //m_ui.PointsTable->selectionChanged();
       }
}

void PointListView::OnAddPoint(bool add)
{
      if (m_pPointList)
      {
          m_pPointList->AddPoint(add);
      }
}

void PointListView::OnRemoveSelectedPoint()
{
    if (!m_pPointSetNode)
        return;
    mitk::PointSet *pointSet = dynamic_cast<mitk::PointSet *>(m_pPointSetNode->GetData());
    if (!pointSet)
        return;
    if (pointSet->GetSize() == 0)
        return;

    QmitkPointListModel *pointListModel = dynamic_cast<QmitkPointListModel *>(m_ui.PointsTable->model());
    pointListModel->RemoveSelectedPoint();
}

void PointListView::OnSavePoints()
{
    if (m_pPointList)
    {
        m_pPointList->SavePoints();
    }
}

void PointListView::OnLoadPoints()
{
    if (m_pPointList)
    {  
        mitk::PointSet::Pointer pPointSet = m_pPointList->LoadPoints();
        m_pPointSetNode->SetData(pPointSet);
        m_pPointList->SetPointSetNode(m_pPointSetNode);
        dynamic_cast<QmitkPointListModel *>(m_ui.PointsTable->model())->SetPointSetNode(m_pPointSetNode);
        m_ui.AddPointBtn->setChecked(false);
        m_pPointList->AddPoint(false);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}