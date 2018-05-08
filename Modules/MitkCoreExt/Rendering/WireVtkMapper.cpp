#include "WireVtkMapper.h"

#include "mitkSurface.h"
#include "mitkDataNode.h"
#include "mitkLine.h"

#include "vtkPolyData.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkLineSource.h"
#include "vtkLine.h"
#include "vtkRegularPolygonSource.h"
#include "vtkMath.h"
#include "vtkPolyLine.h"

namespace mitk
{ 

WireVtkMapper::LocalStorage::LocalStorage()
{
    m_lineActor = vtkSmartPointer<vtkActor>::New();


    m_lineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();


    m_Assembly = vtkSmartPointer<vtkPropAssembly>::New();

    m_lineActor->SetMapper(m_lineMapper);

    m_Assembly->AddPart(m_lineActor);

}

WireVtkMapper::WireVtkMapper()
{
    int x = 0;
}


WireVtkMapper::~WireVtkMapper()
{
}

const mitk::Wire *WireVtkMapper::GetInput() const
{
    return static_cast<Wire *>(GetDataNode()->GetData());
}


vtkProp* WireVtkMapper::GetVtkProp(mitk::BaseRenderer *renderer)
{
    LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
    return ls->m_Assembly;
}


void WireVtkMapper::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
    renderer->GetRenderWindow()->LineSmoothingOn();
    LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
    ls->UpdateGenerateDataTime();

    Wire* wire = static_cast<Wire *>(GetDataNode()->GetData());
    if (!wire)
    {
        return;
    }
    Mass** masses = wire->Simulation()->masses;

    auto polydata = vtkSmartPointer<vtkPolyData>::New();
    auto points = vtkSmartPointer<vtkPoints>::New();
    auto polyLine = vtkSmartPointer<vtkPolyLine>::New();
    polyLine->GetPointIds()->SetNumberOfIds(wire->Simulation()->numOfMasses);
    auto cells = vtkSmartPointer<vtkCellArray>::New();
    for (int i = 0; i < wire->Simulation()->numOfMasses; i++)
    {
        Mass* mass = masses[i];
        points->InsertNextPoint(mass->pos.x, mass->pos.y, mass->pos.z);
        polyLine->GetPointIds()->SetId(i, i);
    }
    cells->InsertNextCell(polyLine);
    polydata->SetPoints(points);
    polydata->SetLines(cells);
    ls->m_lineMapper->SetInputData(polydata);
}

} //end namspace mitk