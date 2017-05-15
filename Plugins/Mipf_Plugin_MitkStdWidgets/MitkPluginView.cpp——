#include "MitkPluginView.h"
#include "MitkMain/IQF_MitkRenderWindow.h"
#include "MitkMain/IQF_MitkDataManager.h"
#include "iqf_main.h"

MitkPluginView::MitkPluginView(QF::IQF_Main* pMain) :PluginView(pMain)
{
    m_pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    m_pMitkRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
}


void MitkPluginView::RequestRenderWindowUpdate(mitk::RenderingManager::RequestType requestType)
{
    if (m_pMitkRenderWindow)
    {
        m_pMitkRenderWindow->GetRenderingManager()->RequestUpdateAll(requestType);
    }
}

void MitkPluginView::CastFromStdNodesToQListNodes(std::vector<mitk::DataNode::Pointer>& stdNodes, QList<mitk::DataNode::Pointer>& qlistNodes)
{
    qlistNodes.clear();
    for (int i = 0; i < stdNodes.size(); i++)
    {
        qlistNodes.append(stdNodes.at(i));
    }
}

QList<mitk::DataNode::Pointer> MitkPluginView::GetCurrentSelection()
{
    QList<mitk::DataNode::Pointer> qlistNodes;
    CastFromStdNodesToQListNodes(m_pMitkDataManager->GetSelectedNodes(), qlistNodes);
    return qlistNodes;
}
