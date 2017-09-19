#include "CQF_MitkDataManager.h"

#include <MitkMain/mitk_main_msg.h>

#include "iqf_main.h"

CQF_MitkDataManager::CQF_MitkDataManager(QF::IQF_Main* pMain):m_pMain(pMain), m_DataStorage(nullptr)
{
    //Init();
}


CQF_MitkDataManager::~CQF_MitkDataManager()
{
}

void CQF_MitkDataManager::Init()
{
    if (!m_DataStorage)
    {
        m_DataStorage = mitk::StandaloneDataStorage::New();
    }
    RelateDataStorage();
}

void CQF_MitkDataManager::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
    m_DataStorage = dataStorage;
}

void CQF_MitkDataManager::RelateDataStorage()
{
    m_DataStorage->ChangedNodeEvent.AddListener(
        mitk::MessageDelegate1<CQF_MitkDataManager, const mitk::DataNode *>(this, &CQF_MitkDataManager::OnNodeChanged));
    m_DataStorage->RemoveNodeEvent.AddListener(
        mitk::MessageDelegate1<CQF_MitkDataManager, const mitk::DataNode *>(this, &CQF_MitkDataManager::OnNodeRemoved));
    m_DataStorage->AddNodeEvent.AddListener(
        mitk::MessageDelegate1<CQF_MitkDataManager, const mitk::DataNode *>(this, &CQF_MitkDataManager::OnNodeAdded));
    m_DataStorage->DeleteNodeEvent.AddListener(
        mitk::MessageDelegate1<CQF_MitkDataManager, const mitk::DataNode *>(this, &CQF_MitkDataManager::OnNodeDeleted));
    m_DataStorage->InteractorChangedNodeEvent.AddListener(
        mitk::MessageDelegate1<CQF_MitkDataManager, const mitk::DataNode *>(this, &CQF_MitkDataManager::OnNodeInteractorChanged));
}


mitk::DataStorage::Pointer CQF_MitkDataManager::GetDataStorage()
{
    return m_DataStorage;
}

void CQF_MitkDataManager::SetSelectedNode(std::vector<mitk::DataNode::Pointer> selectNodes)
{
    m_SelectedNodes = selectNodes;
}

std::vector<mitk::DataNode::Pointer> CQF_MitkDataManager::GetSelectedNodes()
{
    return m_SelectedNodes;
}


void CQF_MitkDataManager::SetNodeSet(std::list<mitk::DataNode::Pointer> nodeSet)
{
    m_NodeSet = nodeSet;
}

std::list<mitk::DataNode::Pointer> CQF_MitkDataManager::GetNodeSet()
{
    return m_NodeSet;
}

void CQF_MitkDataManager::ClearNodeSet()
{
    m_NodeSet.clear();
}

mitk::DataNode::Pointer CQF_MitkDataManager::GetCurrentNode()
{
    if (m_SelectedNodes.size() > 0)
    {
        return m_SelectedNodes.front();
    }
    else if(m_NodeSet.size()>0)
    {
        return m_NodeSet.front();
    }
    else
    {
        return NULL;
    }
}

void CQF_MitkDataManager::OnNodeRemoved(const mitk::DataNode *node)
{
    m_pMain->SendMessageQf(MITK_MESSAGE_NODE_REMOVED, 0, (void*)node);
}

void CQF_MitkDataManager::OnNodeAdded(const mitk::DataNode* node)
{
    m_pMain->SendMessageQf(MITK_MESSAGE_NODE_ADDED, 0, (void*)node);
}

void CQF_MitkDataManager::OnNodeChanged(const mitk::DataNode* node)
{
    m_pMain->SendMessageQf(MITK_MESSAGE_NODE_CHANGED, 0, (void*)node);
}

void CQF_MitkDataManager::OnNodeDeleted(const mitk::DataNode* node)
{
    m_pMain->SendMessageQf(MITK_MESSAGE_NODE_DELETED, 0, (void*)node);
}

void CQF_MitkDataManager::OnNodeInteractorChanged(const mitk::DataNode* node)
{
    m_pMain->SendMessageQf(MITK_MESSAGE_NODE_INTERACTOR_CHANGED, 0, (void*)node);
}

