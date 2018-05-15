#include "MitkPluginView.h"
#include "Res/R.h"

#include "MitkMain/mitk_main_msg.h"

#include "mitkRenderingManager.h"
#include <QList>

#include "qf_log.h"

//Qt
#include <QApplication> 

class MitkPluginViewPrivate
{
public:
    MitkPluginViewPrivate(): m_pMitkDataManager(NULL),m_pMitkRenderWindow(NULL),m_pMitkReferences(NULL), m_dataStorageID(""){}
    IQF_MitkDataManager * m_pMitkDataManager;
    IQF_MitkRenderWindow* m_pMitkRenderWindow;
    IQF_MitkReference* m_pMitkReferences;
    std::string m_dataStorageID;
};


MitkPluginView::MitkPluginView() :PluginView()
{
    base = new MitkPluginViewPrivate;
}

MitkPluginView::MitkPluginView(QF::IQF_Main* pMain):PluginView(pMain)
{
    base = new MitkPluginViewPrivate;
    SetMainPtr(pMain);
}

MitkPluginView::~MitkPluginView() 
{
    delete base;
}

IQF_MitkRenderWindow * MitkPluginView::GetMitkRenderWindowInterface()
{
    return base->m_pMitkRenderWindow;
}
IQF_MitkDataManager* MitkPluginView::GetMitkDataManagerInterface()
{
    return base->m_pMitkDataManager;
}
IQF_MitkReference* MitkPluginView::GetMitkReferenceInterface()
{
    return base->m_pMitkReferences;
}

void MitkPluginView::SetMainPtr(QF::IQF_Main* pMain)
{
    m_pMain = pMain;
    base->m_pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    base->m_pMitkRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
    base->m_pMitkReferences = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);
}
void MitkPluginView::InitResource()
{
    if (HasAttribute("datastorage"))
    {
        base->m_dataStorageID = GetAttribute("datastorage");
        if (!GetDataStorage())
        {
            QF_WARN << "DataStorage with id " << base->m_dataStorageID << " has not been initialized! ";
            return;
        }
    }
    if (HasAttribute("datamanager"))
    {
        QF::IQF_Subject* pSubject = GetMitkDataManagerInterface()->GetDataManagerSubject(GetAttribute("datamanager"));
        if (pSubject)
        {
            pSubject->Attach(this);
        }
    }
    PluginView::InitResource();
}

void MitkPluginView::RequestRenderWindowUpdate(mitk::RenderingManager::RequestType requestType)
{
    if (GetMitkRenderWindowInterface())
    {
        if (GetMitkRenderWindowInterface()->GetRenderingManager())
        {
            GetMitkRenderWindowInterface()->GetRenderingManager()->RequestUpdateAll(requestType);
        }       
    }
}

mitk::DataStorage::Pointer MitkPluginView::GetDataStorage()
{
    return  base->m_pMitkDataManager->GetDataStorage(base->m_dataStorageID);
}
mitk::DataStorage::Pointer MitkPluginView::GetDataStorage() const
{
    return base->m_pMitkDataManager->GetDataStorage(base->m_dataStorageID);
}
void MitkPluginView::SetDataStorageID(const char* dataStorageID)
{
    base->m_dataStorageID = dataStorageID;
}
const char* MitkPluginView::GetDataStorageID()
{
    return base->m_dataStorageID.c_str();
}
QList<mitk::DataNode::Pointer> MitkPluginView::GetCurrentSelection()
{
	QList<mitk::DataNode::Pointer> qlistNodes;
	CastFromStdNodesToQListNodes(GetMitkDataManagerInterface()->GetSelectedNodes(), qlistNodes);
	return qlistNodes;
}

void MitkPluginView::BusyCursorOn()
{
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
}

void MitkPluginView::BusyCursorOff()
{
    QApplication::restoreOverrideCursor();
}

QVariant MitkPluginView::GetGuiProperty(const char* guiId,const char* propertyID)
{
    QObject* obj = (QObject*)R::Instance()->getObjectFromGlobalMap(guiId);
    if (obj)
    {
        return obj->property(propertyID);
    }
    else
    {
        return QVariant(NULL);
    }
}

bool MitkPluginView::SetGuiProperty(const char* guiId, const char* propertyID, const QVariant& value)
{
    QObject* obj = (QObject*)R::Instance()->getObjectFromGlobalMap(guiId);
    if (obj)
    {
        obj->setProperty(propertyID, value);
        return true;
    }
    else
    {
        return false;
    }

}
void MitkPluginView::CastFromStdNodesToQListNodes(std::vector<mitk::DataNode::Pointer>& stdNodes, QList<mitk::DataNode::Pointer>& qlistNodes)
{
	qlistNodes.clear();
	for (int i = 0; i < stdNodes.size(); i++)
	{
		qlistNodes.append(stdNodes.at(i));
	}
}
mitk::NodePredicateBase::Pointer MitkPluginView::CreatePredicate(PredicateType type)
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

mitk::DataNode::Pointer MitkPluginView::ImportVTKImage(vtkImageData* data, const char* name, mitk::DataNode* parentNode, mitk::BaseGeometry* geometry)
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
    return node;
}

mitk::DataNode::Pointer MitkPluginView::ImportVtkPolyData(vtkPolyData* polydata, const char* name, mitk::DataNode* parentNode)
{
    mitk::Surface::Pointer surface = mitk::Surface::New();
    surface->SetVtkPolyData(polydata);
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(surface);
    node->SetName(name);

    GetDataStorage()->Add(node, parentNode);
    return node;

}

mitk::NodePredicateBase::Pointer MitkPluginView::CreateImagePredicate()
{
    return mitk::NodePredicateAnd::New(mitk::TNodePredicateDataType<mitk::Image>::New(),
        mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
}
mitk::NodePredicateBase::Pointer MitkPluginView::CreateSurfacePredicate()
{
    return mitk::NodePredicateAnd::New(mitk::TNodePredicateDataType<mitk::Surface>::New(),
        mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
}
mitk::NodePredicateBase::Pointer MitkPluginView::CreatePointSetPredicate()
{
    return mitk::NodePredicateAnd::New(mitk::NodePredicateDataType::New("PointSet"),
        mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
}

