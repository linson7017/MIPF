#ifndef MitkPluginView_h__
#define MitkPluginView_h__

#include "PluginView.h"

#include "mitkRenderingManager.h"
#include <QList>
#include <mitkDataNode.h>
#include "MitkMain/IQF_MitkRenderWindow.h"
#include "MitkMain/IQF_MitkDataManager.h"
<<<<<<< HEAD
=======
#include "MitkMain/IQF_MitkReference.h"
>>>>>>> db763ad513bdbb631b78902d9da606666071e38d
#include "iqf_main.h"

class IQF_MitkDataManager;
class IQF_MitkRenderWindow;

class MitkPluginView : public PluginView
{
public:
    MitkPluginView(QF::IQF_Main* pMain):PluginView(pMain)
	{
		m_pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
		m_pMitkRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
<<<<<<< HEAD
=======
		m_pMitkReferences = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);
>>>>>>> db763ad513bdbb631b78902d9da606666071e38d
	}
    ~MitkPluginView() {}
    void RequestRenderWindowUpdate(mitk::RenderingManager::RequestType requestType = mitk::RenderingManager::REQUEST_UPDATE_ALL)
	{
		if (m_pMitkRenderWindow)
		{
			m_pMitkRenderWindow->GetRenderingManager()->RequestUpdateAll(requestType);
		}
	}
protected:
    QList<mitk::DataNode::Pointer> GetCurrentSelection()
	{
		QList<mitk::DataNode::Pointer> qlistNodes;
		CastFromStdNodesToQListNodes(m_pMitkDataManager->GetSelectedNodes(), qlistNodes);
		return qlistNodes;
	}
    static void CastFromStdNodesToQListNodes(std::vector<mitk::DataNode::Pointer>& stdNodes, QList<mitk::DataNode::Pointer>& qlistNodes)
	{
		qlistNodes.clear();
		for (int i = 0; i < stdNodes.size(); i++)
		{
			qlistNodes.append(stdNodes.at(i));
		}
	}
protected:
    IQF_MitkDataManager* m_pMitkDataManager;
    IQF_MitkRenderWindow* m_pMitkRenderWindow;
<<<<<<< HEAD
=======
	IQF_MitkReference* m_pMitkReferences;
>>>>>>> db763ad513bdbb631b78902d9da606666071e38d
};


#endif // MitkPluginView_h__
