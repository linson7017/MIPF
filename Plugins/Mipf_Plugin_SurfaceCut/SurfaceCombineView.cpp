#include "SurfaceCombineView.h"

//vtk
#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>



SurfaceCombineView::SurfaceCombineView():MitkPluginView()
{
}


SurfaceCombineView::~SurfaceCombineView()
{
}

void SurfaceCombineView::CreateView()
{
    m_ui.setupUi(this);  

    connect(m_ui.AddBtn, SIGNAL(clicked()),this,SLOT(Add()));
    connect(m_ui.RemoveBtn, SIGNAL(clicked()), this, SLOT(Remove()));
    connect(m_ui.CombineBtn, SIGNAL(clicked()), this, SLOT(Combine()));
}


void SurfaceCombineView::Add()
{
    std::vector<mitk::DataNode::Pointer> selectedNode = m_pMitkDataManager->GetSelectedNodes();
    for (auto node : selectedNode)
    {
        mitk::Surface* surface = dynamic_cast<mitk::Surface*>(node->GetData());
        if (surface)
        {
            if (m_surfaces.count(node->GetName())!=0)
            {
                continue;
            }
            m_ui.SurfaceList->addItem(node->GetName().c_str());
            m_surfaces[node->GetName()] = surface->GetVtkPolyData();
        }
    }
}

void SurfaceCombineView::Remove()
{
    QList<QListWidgetItem*> selectedItems = m_ui.SurfaceList->selectedItems();
    foreach(QListWidgetItem* item, selectedItems)
    {
        m_ui.SurfaceList->takeItem(m_ui.SurfaceList->row(item));
        m_surfaces.erase(item->text().toStdString());
    }
}

void SurfaceCombineView::Combine()
{
    vtkSmartPointer<vtkAppendPolyData> appendFilter =
        vtkSmartPointer<vtkAppendPolyData>::New();

    for (auto data : m_surfaces)
    {
        appendFilter->AddInputData(data.second);
    } 
    appendFilter->Update();

    // Remove any duplicate points.
    vtkSmartPointer<vtkCleanPolyData> cleanFilter =
        vtkSmartPointer<vtkCleanPolyData>::New();
    cleanFilter->SetInputConnection(appendFilter->GetOutputPort());
    cleanFilter->Update();

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    mitk::Surface::Pointer surface = mitk::Surface::New();
    surface->SetVtkPolyData(cleanFilter->GetOutput());
    node->SetData(surface);
    
    node->SetName("Result");
    GetDataStorage()->Add(node);

}
