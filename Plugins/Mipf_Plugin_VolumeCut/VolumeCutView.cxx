#include "VolumeCutView.h" 
#include "iqf_main.h"


//mitk
#include "mitkBoundingShapeInteractor.h"
#include "mitkBoundingShapeObjectFactory.h"
//qmitk
#include "QmitkStdMultiWidget.h"
#include "QmitkRenderWindow.h"


//mitkCoreExt
#include "Interactions/FreehandCutInteractor.h"
#include "Interactions/FreehandCutInteractor.h"
#include "Interactions/FreehandVolumeCutImplementation.h"
  
VolumeCutView::VolumeCutView() :MitkPluginView(), m_boxNumber(0)
{

}
 
VolumeCutView::~VolumeCutView() 
{
}
 
void VolumeCutView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Image>::New());

    m_ui.ModelSelector->SetDataStorage(GetDataStorage());
    m_ui.ModelSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Surface>::New());

    connect(m_ui.AddBoxBtn, SIGNAL(clicked()), this, SLOT(AddBox()));
    connect(m_ui.RemoveBoxBtn, SIGNAL(clicked()), this, SLOT(RemoveBox()));
    connect(m_ui.BoxCutBtn, SIGNAL(clicked()), this, SLOT(BoxCut()));
    connect(m_ui.BoxList, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(BoxSelected(QListWidgetItem *, QListWidgetItem *)));

   
    connect(m_ui.ModelCutBtn, SIGNAL(clicked(bool)), this, SLOT(ModelCut(bool)));
    connect(m_ui.UndoBtn, SIGNAL(clicked()), this, SLOT(Undo()));
    connect(m_ui.RedoBtn, SIGNAL(clicked()), this, SLOT(Redo()));

} 

void VolumeCutView::Undo()
{
    if (m_freehandCutInteractor.IsNotNull())
    {
        static_cast<FreehandCutInteractor*>(m_freehandCutInteractor.GetPointer())->Undo();
    }
}

void VolumeCutView::Redo()
{
    if (m_freehandCutInteractor.IsNotNull())
    {
        static_cast<FreehandCutInteractor*>(m_freehandCutInteractor.GetPointer())->Redo();
    }
}

void VolumeCutView::ModelCut(bool b)
{
    mitk::DataNode* node = m_ui.DataSelector->GetSelectedNode();
    if (!node)
    {
        return;
    }
    if (b)
    {
        if (m_freehandCutInteractor.IsNull())
        {
            m_freehandCutInteractor = FreehandCutInteractor::New();
            static_cast<FreehandCutInteractor*>(m_freehandCutInteractor.GetPointer())->SetImplementation(new FreehandVolumeCutImplementation());
            static_cast<FreehandCutInteractor*>(m_freehandCutInteractor.GetPointer())->SetDataStorage(GetDataStorage());
            static_cast<FreehandCutInteractor*>(m_freehandCutInteractor.GetPointer())->SetRenderer(
                m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow4()->GetRenderer()->GetVtkRenderer());

            
            std::string configPath = m_pMain->GetConfigPath();
            configPath.append("/mitk/Interactions/");
            m_freehandCutInteractor->LoadStateMachine(configPath + "FreehandSurfaceCutInteraction.xml");
            m_freehandCutInteractor->SetEventConfig(configPath + "FreehandSurfaceCutConfig.xml");

            m_freehandCutInteractor->SetDataNode(node);
        }
        node->SetDataInteractor(m_freehandCutInteractor);
        static_cast<FreehandCutInteractor*>(m_freehandCutInteractor.GetPointer())->Start();
        RequestRenderWindowUpdate();
    }
    else
    {
        static_cast<FreehandCutInteractor*>(m_freehandCutInteractor.GetPointer())->Finished();
        node->SetDataInteractor(nullptr);
        RequestRenderWindowUpdate();
    }
}
 
WndHandle VolumeCutView::GetPluginHandle() 
{
    return this; 
}

void VolumeCutView::AddBox()
{
    mitk::RegisterBoundingShapeObjectFactory();

    mitk::BaseGeometry::Pointer surfaceGeometry = static_cast<mitk::BaseGeometry*>(m_ui.DataSelector->GetSelectedNode()->GetData()->GetGeometry());

    m_boxNumber++;
    QString boxName = QString("box%1").arg(m_boxNumber);

    mitk::DataNode::Pointer boxNode = mitk::DataNode::New();
    mitk::GeometryData::Pointer pCroppingObject = mitk::GeometryData::New();
    pCroppingObject->SetGeometry(static_cast<mitk::Geometry3D*>(this->InitializeWithSurfaceGeometry(surfaceGeometry)));
    boxNode->SetData(pCroppingObject);
    boxNode->SetProperty("name", mitk::StringProperty::New(boxName.toStdString()));
    boxNode->SetProperty("color", mitk::ColorProperty::New(1.0, 1.0, 1.0));
    boxNode->SetProperty("opacity", mitk::FloatProperty::New(0.6));
    boxNode->SetProperty("layer", mitk::IntProperty::New(99));
    boxNode->AddProperty("handle size factor", mitk::DoubleProperty::New(1.0 / 40.0));
  //  boxNode->SetBoolProperty("helper object", true);
    boxNode->SetBoolProperty("pickable", true);
    boxNode->SetBoolProperty("VolumeBoxCut", true);

    if (!GetDataStorage()->Exists(boxNode))
    {
        GetDataStorage()->Add(boxNode);
        boxNode->SetVisibility(true);

        if (m_boundingShapeInteractor.IsNull())
        {
            std::string configPath = m_pMain->GetConfigPath();
            configPath.append("/mitk/Interactions/");
            m_boundingShapeInteractor = mitk::BoundingShapeInteractor::New();
            bool s = m_boundingShapeInteractor->LoadStateMachine(configPath + "BoundingShapeInteraction.xml", NULL);
            bool s2 = m_boundingShapeInteractor->SetEventConfig(configPath + "BoundingShapeMouseConfig.xml", NULL);
            static_cast<mitk::BoundingShapeInteractor*>(m_boundingShapeInteractor.GetPointer())->SetRotationEnabled(true);
        }
        m_boundingShapeInteractor->SetDataNode(boxNode);
    }

    // Adjust coordinate system by doing a reinit on
    auto tempDataStorage = mitk::DataStorage::SetOfObjects::New();
    tempDataStorage->InsertElement(0, boxNode);

    m_ui.BoxList->addItem(boxName);
}

void VolumeCutView::RemoveBox()
{
    if (m_ui.BoxList->currentRow() < 0)
    {
        return;
    }
    GetDataStorage()->Remove(GetDataStorage()->GetNamedNode(m_ui.BoxList->currentItem()->text().toStdString()));
    m_ui.BoxList->removeItemWidget(m_ui.BoxList->currentItem());
    m_ui.BoxList->takeItem(m_ui.BoxList->currentRow());
    m_ui.BoxList->update();
}

void VolumeCutView::BoxSelected(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
    {
        return;
    }
    m_boundingShapeInteractor->SetDataNode(GetDataStorage()->GetNamedNode(current->text().toStdString()));

}

mitk::Geometry3D::Pointer VolumeCutView::InitializeWithSurfaceGeometry(mitk::BaseGeometry::Pointer geometry)
{
    if (geometry == nullptr)
        mitkThrow() << "Geometry is not valid.";

    auto boundingGeometry = mitk::Geometry3D::New();
    boundingGeometry->SetBounds(geometry->GetBounds());
    boundingGeometry->SetImageGeometry(geometry->GetImageGeometry());
    boundingGeometry->SetOrigin(geometry->GetOrigin());
    boundingGeometry->SetSpacing(geometry->GetSpacing());
    boundingGeometry->SetIndexToWorldTransform(geometry->GetIndexToWorldTransform());
    boundingGeometry->Modified();
    return boundingGeometry;
}

#include <vtkClipPolyData.h>
#include <vtkBox.h>
#include <vtkImplicitFunctionToImageStencil.h>
#include <vtkImageStencil.h>

void VolumeCutView::BoxCut()
{
    mitk::Image* image = static_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());
    if (!image)
    {
        return;
    }
  /*  mitk::Point3D p;
    p[0] = 0;
    p[1] = 0;
    p[2] = 0;

    image->GetGeometry()->SetOrigin(p);*/
    vtkSmartPointer<vtkImageData> tempImage = vtkSmartPointer<vtkImageData>::New();
    tempImage->DeepCopy(image->GetVtkImageData());
    MITK_INFO << "vtkImageData:[" << tempImage->GetBounds()[0]<<" " << tempImage->GetBounds()[1] << " " << tempImage->GetBounds()[2] << " "
        << tempImage->GetBounds()[3] << " " << tempImage->GetBounds()[4] << " " << tempImage->GetBounds()[5] << "]["
        << tempImage->GetBounds()[0] << " " << tempImage->GetOrigin()[1] << " " << tempImage->GetOrigin()[2]<<"]";


    MITK_INFO << image->GetGeometry()->GetIndexToWorldTransform()->GetTranslation();
    MITK_INFO << image->GetGeometry()->GetIndexToWorldTransform()->GetScale();
    vtkMatrix4x4* vm = image->GetGeometry()->GetVtkMatrix();

    MITK_INFO << image->GetGeometry()->GetBounds() << image->GetGeometry()->GetOrigin();

    mitk::NodePredicateProperty::Pointer propertyPredicate = mitk::NodePredicateProperty::New("VolumeBoxCut", mitk::BoolProperty::New(true));
    mitk::DataStorage::SetOfObjects::ConstPointer boxs = GetDataStorage()->GetSubset(propertyPredicate);
    for (mitk::DataStorage::SetOfObjects::const_iterator iter = boxs->begin(); iter != boxs->end(); ++iter)
    {
        mitk::DataNode* node = *iter;
        node->GetData()->UpdateOutputInformation();
        node->Modified();

        mitk::BaseGeometry::BoundsArrayType bound = node->GetData()->GetGeometry()->GetBounds();
        mitk::Point3D origin = node->GetData()->GetGeometry()->GetOrigin();

        MITK_INFO << bound<< origin;
        bound[0] *= vm->GetElement(0, 0);
        bound[1] *= vm->GetElement(0, 0);
        bound[2] *= vm->GetElement(1, 1);
        bound[3] *= vm->GetElement(1, 1);
        bound[4] *= vm->GetElement(2, 2);
        bound[5] *= vm->GetElement(2, 2);
        bound[0] += origin[0];
        bound[1] += origin[0];
        bound[2] += origin[1];
        bound[3] += origin[1];
        bound[4] += origin[2];
        bound[5] += origin[2];


        auto clipFunction = vtkSmartPointer<vtkBox>::New();
        clipFunction->SetBounds(bound.GetDataPointer());

        /*auto transform = vtkSmartPointer<vtkTransform>::New();
        transform->PreMultiply();
        transform->SetMatrix(vm);
        transform->Translate(-tempImage->GetOrigin()[0] + origin[0], -tempImage->GetOrigin()[1] + origin[1], -tempImage->GetOrigin()[2] + origin[2]);
        clipFunction->SetTransform(transform);*/
        

        MITK_INFO<<"Box Bounds"<<clipFunction->GetBounds()[0]<<" " << clipFunction->GetBounds()[1] << " " << clipFunction->GetBounds()[2] 
            << " " << clipFunction->GetBounds()[3] << " " << clipFunction->GetBounds()[4] << " " << clipFunction->GetBounds()[5];



        mitk::Point3D iorigin = image->GetGeometry()->GetOrigin();
        mitk::Vector3D ispacing = image->GetGeometry()->GetSpacing();


        vtkSmartPointer<vtkImplicitFunctionToImageStencil> functionToStencil =
            vtkSmartPointer<vtkImplicitFunctionToImageStencil>::New();
        functionToStencil->SetInput(clipFunction);
        functionToStencil->SetOutputOrigin(tempImage->GetOrigin());
        functionToStencil->SetOutputSpacing(tempImage->GetSpacing());
        functionToStencil->SetOutputWholeExtent(tempImage->GetExtent());
        functionToStencil->Update();

        auto stencilToImage = vtkSmartPointer<vtkImageStencil>::New();
        stencilToImage->SetInputData(tempImage);
        stencilToImage->SetStencilData(functionToStencil->GetOutput());
        if (m_ui.BoxCutInsideOutCheckBox->isChecked())
        {
            stencilToImage->ReverseStencilOff();
        }
        else
        {
            stencilToImage->ReverseStencilOn();
        }  
        stencilToImage->SetBackgroundValue(0.0);
        stencilToImage->Update();
        tempImage->DeepCopy(stencilToImage->GetOutput());

    }
    image->GetVtkImageData()->DeepCopy(tempImage);
    RequestRenderWindowUpdate();

}
