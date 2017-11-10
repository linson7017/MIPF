#include "MapperTestView.h"
#include "iqf_main.h"
#include "Res/R.h"

#include "mitkNodePredicateDataType.h"



//mitk
#include "mitkImage.h"
#include "mitkRenderWindow.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkRenderWindow.h"
#include "mitkBasePropertySerializer.h"
#include "tinyxml.h"



//vtk
#include "vtkJPEGReader.h"
#include "vtkTexture.h"
#include "vtkImageNoiseSource.h"
#include "vtkCamera.h"
#include "vtkRenderer.h"

#include "Rendering/ObjectFactoryExt.h"
#include "Rendering/TexturedVtkMapper3D.h"

#include "Interactions/FreehandSurfaceCutInteractor.h"

#include "MitkMain/IQF_MitkRenderWindow.h"

//itk
#include "itkCreateObjectFunction.h"

namespace mitk
{
    class NewObject : public itk::Object
    {
    public:
        typedef NewObject                Self;
        typedef itk::Object        Superclass;
        typedef itk::SmartPointer<Self>       Pointer;
        typedef itk::SmartPointer<const Self> ConstPointer;

        itkNewMacro(Self);

        //virtual const char *GetNameOfClass() const;  
        itkTypeMacro(Self, Superclass);

        void Test() { MITK_INFO << "Test"; }

    };
}


MapperTestView::MapperTestView() :MitkPluginView(), m_curveDrawInteractor(nullptr)
{    
}

void MapperTestView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "") == 0)
    {
        //do what you want for the message
    }
}

void MapperTestView::CreateView()
{
    RegisterObjectFactoryExt();
    m_pMain->Attach(this);
    m_ui.setupUi(this);

    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object",mitk::BoolProperty::New(true))));

    connect(m_ui.ApplyBtn, SIGNAL(clicked()), this, SLOT(Apply()));
    connect(m_ui.CutBtn, SIGNAL(clicked(bool)), this, SLOT(Cut(bool)));
    connect(m_ui.UndoBtn, SIGNAL(clicked()), this, SLOT(Undo()));
    connect(m_ui.RedoBtn, SIGNAL(clicked()), this, SLOT(Redo()));
    connect(m_ui.InsideOutCheckBox, SIGNAL(clicked(bool)), this, SLOT(InsideOut(bool)));

}

void MapperTestView::InsideOut(bool flag)
{
      if (m_curveDrawInteractor)
      {
          static_cast<FreehandSurfaceCutInteractor*>(m_curveDrawInteractor.GetPointer())->SetInsideOut(flag);
      }
}


void MapperTestView::Cut(bool enableCut)
{
   /* itk::LightObject::Pointer la =  itk::CreateObjectFunction<mitk::NewObject>::New();
    mitk::NewObject *obj = dynamic_cast<mitk::NewObject *>(la.GetPointer());
    obj->Test();*/

    //std::list<itk::LightObject::Pointer> objects = itk::ObjectFactoryBase::CreateAllInstance("NewObject");
    //for (auto iter = objects.begin(); iter != objects.end(); ++iter)
    //{
    //    if (mitk::NewObject *obj = dynamic_cast<mitk::NewObject *>(iter->GetPointer()))
    //    {
    //        obj->Test();
    //    }
    //}
    //return;


    mitk::DataNode* node = m_ui.DataSelector->GetSelectedNode();

    MITK_INFO << node->GetData();

    mitk::BaseProperty* property = node->GetProperty("LookupTable");

    std::string serializername = property->GetNameOfClass();
    serializername.append("Serializer");
    std::list<itk::LightObject::Pointer> allSerializers =
        itk::ObjectFactoryBase::CreateAllInstance(serializername.c_str());

    for (auto iter = allSerializers.begin(); iter != allSerializers.end(); ++iter)
    {
        if (mitk::BasePropertySerializer *serializer = dynamic_cast<mitk::BasePropertySerializer *>(iter->GetPointer()))
        {
            serializer->SetProperty(property);
            try
            {
                TiXmlElement *valueelement = serializer->Serialize();
               // std::string str1 = valueelement->GetText();
                std::string str2 = valueelement->FirstChildElement("Table")->FirstChildElement()->FirstAttribute()->Value();
                int x = 0;
            }
            catch (std::exception &e)
            {
                MITK_ERROR << "Serializer " << serializer->GetNameOfClass() << " failed: " << e.what();
                // \TODO: log only if all potential serializers fail?
            }
        }
    }

    if (!node)
    {
        return;
    }
    if (enableCut)
    {
        if (m_curveDrawInteractor.IsNull())
        {
            m_curveDrawInteractor = FreehandSurfaceCutInteractor::New();
            static_cast<FreehandSurfaceCutInteractor*>(m_curveDrawInteractor.GetPointer())->SetDataStorage(GetDataStorage());
            static_cast<FreehandSurfaceCutInteractor*>(m_curveDrawInteractor.GetPointer())->SetInsideOut(m_ui.InsideOutCheckBox->isChecked());
            static_cast<FreehandSurfaceCutInteractor*>(m_curveDrawInteractor.GetPointer())->SetRenderer(
                m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow4()->GetRenderer()->GetVtkRenderer());

            std::string configpath = m_pMain->GetConfigPath();
            configpath.append("/mitk/Interactions/");

            m_curveDrawInteractor->LoadStateMachine(configpath + "FreehandSurfaceCutInteraction.xml");
            m_curveDrawInteractor->SetEventConfig(configpath + "FreehandSurfaceCutConfig.xml");
            m_curveDrawInteractor->SetDataNode(node); 

        }
        node->SetDataInteractor(m_curveDrawInteractor);
        static_cast<FreehandSurfaceCutInteractor*>(m_curveDrawInteractor.GetPointer())->Start();
        RequestRenderWindowUpdate();
    }
    else
    {
        static_cast<FreehandSurfaceCutInteractor*>(m_curveDrawInteractor.GetPointer())->Finished();
        node->SetDataInteractor(nullptr);
        static_cast<FreehandSurfaceCutInteractor*>(m_curveDrawInteractor.GetPointer())->End();
        RequestRenderWindowUpdate();

    }
}

#include "vtkMapper.h"
#include "vtkPlanes.h"
#include "vtkplane.h"
#include "vtkVolume.h"
#include "vtkAbstractVolumeMapper.h"
#include "vtkBoxWidget.h"

void MapperTestView::Apply()
{
    mitk::DataNode* node = m_ui.DataSelector->GetSelectedNode();
    mitk::VtkMapper* mapper = dynamic_cast<mitk::VtkMapper*>(node->GetMapper(mitk::BaseRenderer::Standard3D));
    vtkProp* prop = mapper->GetVtkProp(m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow4()->GetRenderer());
    vtkVolume* volume = dynamic_cast<vtkVolume*>(prop);
    if (volume)
    {
        vtkSmartPointer<vtkBoxWidget> boxWidget =
            vtkSmartPointer<vtkBoxWidget>::New();
        boxWidget->SetInteractor(m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow4()->GetVtkRenderWindowInteractor());

        boxWidget->SetProp3D(volume);
        boxWidget->SetPlaceFactor(0.8); // Make the box 1.25x larger than the actor
        boxWidget->PlaceWidget();
        boxWidget->On();

        const mitk::PlaneGeometry* mitkPlane = m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow3()->GetRenderer()->GetCurrentWorldPlaneGeometry();
        vtkPlanes* planes = vtkPlanes::New();
        vtkPlane* plane = vtkPlane::New();
        const double* normal = mitkPlane->GetNormal().GetDataPointer();
        plane->SetNormal(-normal[0],-normal[1],-normal[2]);
        const double* origin = mitkPlane->GetOrigin().GetDataPointer();
        plane->SetOrigin(origin[0], origin[1], origin[2]);
        boxWidget->GetPlanes(planes);
       // volume->GetMapper()->SetClippingPlanes(planes);
        volume->GetMapper()->AddClippingPlane(plane);
    }



    /*   vtkPropCollection *props = m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow4()->GetRenderer()->GetVtkRenderer()->GetViewProps();
       int numberOfProps = props->GetNumberOfItems();
       auto propVisibilities = new bool[numberOfProps];
       props->InitTraversal();
       int i = 0;
       while (i < numberOfProps)
       {
           vtkProp *p = props->GetNextProp();
           p->SetVisibility(false);
           vtkVolume* volume = dynamic_cast<vtkVolume*>(p);
           if (volume)
           {
               const mitk::PlaneGeometry* mitkPlane = m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow3()->GetRenderer()->GetCurrentWorldPlaneGeometry();
               vtkPlanes* planes = vtkPlanes::New();
               vtkPlane* plane = vtkPlane::New();
               plane->SetNormal(mitkPlane->GetNormal().GetDataPointer());
               const double* origin = mitkPlane->GetOrigin().GetDataPointer();
               plane->SetOrigin(origin[0],origin[1],origin[2]);

               volume->GetMapper()->AddClippingPlane(plane);
           }
           ++i;
       }*/

    /*auto noiseSource = vtkSmartPointer<vtkImageNoiseSource>::New();
    noiseSource->SetMinimum(0.0);
    noiseSource->SetMaximum(1.0);
    noiseSource->SetWholeExtent(0, 128, 0, 128, 0, 128);
    noiseSource->Update();

    mitk::DataNode* node = m_ui.DataSelector->GetSelectedNode();
    if (!node)
    {
        return;
    }

    auto reader = vtkSmartPointer<vtkJPEGReader>::New();
    reader->SetFileName("D:/texture.jpg");
    reader->Update();

    auto texture = vtkSmartPointer<vtkTexture>::New();
    texture->SetInputData(reader->GetOutput());
    texture->Update();


    mitk::TexturedVtkMapper3D::Pointer texturedMapper = mitk::TexturedVtkMapper3D::New();
    texturedMapper->SetTexture(texture);
    texturedMapper->SetShaderSource("S:/Vertex.program","S:/Fragment.program");
    node->SetMapper(mitk::BaseRenderer::Standard3D, texturedMapper);
    node->SetStringProperty("3d mapper type","textured");*/
}

void MapperTestView::Undo()
{
    static_cast<FreehandSurfaceCutInteractor*>(m_curveDrawInteractor.GetPointer())->Undo();
}

void MapperTestView::Redo()
{
    static_cast<FreehandSurfaceCutInteractor*>(m_curveDrawInteractor.GetPointer())->Redo();

}

#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector3D>

QMatrix4x4  CaculateTransformMatrix(const QMatrix4x4& vm,const QMatrix4x4& m,const QVector3D& positionInImage)
{
    QMatrix4x4 am = vm.inverted();
    QVector3D t = positionInImage.toVector4D -am*m.column(3).toVector3D();
    am.setColumn(3, QVector4D(t, 1));
    return am;
}