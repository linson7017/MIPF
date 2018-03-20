#include "CQF_MitkDataManager.h"

#include <MitkMain/mitk_main_msg.h>

#include "iqf_main.h"

CQF_MitkDataManager::CQF_MitkDataManager(QF::IQF_Main* pMain):m_pMain(pMain),
m_DefaultDataStorageID("")
{
    //Init();
}


CQF_MitkDataManager::~CQF_MitkDataManager()
{
}

void CQF_MitkDataManager::Init()
{
    if (m_DataStorageMap.empty())
    {
        mitk::DataStorage::Pointer defaultDataStorage = mitk::StandaloneDataStorage::New();
        m_DataStorageMap[m_DefaultDataStorageID] = defaultDataStorage;
        RelateDataStorage(defaultDataStorage);
    }   
}

void CQF_MitkDataManager::SetDataStorage(mitk::DataStorage::Pointer dataStorage, const std::string& id)
{
    if (dataStorage.IsNull())
    {
        return;
    }
    m_DataStorageMap[id] = dataStorage;
    RelateDataStorage(dataStorage);
}

void CQF_MitkDataManager::RelateDataStorage(mitk::DataStorage::Pointer pDataStorage)
{
    pDataStorage->ChangedNodeEvent.AddListener(
        mitk::MessageDelegate1<CQF_MitkDataManager, const mitk::DataNode *>(this, &CQF_MitkDataManager::OnNodeChanged));
    pDataStorage->RemoveNodeEvent.AddListener(
        mitk::MessageDelegate1<CQF_MitkDataManager, const mitk::DataNode *>(this, &CQF_MitkDataManager::OnNodeRemoved));
    pDataStorage->AddNodeEvent.AddListener(
        mitk::MessageDelegate1<CQF_MitkDataManager, const mitk::DataNode *>(this, &CQF_MitkDataManager::OnNodeAdded));
    pDataStorage->DeleteNodeEvent.AddListener(
        mitk::MessageDelegate1<CQF_MitkDataManager, const mitk::DataNode *>(this, &CQF_MitkDataManager::OnNodeDeleted));
    pDataStorage->InteractorChangedNodeEvent.AddListener(
        mitk::MessageDelegate1<CQF_MitkDataManager, const mitk::DataNode *>(this, &CQF_MitkDataManager::OnNodeInteractorChanged));
}


mitk::DataStorage::Pointer CQF_MitkDataManager::GetDataStorage(const std::string& id)
{
    DataStorageMapType::iterator it = m_DataStorageMap.find(id);
    if (it != m_DataStorageMap.end())
    {
        return it->second;
    }
    else
    {
        return NULL;
    }
}

void CQF_MitkDataManager::SetSelectedNode(std::vector<mitk::DataNode::Pointer> selectNodes, const std::string& id)
{
    m_SelectedNodes = selectNodes;
}

std::vector<mitk::DataNode::Pointer> CQF_MitkDataManager::GetSelectedNodes(const std::string& id)
{
    return m_SelectedNodes;
}


void CQF_MitkDataManager::SetNodeSet(std::list<mitk::DataNode::Pointer> nodeSet, const std::string& id)
{
    m_NodeSet = nodeSet;
}

std::list<mitk::DataNode::Pointer> CQF_MitkDataManager::GetNodeSet(const std::string& id)
{
    return m_NodeSet;
}

void CQF_MitkDataManager::ClearNodeSet(const std::string& id)
{
    m_NodeSet.clear();
}

mitk::DataNode::Pointer CQF_MitkDataManager::GetCurrentNode(const std::string& id)
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

void CQF_MitkDataManager::SetDataManagerSubject(QF::IQF_Subject* pSubject, const std::string& id)
{
    if (pSubject)
    {
        m_DataManagerSubjects[id] = pSubject;
    }    
}

QF::IQF_Subject* CQF_MitkDataManager::GetDataManagerSubject(const std::string& id)
{
    SubjectMapType::iterator it = m_DataManagerSubjects.find(id);
    if (it != m_DataManagerSubjects.end())
    {
        return it->second;
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

