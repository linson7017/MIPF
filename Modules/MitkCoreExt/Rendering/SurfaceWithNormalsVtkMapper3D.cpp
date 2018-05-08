#include "SurfaceWithNormalsVtkMapper3D.h"


#include <vtkActor.h>
#include <vtkPolyDataMapper.h>

#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkArrowSource.h>
#include <vtkGlyph3D.h>
#include <vtkCellData.h>
#include <vtkAppendPolyData.h>
#include <vtkLineSource.h>
#include <vtkCell.h>


#include <mitkCameraController.h>

namespace mitk
{
    SurfaceWithNormalsVtkMapper3D::LocalStorage::LocalStorage()
    {
        m_Actor = vtkSmartPointer<vtkActor>::New();
        m_Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        m_Actor->SetMapper(m_Mapper);


        m_Assembly = vtkSmartPointer<vtkPropAssembly>::New();
        m_Assembly->AddPart(m_Actor);
    }


SurfaceWithNormalsVtkMapper3D::SurfaceWithNormalsVtkMapper3D()
{
}


SurfaceWithNormalsVtkMapper3D::~SurfaceWithNormalsVtkMapper3D()
{
}


const mitk::Surface *SurfaceWithNormalsVtkMapper3D::GetInput() const
{
    return static_cast<Surface *>(GetDataNode()->GetData());
}


vtkProp* SurfaceWithNormalsVtkMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
    LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
    return ls->m_Assembly;
}




void SurfaceWithNormalsVtkMapper3D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
   /* if (renderer->GetMapperID()!=mitk::BaseRenderer::Standard3D)
    {
        return;
    }*/
    bool visible = false;
    GetDataNode()->GetVisibility(visible, renderer);
    if (!visible)
    {
        return;
    }
    
    LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
    ls->UpdateGenerateDataTime();

    vtkPolyData* polyData = GetInput()->GetVtkPolyData();
    
    vtkFloatArray* normalDataFloat =
        vtkFloatArray::SafeDownCast(polyData->GetCellData()->GetArray("Normals"));

    if (normalDataFloat)
    {
        int nc = normalDataFloat->GetNumberOfTuples();
        
        auto appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
        for (int i=0;i<nc;i++)
        {
            mitk::Vector3D normal(normalDataFloat->GetTuple(i));
            normal.Normalize();
            double bounds[6];
            polyData->GetCellBounds(i, bounds);
            mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
            geometry->SetBounds(bounds);
            mitk::Point3D target = geometry->GetCenter() + normal * 1;
            
            vtkSmartPointer<vtkLineSource> lineSource =
                vtkSmartPointer<vtkLineSource>::New();
            lineSource->SetPoint1(geometry->GetCenter().GetDataPointer());
            lineSource->SetPoint2(target.GetDataPointer());
            lineSource->Update();

            appendFilter->AddInputData(lineSource->GetOutput());
        }
        appendFilter->Update();
        ls->m_Mapper->SetInputData(appendFilter->GetOutput());
    }



}


} //end namspace mitk
