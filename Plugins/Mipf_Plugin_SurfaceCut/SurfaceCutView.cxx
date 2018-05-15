#include "SurfaceCutView.h"
#include "iqf_main.h"
#include "Res/R.h"

#include "Rendering/ObjectFactoryExt.h"
#include "Rendering/TexturedVtkMapper3D.h"
#include "Interactions/FreehandCutInteractor.h"
#include "Interactions/FreehandCutInteractor.h"
#include "Interactions/FreehandSurfaceCutImplementation.h"
#include "Interactions/GeometryInteractor.h"

#include "MitkMain/IQF_MitkRenderWindow.h"

//qmitk
#include "QmitkStdMultiWidget.h"
#include "QmitkRenderWindow.h"

//mitk
#include "mitkBoundingShapeInteractor.h"
#include "mitkBoundingShapeObjectFactory.h"
#include <mitkAffineBaseDataInteractor3D.h>

//micro services
#include <usModuleRegistry.h>
#include <usGetModuleContext.h>

//qt
#include <QInputDialog>
#include <QMessageBox>

SurfaceCutView::SurfaceCutView() :MitkPluginView(),
m_freehandCutInteractor(nullptr), m_boundingShapeInteractor(nullptr), m_geometryInteractor(nullptr), m_boxNumber(0)
{

}

SurfaceCutView::~SurfaceCutView()
{

}

void SurfaceCutView::CreateView()
{
    RegisterObjectFactoryExt();
    m_pMain->Attach(this);
    m_ui.setupUi(this);

    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(CreateSurfacePredicate());

    //freehand cut
    connect(m_ui.FreehandCutBtn, SIGNAL(clicked(bool)), this, SLOT(FreehandCut(bool)));
    connect(m_ui.UndoBtn, SIGNAL(clicked()), this, SLOT(Undo()));
    connect(m_ui.RedoBtn, SIGNAL(clicked()), this, SLOT(Redo()));
    connect(m_ui.InsideOutCheckBox, SIGNAL(clicked(bool)), this, SLOT(InsideOut(bool)));

    //box cut
    connect(m_ui.AddBoxBtn, SIGNAL(clicked()), this, SLOT(AddBox()));
    connect(m_ui.RemoveBoxBtn, SIGNAL(clicked()), this, SLOT(RemoveBox()));
    connect(m_ui.BoxCutBtn, SIGNAL(clicked()), this, SLOT(BoxCut()));
    connect(m_ui.BoxList, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(BoxSelected(QListWidgetItem *, QListWidgetItem *)));
    


    //geometry cut
    m_ui.GeometryList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(m_ui.AddGeometryBtn, SIGNAL(clicked()), this, SLOT(AddGeometry()));
    connect(m_ui.RemoveGeometryBtn, SIGNAL(clicked()), this, SLOT(RemoveGeometry()));
    connect(m_ui.GeometryCutBtn, SIGNAL(clicked()), this, SLOT(GeometryCut()));

    connect(m_ui.AnchorOriginRadioBtn, SIGNAL(clicked(bool)), this, SLOT(OnOriginPointRadioButton(bool)));
    connect(m_ui.AnchorCenterRadioBtn, SIGNAL(clicked(bool)), this, SLOT(OnCenterPointRadioButton(bool)));
    connect(m_ui.GeometryList, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(GeometrySelected(QListWidgetItem *, QListWidgetItem *)));
    connect(m_ui.GeometryList, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(GeometryChanged(QListWidgetItem *)));
}

/**************Freehand Cut********************/
void SurfaceCutView::InsideOut(bool flag)
{
    if (m_pImplementation)
    {
        m_pImplementation->SetInsideOut(flag);
    }
}

void SurfaceCutView::FreehandCut(bool enableCut)
{
    mitk::DataNode* node = m_ui.DataSelector->GetSelectedNode();
    if (!node)
    {
        return;
    }
    if (enableCut)
    {
        //create implementation
        if (!m_pImplementation)
        {
            m_pImplementation = new FreehandSurfaceCutImplementation();
            m_pImplementation->SetInsideOut(m_ui.InsideOutCheckBox->isChecked());     
        }
        m_pImplementation->Init(node);
        //create interactor
        if (m_freehandCutInteractor.IsNull())
        {
            m_freehandCutInteractor = FreehandCutInteractor::New();         
            //connect 
            FreehandCutInteractor* pInteractor = static_cast<FreehandCutInteractor*>(m_freehandCutInteractor.GetPointer());
            pInteractor->UndoEvent.AddListener(mitk::MessageDelegate<CutImplementation>(m_pImplementation, &CutImplementation::Undo));
            pInteractor->RedoEvent.AddListener(mitk::MessageDelegate<CutImplementation>(m_pImplementation, &CutImplementation::Redo));
            pInteractor->FinishedEvent.AddListener(mitk::MessageDelegate<CutImplementation>(m_pImplementation, &CutImplementation::Finished));
            pInteractor->ResetEvent.AddListener(mitk::MessageDelegate<CutImplementation>(m_pImplementation, &CutImplementation::Reset));
            pInteractor->ReleaseEvent.AddListener(mitk::MessageDelegate<CutImplementation>(m_pImplementation, &CutImplementation::Release));
            pInteractor->ProcessEvent.AddListener(mitk::MessageDelegate2<CutImplementation, vtkObject*, mitk::InteractionEvent *>(m_pImplementation, &CutImplementation::Cut));

            //
            static_cast<FreehandCutInteractor*>(m_freehandCutInteractor.GetPointer())->SetDataStorage(GetDataStorage());
            static_cast<FreehandCutInteractor*>(m_freehandCutInteractor.GetPointer())->SetRenderer(
                GetMitkRenderWindowInterface()->GetMitkStdMultiWidget()->GetRenderWindow4()->GetRenderer()->GetVtkRenderer());

            std::string configpath = m_pMain->GetConfigPath();
            configpath.append("/mitk/Interactions/");
            m_freehandCutInteractor->LoadStateMachine(configpath + "FreehandSurfaceCutInteraction.xml");
            m_freehandCutInteractor->SetEventConfig(configpath + "FreehandSurfaceCutConfig.xml");
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

void SurfaceCutView::Undo()
{
    if (m_freehandCutInteractor)
    {
        static_cast<FreehandCutInteractor*>(m_freehandCutInteractor.GetPointer())->Undo();
    }
}

void SurfaceCutView::Redo()
{
    if (m_freehandCutInteractor)
    {
        static_cast<FreehandCutInteractor*>(m_freehandCutInteractor.GetPointer())->Redo();
    }
}

/**************Box Cut********************/
void SurfaceCutView::AddBox()
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
    boxNode->SetBoolProperty("helper object", true);
    boxNode->SetBoolProperty("pickable", true);
    boxNode->SetBoolProperty("BoxCut", true);

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

void SurfaceCutView::RemoveBox()
{
    if (m_ui.BoxList->currentRow()<0)
    {
        return;
    }
    GetDataStorage()->Remove(GetDataStorage()->GetNamedNode(m_ui.BoxList->currentItem()->text().toStdString()));
    m_ui.BoxList->removeItemWidget(m_ui.BoxList->currentItem());
    m_ui.BoxList->takeItem(m_ui.BoxList->currentRow());
    m_ui.BoxList->update();
}

void SurfaceCutView::BoxSelected(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
    {
        return;
    }
    m_boundingShapeInteractor->SetDataNode(GetDataStorage()->GetNamedNode(current->text().toStdString()));

}

mitk::Geometry3D::Pointer SurfaceCutView::InitializeWithSurfaceGeometry(mitk::BaseGeometry::Pointer geometry)
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

void SurfaceCutView::BoxCut()
{
    mitk::Surface* surface = static_cast<mitk::Surface*>(m_ui.DataSelector->GetSelectedNode()->GetData());
    if (!surface)
    {
        return;
    }
    auto originPolydata = vtkSmartPointer<vtkPolyData>::New();
    originPolydata->DeepCopy(surface->GetVtkPolyData());

    mitk::NodePredicateProperty::Pointer propertyPredicate = mitk::NodePredicateProperty::New("BoxCut", mitk::BoolProperty::New(true));
    mitk::DataStorage::SetOfObjects::ConstPointer boxs = GetDataStorage()->GetSubset(propertyPredicate);
    for (mitk::DataStorage::SetOfObjects::const_iterator iter = boxs->begin(); iter != boxs->end(); ++iter)
    {
        mitk::DataNode* node = *iter;
        node->GetData()->UpdateOutputInformation();
        node->Modified();

        mitk::BaseGeometry::BoundsArrayType bound = node->GetData()->GetGeometry()->GetBounds();
        mitk::Point3D origin = node->GetData()->GetGeometry()->GetOrigin();
        //QF_INFO << node->GetData()->GetGeometry() << ":" << bound << ". Origin:" << origin;

        bound[0] = bound[0] + origin[0];
        bound[1] = bound[1] + origin[0];
        bound[2] = bound[2] + origin[1];
        bound[3] = bound[3] + origin[1];
        bound[4] = bound[4] + origin[2];
        bound[5] = bound[5] + origin[2];

        auto clipFunction = vtkSmartPointer<vtkBox>::New();
        clipFunction->SetBounds(bound.GetDataPointer());

        vtkSmartPointer<vtkClipPolyData> clipper =
            vtkSmartPointer<vtkClipPolyData>::New();
        clipper->SetInputData(originPolydata);
        clipper->SetClipFunction(clipFunction);
        clipper->SetInsideOut(m_ui.BoxCutInsideOutCheckBox->isChecked());
        clipper->Update();

        originPolydata->DeepCopy(clipper->GetOutput());
    }
    surface->SetVtkPolyData(originPolydata);
    RequestRenderWindowUpdate();

}


void SurfaceCutView::AddGeometry()
{
    bool ok = false;
    QString name = QInputDialog::getText(QApplication::activeWindow()
        , "Add Cut Geometry...", "Enter name of the geometry", QLineEdit::Normal, "Cut Geometry", &ok);
    if (!ok || name.isEmpty())
        return;
    if (m_geometryClipFunctions.count(name.toStdString()))
    {
        QMessageBox box;
        box.setText("Name has existed ! Please rename another name !");
        return;
    }


    mitk::DataNode::Pointer node = mitk::DataNode::New();
    mitk::Surface::Pointer surface = mitk::Surface::New();
    
    mitk::BaseGeometry::Pointer surfaceGeometry = static_cast<mitk::BaseGeometry*>(m_ui.DataSelector->GetSelectedNode()->GetData()->GetGeometry());

    vtkSmartPointer<vtkImplicitFunction> cutFunction;
    surface->SetVtkPolyData(CreateGeometry(surfaceGeometry, cutFunction));
    node->SetData(surface);
    node->SetName(name.toStdString());
    node->SetBoolProperty("geometry cut", true);
    node->SetBoolProperty("helper object", true);
    m_geometryClipFunctions[name.toStdString()] = cutFunction;
    GetDataStorage()->Add(node);


    if (m_geometryInteractor.IsNull())
    {
        m_geometryInteractor = GeometryInteractor::New();
        if (0)
        {
            m_geometryInteractor->LoadStateMachine("AffineInteraction3D.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
            m_geometryInteractor->SetEventConfig("AffineKeyConfig.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
        }
        else if (1)
        {
            bool s = m_geometryInteractor->LoadStateMachine("AffineInteraction3D.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
            s = m_geometryInteractor->SetEventConfig("AffineMouseConfig.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
        }
    }
    node->SetBoolProperty("pickable", true);
    //node->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point X", node->GetData()->GetGeometry()->GetCenter()[0]);
    //node->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point Y", node->GetData()->GetGeometry()->GetCenter()[1]);
    //node->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point Z", node->GetData()->GetGeometry()->GetCenter()[2]);
    //m_geometryInteractor->SetDataNode(node);

    QListWidgetItem* item = new QListWidgetItem(name, m_ui.GeometryList);
    item->setCheckState(Qt::Checked);
    m_ui.GeometryList->addItem(item);
    m_ui.GeometryList->setCurrentItem(item);
}

void SurfaceCutView::RemoveGeometry()
{
    
    m_ui.BoxList->update();
    QList<QListWidgetItem*> selectedItems = m_ui.GeometryList->selectedItems();
    foreach (QListWidgetItem* item , selectedItems)
    {
        GetDataStorage()->Remove(GetDataStorage()->GetNode(
            mitk::NodePredicateAnd::New(
                mitk::NodePredicateProperty::New("geometry cut", mitk::BoolProperty::New(true)),
                mitk::NodePredicateProperty::New("name", mitk::StringProperty::New(item->text().toStdString()))
            )
        ));
        m_ui.GeometryList->takeItem(m_ui.GeometryList->row(item));
        m_geometryClipFunctions.erase(item->text().toStdString());
    }   
}

void SurfaceCutView::OnOriginPointRadioButton(bool)
{
    if (m_geometryInteractor.IsNotNull())
    {
        mitk::DataNode* node = m_geometryInteractor->GetDataNode();
        if (node)
        {
            m_geometryInteractor->GetDataNode()->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point X", node->GetData()->GetGeometry()->GetOrigin()[0]);
            m_geometryInteractor->GetDataNode()->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point Y", node->GetData()->GetGeometry()->GetOrigin()[1]);
            m_geometryInteractor->GetDataNode()->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point Z", node->GetData()->GetGeometry()->GetOrigin()[2]);
        }     
    }
}

void SurfaceCutView::OnCenterPointRadioButton(bool)
{
    if (m_geometryInteractor.IsNotNull())
    {
        mitk::DataNode* node = m_geometryInteractor->GetDataNode();
        if (node)
        {
            m_geometryInteractor->GetDataNode()->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point X", node->GetData()->GetGeometry()->GetCenter()[0]);
            m_geometryInteractor->GetDataNode()->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point Y", node->GetData()->GetGeometry()->GetCenter()[1]);
            m_geometryInteractor->GetDataNode()->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point Z", node->GetData()->GetGeometry()->GetCenter()[2]);
        }
    }
}

void SurfaceCutView::GeometryCut()
{
    mitk::Surface* surface = static_cast<mitk::Surface*>(m_ui.DataSelector->GetSelectedNode()->GetData());
    if (!surface)
    {
        return;
    }
    auto originPolydata = vtkSmartPointer<vtkPolyData>::New();
    originPolydata->DeepCopy(surface->GetVtkPolyData());

    mitk::NodePredicateProperty::Pointer propertyPredicate = mitk::NodePredicateProperty::New("geometry cut", mitk::BoolProperty::New(true));
    mitk::DataStorage::SetOfObjects::ConstPointer geometries = GetDataStorage()->GetSubset(propertyPredicate);
    for (mitk::DataStorage::SetOfObjects::const_iterator iter = geometries->begin(); iter != geometries->end(); ++iter)
    {
        mitk::DataNode* node = *iter;
        vtkSmartPointer<vtkImplicitFunction> clipFunction = m_geometryClipFunctions[node->GetName()];
        if (!clipFunction)
        {
            continue;
        }

        node->GetData()->UpdateOutputInformation();
        node->Modified();
        if (clipFunction)
        {
            clipFunction->SetTransform(node->GetData()->GetUpdatedGeometry()->GetVtkTransform()->GetInverse());
            vtkSmartPointer<vtkClipPolyData> clipper =
                vtkSmartPointer<vtkClipPolyData>::New();
            clipper->SetInputData(originPolydata);
            clipper->SetClipFunction(clipFunction);
            clipper->Update();
            originPolydata->DeepCopy(clipper->GetOutput());
            surface->SetVtkPolyData(originPolydata);
        }
    }
    RequestRenderWindowUpdate();
}

void SurfaceCutView::GeometrySelected(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
    {
        return;
    }
    mitk::NodePredicateAnd::Pointer predicate = mitk::NodePredicateAnd::New(
        mitk::NodePredicateProperty::New("geometry cut", mitk::BoolProperty::New(true)),
        mitk::NodePredicateProperty::New("name", mitk::StringProperty::New(current->text().toStdString())));
    m_geometryInteractor->SetDataNode(GetDataStorage()->GetNode(predicate));
    RequestRenderWindowUpdate();
}

void SurfaceCutView::GeometryChanged(QListWidgetItem *current)
{
    if (!current)
    {
        return;
    }
    mitk::NodePredicateAnd::Pointer predicate = mitk::NodePredicateAnd::New(
        mitk::NodePredicateProperty::New("geometry cut", mitk::BoolProperty::New(true)),
        mitk::NodePredicateProperty::New("name", mitk::StringProperty::New(current->text().toStdString())));
    GetDataStorage()->GetNode(predicate)->SetVisibility(current->checkState() == Qt::Checked);
    RequestRenderWindowUpdate();
}

#include "vtkSphere.h"
#include "vtkSphereSource.h"
#include "vtkBox.h"
#include "vtkCubeSource.h"
#include "vtkCylinderSource.h"

vtkSmartPointer<vtkPolyData> SurfaceCutView::CreateGeometry(mitk::BaseGeometry::Pointer geometry, vtkSmartPointer<vtkImplicitFunction>& geometryClipFunction)
{
    QString shape = m_ui.GeometrySelector->currentText();
    auto outputPolyData = vtkSmartPointer<vtkPolyData>::New();
    if (shape.compare("Sphere",Qt::CaseSensitive)==0)
    {
        geometryClipFunction = vtkSmartPointer<vtkSphere>::New();
        vtkSphere* sphere = static_cast<vtkSphere*>(geometryClipFunction.GetPointer());
        sphere->SetCenter(geometry->GetCenter()[0], geometry->GetCenter()[1], geometry->GetCenter()[2]);
        sphere->SetRadius(100);
        vtkSmartPointer<vtkSphereSource>  sphereSource = vtkSmartPointer<vtkSphereSource>::New();
        sphereSource->SetCenter(geometry->GetCenter()[0], geometry->GetCenter()[1], geometry->GetCenter()[2]);
        sphereSource->SetRadius(100);
        sphereSource->SetPhiResolution(50);
        sphereSource->SetThetaResolution(50);
        sphereSource->Update();
        outputPolyData->DeepCopy(sphereSource->GetOutput());
    }
    else if (shape.compare("Box", Qt::CaseSensitive) == 0)
    {
        geometryClipFunction = vtkSmartPointer<vtkBox>::New();
        vtkBox* box = static_cast<vtkBox*>(geometryClipFunction.GetPointer());
        box->SetBounds(geometry->GetBounds().GetDataPointer());

        vtkSmartPointer<vtkCubeSource>  boxSource = vtkSmartPointer<vtkCubeSource>::New();
        boxSource->SetBounds(geometry->GetBounds().GetDataPointer());
        boxSource->Update();
        outputPolyData->DeepCopy(boxSource->GetOutput());
    }
    return outputPolyData;
}



