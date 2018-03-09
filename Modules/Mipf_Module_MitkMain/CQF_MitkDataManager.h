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
    virtual void SetDataStorage(mitk::DataStorage::Pointer dataStorage, const std::string& id = "");
    virtual mitk::DataStorage::Pointer GetDataStorage(const std::string& id = "");
    virtual void SetSelectedNode(std::vector<mitk::DataNode::Pointer> selectNodes, const std::string& id = "");
    virtual std::vector<mitk::DataNode::Pointer> GetSelectedNodes(const std::string& id = "");

    virtual void SetNodeSet(std::list<mitk::DataNode::Pointer> nodeSet, const std::string& id = "");
    virtual std::list<mitk::DataNode::Pointer> GetNodeSet(const std::string& id = "");
    virtual mitk::DataNode::Pointer GetCurrentNode(const std::string& id = "");
    virtual void ClearNodeSet(const std::string& id = "");
private:
    void RelateDataStorage(mitk::DataStorage::Pointer pDataStorage);
    void OnNodeRemoved(const mitk::DataNode *node);
    void OnNodeAdded(const mitk::DataNode* node);
    void OnNodeChanged(const mitk::DataNode* node);
    void OnNodeDeleted(const mitk::DataNode* node);
    void OnNodeInteractorChanged(const mitk::DataNode* node);

    typedef std::map<std::string, mitk::DataStorage::Pointer> DataStorageMapType;
    DataStorageMapType m_DataStorageMap;
    std::string m_DefaultDataStorageID;

    std::vector<mitk::DataNode::Pointer> m_SelectedNodes;
    std::list<mitk::DataNode::Pointer> m_NodeSet;
    QF::IQF_Main* m_pMain;
};

#endif // CQF_MitkDataStorage_h__
