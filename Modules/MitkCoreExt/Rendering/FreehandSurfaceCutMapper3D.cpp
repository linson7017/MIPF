#include "FreehandSurfaceCutMapper3D.h"
#include "vtkPolyData.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkPropAssembly.h"

mitk::FreehandSurfaceCutMapper3D::LocalStorage::LocalStorage()
{
    m_Actor = vtkSmartPointer<vtkActor>::New();
    m_Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_Assembly = vtkSmartPointer<vtkPropAssembly>::New();

    m_Actor->SetMapper(m_Mapper);

    m_Assembly->AddPart(m_Actor);
}


mitk::FreehandSurfaceCutMapper3D::FreehandSurfaceCutMapper3D()
{
}

mitk::FreehandSurfaceCutMapper3D::~FreehandSurfaceCutMapper3D()
{
}

const mitk::Surface *mitk::FreehandSurfaceCutMapper3D::GetInput() const
{
    return static_cast<Surface *>(GetDataNode()->GetData());
}

vtkProp* mitk::FreehandSurfaceCutMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
    LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
    return ls->m_Assembly;
}

void mitk::FreehandSurfaceCutMapper3D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
    LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
    ls->UpdateGenerateDataTime();

    ls->m_Mapper->SetInputData(GetInput()->GetVtkPolyData());

    float lineWidth = 1.0;
    GetDataNode()->GetFloatProperty("line width", lineWidth);
    ls->m_Actor->GetProperty()->SetLineWidth(lineWidth);

    float color[] = {0.0,1.0,0.0};
    GetDataNode()->GetColor(color);
    ls->m_Actor->GetProperty()->SetColor(color[0],color[1],color[2]);

}
