#ifndef CQF_MitkDataStorage_h__
#define CQF_MitkDataStorage_h__

#pragma once

#include "MitkMain/IQF_MitkDataManager.h"
#include <mitkStandaloneDataStorage.h>
#include <mitkDataNode.h>

namespace QF
{
    class IQF_Main;
}

class CQF_MitkDataManager : public IQF_MitkDataManager
{
public:
    CQF_MitkDataManager(QF::IQF_Main* pMain);
    ~CQF_MitkDataManager();
    virtual void Init();
    virtual bool Load(const char* filename);
    virtual mitk::DataStorage::Pointer GetDataStorage();

    virtual void SetNodeSet(std::list<mitk::DataNode::Pointer> nodeSet);
    virtual std::list<mitk::DataNode::Pointer> GetNodeSet();
    virtual mitk::DataNode::Pointer GetCurrentNode();
    virtual void ClearNodeSet();

    virtual void SetSelectedNode(std::vector<mitk::DataNode::Pointer> selectNodes);
    virtual std::vector<mitk::DataNode::Pointer> GetSelectedNodes();
private:
    void RelateDataStorage();
    void OnNodeRemoved(const mitk::DataNode *node);
    void OnNodeAdded(const mitk::DataNode* node);
    void OnNodeChanged(const mitk::DataNode* node);
    void OnNodeDeleted(const mitk::DataNode* node);
    void OnNodeInteractorChanged(const mitk::DataNode* node);


    mitk::StandaloneDataStorage::Pointer m_DataStorage;
    std::vector<mitk::DataNode::Pointer> m_SelectedNodes;
    std::list<mitk::DataNode::Pointer> m_NodeSet;
    QF::IQF_Main* m_pMain;
};

#endif // CQF_MitkDataStorage_h__
