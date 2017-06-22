#ifndef MitkPluginView_h__
#define MitkPluginView_h__

#include "PluginView.h"

#include "mitkRenderingManager.h"
#include <QList>
#include <mitkDataNode.h>
#include "MitkMain/IQF_MitkRenderWindow.h"
#include "MitkMain/IQF_MitkDataManager.h"
#include "MitkMain/IQF_MitkReference.h"
#include "iqf_main.h"

//predicate
#include "mitkSurface.h"
#include "mitkImage.h"
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>

//Qt
#include <QApplication> 

class IQF_MitkDataManager;
class IQF_MitkRenderWindow;

class MitkPluginView : public PluginView
{
public:
    MitkPluginView(QF::IQF_Main* pMain):PluginView(pMain)
	{
		m_pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
		m_pMitkRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
		m_pMitkReferences = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);
	}
    ~MitkPluginView() {}
protected:
    void RequestRenderWindowUpdate(mitk::RenderingManager::RequestType requestType = mitk::RenderingManager::REQUEST_UPDATE_ALL)
    {
        if (m_pMitkRenderWindow)
        {
            if (m_pMitkRenderWindow->GetRenderingManager())
            {
                m_pMitkRenderWindow->GetRenderingManager()->RequestUpdateAll(requestType);
            }       
        }
    }
    mitk::DataStorage::Pointer GetDataStorage()
    {
        return m_pMitkDataManager->GetDataStorage();
    }
    QList<mitk::DataNode::Pointer> GetCurrentSelection()
	{
		QList<mitk::DataNode::Pointer> qlistNodes;
		CastFromStdNodesToQListNodes(m_pMitkDataManager->GetSelectedNodes(), qlistNodes);
		return qlistNodes;
	}
    void BusyCursorOn()
    {
        QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    }
    void BusyCursorOff()
    {
        QApplication::restoreOverrideCursor();
    }
    static void CastFromStdNodesToQListNodes(std::vector<mitk::DataNode::Pointer>& stdNodes, QList<mitk::DataNode::Pointer>& qlistNodes)
	{
		qlistNodes.clear();
		for (int i = 0; i < stdNodes.size(); i++)
		{
			qlistNodes.append(stdNodes.at(i));
		}
	}
    static mitk::NodePredicateBase::Pointer CreatePredicate(int type)
    {
        auto imageType = mitk::TNodePredicateDataType<mitk::Image>::New();
        auto surfaceType = mitk::TNodePredicateDataType<mitk::Surface>::New();
        auto nonHelperObject = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"));
        auto isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
        auto isSegmentation = mitk::NodePredicateProperty::New("segmentation", mitk::BoolProperty::New(true));
        auto isBinaryOrSegmentation = mitk::NodePredicateOr::New(isBinary, isSegmentation);

        mitk::NodePredicateBase::Pointer returnValue;

        switch (type)
        {
        case 1:
            returnValue = imageType.GetPointer();
            break;
        case 2:
            returnValue = surfaceType.GetPointer();
            break;
        default:
            assert(false && "Unknown predefined predicate!");
            return nullptr;
        }

        return mitk::NodePredicateAnd::New(returnValue, nonHelperObject).GetPointer();
    }
protected:
    IQF_MitkDataManager* m_pMitkDataManager;
    IQF_MitkRenderWindow* m_pMitkRenderWindow;
	IQF_MitkReference* m_pMitkReferences;
};


#endif // MitkPluginView_h__
