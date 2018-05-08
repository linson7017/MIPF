#include "SurfaceIntersectView.h"

#include <vtkPolyData.h>
#include <vtkIntersectionPolyDataFilter.h>

SurfaceIntersectView::SurfaceIntersectView()
{
}


SurfaceIntersectView::~SurfaceIntersectView()
{
}

void SurfaceIntersectView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.DataSelector1->SetDataStorage(GetDataStorage());
    m_ui.DataSelector1->SetPredicate(CreateSurfacePredicate());
    m_ui.DataSelector2->SetDataStorage(GetDataStorage());
    m_ui.DataSelector2->SetPredicate(CreateSurfacePredicate());

    connect(m_ui.ApplyBtn, &QPushButton::clicked, this, &SurfaceIntersectView::Apply);
}

WndHandle SurfaceIntersectView::GetPluginHandle()
{
    return this;
}

void SurfaceIntersectView::Apply()
{
    mitk::Surface* surface1 = dynamic_cast<mitk::Surface*>(m_ui.DataSelector1->GetSelectedNode()->GetData());
    mitk::Surface* surface2 = dynamic_cast<mitk::Surface*>(m_ui.DataSelector2->GetSelectedNode()->GetData());


    if (!surface1||!surface2)
    {
        return;
    }

    vtkSmartPointer<vtkIntersectionPolyDataFilter> intersectionPolyDataFilter =
        vtkSmartPointer<vtkIntersectionPolyDataFilter>::New();
    intersectionPolyDataFilter->SetInputData(0, surface1->GetVtkPolyData());
    intersectionPolyDataFilter->SetInputData(1, surface2->GetVtkPolyData());
    intersectionPolyDataFilter->Update();


    ImportVtkPolyData(intersectionPolyDataFilter->GetOutput(), "intersect")->SetColor(1.0,1.0,0.0);
}
