#include "MapperTestView.h"
#include "iqf_main.h"
#include "Res/R.h"

#include "mitkNodePredicateDataType.h"



//mitk
#include "mitkImage.h"
#include "mitkRenderWindow.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkRenderWindow.h"
#include "vtkCamera.h"
#include "vtkRenderer.h"

//vtk
#include "vtkJPEGReader.h"
#include "vtkTexture.h"
#include "vtkImageNoiseSource.h"

#include "Rendering/ObjectFactoryExt.h"
#include "Rendering/TexturedVtkMapper3D.h"

#include "Interactions/FreehandSurfaceCutInteractor.h"

#include "MitkMain/IQF_MitkRenderWindow.h"

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
    m_ui.DataSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Surface>::New());

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
    mitk::DataNode* node = m_ui.DataSelector->GetSelectedNode();
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


void MapperTestView::Apply()
{
    auto noiseSource = vtkSmartPointer<vtkImageNoiseSource>::New();
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
    node->SetStringProperty("3d mapper type","textured");
}

void MapperTestView::Undo()
{
    static_cast<FreehandSurfaceCutInteractor*>(m_curveDrawInteractor.GetPointer())->Undo();
}

void MapperTestView::Redo()
{
    static_cast<FreehandSurfaceCutInteractor*>(m_curveDrawInteractor.GetPointer())->Redo();

}