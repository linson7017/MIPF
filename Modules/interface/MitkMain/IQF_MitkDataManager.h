#ifndef IQF_MitkDataStorage_h__
#define IQF_MitkDataStorage_h__

#include "mitkDataStorage.h"

const char QF_MitkMain_DataManager[] = "QF_MitkMain_DataManager";

class IQF_MitkDataManager
{
public:
    virtual void Init() = 0;
    virtual mitk::DataStorage::Pointer GetDataStorage() = 0;
    virtual void SetSelectedNode(std::vector<mitk::DataNode::Pointer> selectNodes) = 0;
    virtual std::vector<mitk::DataNode::Pointer> GetSelectedNodes() = 0;

    virtual void SetNodeSet(std::list<mitk::DataNode::Pointer> nodeSet) = 0;
    virtual std::list<mitk::DataNode::Pointer> GetNodeSet() = 0;
    virtual mitk::DataNode::Pointer GetCurrentNode() = 0;
    virtual void ClearNodeSet() = 0;
};


#endif // IQF_MitkDataStorage_h__