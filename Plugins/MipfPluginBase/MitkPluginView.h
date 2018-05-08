#ifndef MitkPluginView_h__
#define MitkPluginView_h__

#include "PluginView.h"
#include "MitkMain/mitk_main_msg.h"

#include <QList>
#include <QVariant>
#include <mitkDataNode.h>
#include "MitkMain/IQF_MitkRenderWindow.h"
#include "MitkMain/IQF_MitkDataManager.h"
#include "MitkMain/IQF_MitkReference.h"


//predicate
#include "mitkSurface.h"
#include "mitkImage.h"
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>

//mitk
#include "mitkRenderingManager.h"
#include "mitkNodePredicateBase.h"

class IQF_MitkDataManager;
class IQF_MitkRenderWindow;
class vtkImageData;
class vtkPolyData;
class MitkPluginViewPrivate;

class QF_API  MitkPluginView : public PluginView
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
    MitkPluginView();
    MitkPluginView(QF::IQF_Main* pMain);
    ~MitkPluginView();
    virtual void SetMainPtr(QF::IQF_Main* pMain);
    virtual void InitResource();
protected:
    IQF_MitkRenderWindow * GetMitkRenderWindowInterface();
    IQF_MitkDataManager* GetMitkDataManagerInterface();
    IQF_MitkReference* GetMitkReferenceInterface();
    void RequestRenderWindowUpdate(mitk::RenderingManager::RequestType requestType = mitk::RenderingManager::REQUEST_UPDATE_ALL);
    mitk::DataStorage::Pointer GetDataStorage();
    mitk::DataStorage::Pointer GetDataStorage() const;
    void SetDataStorageID(const char* dataStorageID);
    const char* GetDataStorageID();
    QList<mitk::DataNode::Pointer> GetCurrentSelection();
    void BusyCursorOn();
    void BusyCursorOff();
    QVariant GetGuiProperty(const char* guiId, const char* propertyID);
    bool SetGuiProperty(const char* guiId, const char* propertyID, const QVariant& value);
    static void CastFromStdNodesToQListNodes(std::vector<mitk::DataNode::Pointer>& stdNodes, QList<mitk::DataNode::Pointer>& qlistNodes);
    static mitk::NodePredicateBase::Pointer CreatePredicate(PredicateType type);

    mitk::DataNode::Pointer ImportVTKImage(vtkImageData* data, const char* name, mitk::DataNode* parentNode = nullptr, mitk::BaseGeometry* geometry = nullptr);
    
    template<class TImageType>
    mitk::DataNode::Pointer MitkPluginView::ImportITKImage(TImageType* itkImage, const char* name, mitk::DataNode* parentNode=nullptr)
    {
        mitk::Image::Pointer image;
        mitk::CastToMitkImage(itkImage, image);
        mitk::DataNode::Pointer node = mitk::DataNode::New();
        node->SetData(image);
        node->SetName(name);

        GetDataStorage()->Add(node, parentNode);
        return node;
    }
   
    mitk::DataNode::Pointer ImportVtkPolyData(vtkPolyData* polydata, const char* name, mitk::DataNode* parentNode = nullptr);

    static mitk::NodePredicateBase::Pointer CreateImagePredicate();
    static mitk::NodePredicateBase::Pointer CreateSurfacePredicate();
    static mitk::NodePredicateBase::Pointer CreatePointSetPredicate();
private:
    MitkPluginViewPrivate* base;
};


#endif // MitkPluginView_h__
