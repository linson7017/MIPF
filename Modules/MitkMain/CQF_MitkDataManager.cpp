#include "CQF_MitkDataManager.h"

#include <mitkIOUtil.h>
#include <MitkMain/mitk_main_msg.h>
#include "mitkRenderingManager.h"
#include "iqf_main.h"

CQF_MitkDataManager::CQF_MitkDataManager(QF::IQF_Main* pMain):m_pMain(pMain)
{
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

bool CQF_MitkDataManager::Load(const char* filename)
{
    // Load datanode (eg. many image formats, surface formats, etc.)
    mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(filename, *m_DataStorage);
    if (dataNodes->empty())
    {
        fprintf(stderr, "Could not open file %s \n\n", filename);
        return false;
    }
    mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(dataNodes->at(0)->GetData());
    if ((image.IsNotNull()))
    {
        mitk::TimeGeometry::Pointer geo = m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll());
        mitk::RenderingManager::GetInstance()->InitializeViews(geo);
        return true;
    }
    else 
    {
        fprintf(stderr, "Image file %s is empty! \n\n", filename);
        return false;
    }
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
    //Set Current Image
    mitk::Image::Pointer image = 0;
    std::string name = "";
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