#include "ModelExporterView.h" 
#include "iqf_main.h"  

#include "QmitkStdMultiWidget.h"
#include "mitkRenderingManager.h"
#include <QFileDialog>

#include <vtkOBJExporter.h>
#include <vtkX3DExporter.h>
#include <vtkRendererCollection.h>
#include <vtkRenderer.h>
#include <vtkPolyDataMapper.h>  
#include <vtkActor.h>  
#include <vtkRenderWindow.h>  
#include <vtkRenderer.h>  
#include <vtkRenderWindowInteractor.h>  

#include <vtkOBJImporter.h>

#include <VTKSceneViwer.h>
  
ModelExporterView::ModelExporterView() :MitkPluginView() 
{
}
 
ModelExporterView::~ModelExporterView() 
{
}
 
void ModelExporterView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Surface>::New());

    connect(m_ui.ApplyBtn, &QPushButton::clicked,this, &ModelExporterView::Apply);
    connect(m_ui.ImportBtn, &QPushButton::clicked, this, &ModelExporterView::Import);

    m_sceneViewer = new VTKSceneViwer(m_pMain,this);
} 
 
WndHandle ModelExporterView::GetPluginHandle() 
{
    return this; 
}

void ModelExporterView::Import()
{

}

void ModelExporterView ::Apply()
{
    //所有注册的窗口中render的actors数目都为零，奇怪！！
  /*  mitk::RenderingManager::RenderWindowVector renderwindows = m_pMitkRenderWindow->GetRenderingManager()->GetAllRegisteredRenderWindows();
    for (int j = 0; j < renderwindows.size(); j++)
    {
        vtkRenderWindow* renwin = renderwindows.at(j);
        vtkRendererCollection* renderers = renwin->GetRenderers();
        int numberOfRenderers = renderers->GetNumberOfItems();
        renderers->InitTraversal();
        int i = 0;
        while (i < numberOfRenderers)
        {
            vtkRenderer *r = renderers->GetNextItem();
            MITK_INFO << "Number Of actors in renderer " << i << " of renderwindow " << j << " : " << r->GetActors()->GetNumberOfItems();
            ++i;
        }
    }*/
    if (m_sceneViewer->isHidden())
    {
        m_sceneViewer->show();
    }

    if (m_ui.DataSelector->GetSelectedNode().IsNull())
    {
        return;
    }
    mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_ui.DataSelector->GetSelectedNode()->GetData());
    if (!surface)
    {
        return;
    }
    m_sceneViewer->AddPolyData(surface->GetVtkPolyData(), m_ui.DataSelector->GetSelectedNode()->GetName(),
        m_ui.DataSelector->GetSelectedNode(),
        m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow4()->GetRenderer());
    m_sceneViewer->show();

    return;

    
     

}

