#ifndef MitkPluginView_h__
#define MitkPluginView_h__

#include "PluginView.h"

#include "mitkRenderingManager.h"
#include <QList>
#include <mitkDataNode.h>

class IQF_MitkDataManager;
class IQF_MitkRenderWindow;

class MitkPluginView : public PluginView
{
public:
    MitkPluginView(QF::IQF_Main* pMain);
    ~MitkPluginView() {}
    void RequestRenderWindowUpdate(mitk::RenderingManager::RequestType requestType = mitk::RenderingManager::REQUEST_UPDATE_ALL);
protected:
    QList<mitk::DataNode::Pointer> GetCurrentSelection();
    static void CastFromStdNodesToQListNodes(std::vector<mitk::DataNode::Pointer>& stdNodes, QList<mitk::DataNode::Pointer>& qlistNodes);
protected:
    IQF_MitkDataManager* m_pMitkDataManager;
    IQF_MitkRenderWindow* m_pMitkRenderWindow;
};


#endif // MitkPluginView_h__
