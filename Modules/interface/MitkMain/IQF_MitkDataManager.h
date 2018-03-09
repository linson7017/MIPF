#ifndef IQF_MitkDataStorage_h__
#define IQF_MitkDataStorage_h__

#include "mitkDataStorage.h"

const char QF_MitkMain_DataManager[] = "QF_MitkMain_DataManager";

class IQF_MitkDataManager
{
public:
    virtual void Init() = 0;
    virtual void SetDataStorage(mitk::DataStorage::Pointer dataStorage,const std::string& id="")=0;
    virtual mitk::DataStorage::Pointer GetDataStorage(const std::string& id = "") = 0;
    virtual void SetSelectedNode(std::vector<mitk::DataNode::Pointer> selectNodes, const std::string& id = "") = 0;
    virtual std::vector<mitk::DataNode::Pointer> GetSelectedNodes(const std::string& id = "") = 0;

    virtual void SetNodeSet(std::list<mitk::DataNode::Pointer> nodeSet, const std::string& id = "") = 0;
    virtual std::list<mitk::DataNode::Pointer> GetNodeSet(const std::string& id = "") = 0;
    virtual mitk::DataNode::Pointer GetCurrentNode(const std::string& id = "") = 0;
    virtual void ClearNodeSet(const std::string& id = "") = 0;
};


#endif // IQF_MitkDataStorage_h__