#include "ColoredSurfaceVtkMapper.h"

#include "vtkPolyData.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkPropAssembly.h"
#include "vtkPointData.h"


#include "mitkVtkRepresentationProperty.h"
#include "mitkLookupTableProperty.h"



//mitk::ColoredSurfaceVtkMapper::LocalStorage::LocalStorage()
//{
//    m_Actor = vtkSmartPointer<vtkActor>::New();
//    m_Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//    m_Assembly = vtkSmartPointer<vtkPropAssembly>::New();
//
//    m_Actor->SetMapper(m_Mapper);
//
//    m_Assembly->AddPart(m_Actor);
//}

mitk::ColoredSurfaceVtkMapper::ColoredSurfaceVtkMapper()
{
}


mitk::ColoredSurfaceVtkMapper::~ColoredSurfaceVtkMapper()
{
}

//const mitk::Surface *mitk::ColoredSurfaceVtkMapper::GetInput() const
//{
//    return static_cast<Surface *>(GetDataNode()->GetData());
//}
//
//vtkProp* mitk::ColoredSurfaceVtkMapper::GetVtkProp(mitk::BaseRenderer *renderer)
//{
//    LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
//    return ls->m_Assembly;
//}

//void mitk::ColoredSurfaceVtkMapper::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
//{
//    LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
//    ls->UpdateGenerateDataTime();
//
//    ls->m_Mapper->SetInputData(GetInput()->GetVtkPolyData());
//
//    ApplyProperties(renderer);
//}

//void mitk::ColoredSurfaceVtkMapper::ApplyProperties(mitk::BaseRenderer *renderer)
//{
//    LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
//    vtkDataArray* array = GetInput()->GetVtkPolyData()->GetPointData()->GetArray("RegionId");
//    if (array)
//    {
//        ls->m_Mapper->SetScalarRange(array->GetRange());
//    }
//    mitk::LookupTableProperty* lutp = dynamic_cast<mitk::LookupTableProperty*>(GetDataNode()->GetProperty("lookup table"));
//
//    if (lutp)
//    {
//        ls->m_Mapper->SetLookupTable(lutp->GetLookupTable()->GetVtkLookupTable());
//    }
//
//
//    float opacity = 1.0;
//    GetDataNode()->GetOpacity(opacity, renderer);
//    ls->m_Actor->GetProperty()->SetOpacity(opacity);
//
//    mitk::VtkRepresentationProperty::Pointer representation = mitk::VtkRepresentationProperty::New();
//    GetDataNode()->GetProperty(representation, "material.representation");
//    ls->m_Actor->GetProperty()->SetRepresentation(representation->GetVtkRepresentation());
//}


void mitk::ColoredSurfaceVtkMapper::ApplyAllProperties(mitk::BaseRenderer *renderer, vtkActor *actor)
{
    LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
    mitk::SurfaceVtkMapper3D::ApplyAllProperties(renderer, actor);

   std::string colorArray;
    if (this->GetDataNode()->GetStringProperty("ColorArray", colorArray, renderer))
    {
        ls->m_VtkPolyDataMapper->SelectColorArray(colorArray.c_str());
    }

}