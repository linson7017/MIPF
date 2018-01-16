#ifndef MitkPluginView_h__
#define MitkPluginView_h__

#include "PluginView.h"
#include "Res/R.h"

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
    enum PredicateType
    {
         Image,
         Surface,
         Binary,
         Segmentation,
         BinaryOrSegmentation
    };
    MitkPluginView() :PluginView()
    {
    }
    MitkPluginView(QF::IQF_Main* pMain):PluginView(pMain)
	{
        SetMainPtr(pMain);
	}
    ~MitkPluginView() {}
    virtual void SetMainPtr(QF::IQF_Main* pMain)
    {
        m_pMain = pMain;
        m_pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        m_pMitkRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
        m_pMitkReferences = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);
    }
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
    QVariant GetGuiProperty(const char* guiId,const char* propertyID)
    {
        if (m_pR)
        {
            QObject* obj = (QObject*)m_pR->getObjectFromGlobalMap(guiId);
            if (obj)
            {
                return obj->property(propertyID);
            }
            else
            {
                return QVariant(NULL);
            }
        }
        else
        {
            return QVariant(NULL);
        }
    }
    bool SetGuiProperty(const char* guiId, const char* propertyID, const QVariant& value)
    {
        if (m_pR)
        {
            QObject* obj = (QObject*)m_pR->getObjectFromGlobalMap(guiId);
            if (obj)
            {
                obj->setProperty(propertyID, value);
            }
        }
        else
        {
            return false;
        }
    }
    static void CastFromStdNodesToQListNodes(std::vector<mitk::DataNode::Pointer>& stdNodes, QList<mitk::DataNode::Pointer>& qlistNodes)
	{
		qlistNodes.clear();
		for (int i = 0; i < stdNodes.size(); i++)
		{
			qlistNodes.append(stdNodes.at(i));
		}
	}
    static mitk::NodePredicateBase::Pointer CreatePredicate(PredicateType type)
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
        case Image:
            returnValue = imageType.GetPointer();
            break;
        case Surface:
            returnValue = surfaceType.GetPointer();
            break;
        case Binary:
            returnValue = isBinary.GetPointer();
            break;
        case Segmentation:
            returnValue = isSegmentation.GetPointer();
            break;
        case BinaryOrSegmentation:
            returnValue = isBinaryOrSegmentation.GetPointer();
            break;
        default:
            assert(false && "Unknown predefined predicate!");
            return nullptr;
        }

        return mitk::NodePredicateAnd::New(returnValue, nonHelperObject).GetPointer();
    }

    void ImportVTKImage(vtkImageData* data, const char* name, mitk::DataNode* parentNode = nullptr, mitk::BaseGeometry* geometry = nullptr)
    {
        mitk::DataNode::Pointer node = mitk::DataNode::New();
        mitk::Image::Pointer image = mitk::Image::New();
        image->Initialize(data);
        if (geometry)
        {
            image->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(geometry->GetVtkMatrix());
            image->GetGeometry()->SetOrigin(geometry->GetOrigin());
        }
        image->SetVolume(data->GetScalarPointer());
        node->SetData(image);
        node->SetName(name);
        GetDataStorage()->Add(node, parentNode);
    }
    template<class TImageType>
    void ImportITKImage(TImageType* itkImage, const char* name, mitk::DataNode* parentNode = nullptr)
    {
        mitk::Image::Pointer image;
        mitk::CastToMitkImage(itkImage, image);
        mitk::DataNode::Pointer node = mitk::DataNode::New();
        node->SetData(image);
        node->SetName(name);

        GetDataStorage()->Add(node, parentNode);
    }
    mitk::DataNode::Pointer ImportVtkPolyData(vtkPolyData* polydata, const char* name, mitk::DataNode* parentNode = nullptr)
    {
        mitk::Surface::Pointer surface = mitk::Surface::New();
        surface->SetVtkPolyData(polydata);
        mitk::DataNode::Pointer node = mitk::DataNode::New();
        node->SetData(surface);
        node->SetName(name);

        GetDataStorage()->Add(node, parentNode);
        return node;

    }
protected:
    IQF_MitkDataManager* m_pMitkDataManager;
    IQF_MitkRenderWindow* m_pMitkRenderWindow;
	IQF_MitkReference* m_pMitkReferences;
};


#endif // MitkPluginView_h__
