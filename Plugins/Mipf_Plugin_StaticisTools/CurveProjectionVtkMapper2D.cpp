#include "CurveProjectionVtkMapper2D.h"

#include "mitkSurface.h"
#include "mitkDataNode.h"

#include "vtkPolyData.h"
#include "vtkActor2D.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkProperty2D.h"
#include "vtkLineSource.h"

using namespace mitk;


CurveProjectionVtkMapper2D::LocalStorage::LocalStorage()
{
    m_lineActor = vtkSmartPointer<vtkActor2D>::New();
    m_Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    m_Assembly = vtkSmartPointer<vtkPropAssembly>::New();

    m_lineActor->SetMapper(m_Mapper);

    m_lineActor->GetProperty()->SetLineStipplePattern(0xf0f0f0);
    m_lineActor->GetProperty()->SetColor(0, 0, 1.0);
    m_lineActor->GetProperty()->SetLineWidth(2.0);

    m_Assembly->AddPart(m_lineActor);


    vtkCoordinate *tcoord = vtkCoordinate::New();
    tcoord->SetCoordinateSystemToWorld();
    m_Mapper->SetTransformCoordinate(tcoord);
    tcoord->Delete();

}

CurveProjectionVtkMapper2D::CurveProjectionVtkMapper2D()
{
}


CurveProjectionVtkMapper2D::~CurveProjectionVtkMapper2D()
{
}

const mitk::Surface *CurveProjectionVtkMapper2D::GetInput() const
{
    return static_cast<Surface *>(GetDataNode()->GetData());
}


vtkProp* CurveProjectionVtkMapper2D::GetVtkProp(mitk::BaseRenderer *renderer)
{
    LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
    return ls->m_Assembly;
}

void CurveProjectionVtkMapper2D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
    //renderer->GetRenderWindow()->LineSmoothingOn();
    LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
    ls->UpdateGenerateDataTime();

    vtkPolyData* lineData = GetInput()->GetVtkPolyData();

    if (!lineData)
    {
        return;
    }

    if (lineData->GetPoints()->GetNumberOfPoints() != 2)
    {
        return;
    }

    double start[3];
    double end[3];
    lineData->GetPoint(0, start);
    lineData->GetPoint(1, end);

    mitk::Point3D projected_start(start);
    mitk::Point3D projected_end(end);

    const mitk::PlaneGeometry * planeGeometry = renderer->GetSliceNavigationController()->GetCurrentPlaneGeometry();
    planeGeometry->Project(projected_start, projected_start);
    planeGeometry->Project(projected_end, projected_end);

    vtkSmartPointer<vtkLineSource> lineSource =
        vtkSmartPointer<vtkLineSource>::New();
    lineSource->SetPoint1(projected_start.GetDataPointer());
    lineSource->SetPoint2(projected_end.GetDataPointer());
    lineSource->Update();

    ls->m_Mapper->SetInputData(lineSource->GetOutput());

}
