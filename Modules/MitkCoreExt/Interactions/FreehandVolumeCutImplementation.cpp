#include "FreehandVolumeCutImplementation.h"

#include "mitkInteractionPositionEvent.h"
#include "mitkImage.h"

#include <vtkLinearExtrusionFilter.h>
#include <vtkPolygon.h>
#include <vtkImageData.h>
#include <vtkImageStencil.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

FreehandVolumeCutImplementation::FreehandVolumeCutImplementation()
{
}

FreehandVolumeCutImplementation::~FreehandVolumeCutImplementation()
{
}

vtkSmartPointer<vtkDataObject> FreehandVolumeCutImplementation::CutImpl(vtkObject* pCutData, mitk::InteractionEvent * interactionEvent)
{
    vtkPoints* curvePoints = dynamic_cast<vtkPoints*>(pCutData);
    mitk::InteractionPositionEvent *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
    if (!positionEvent|| !curvePoints)
    {
        return nullptr;
    }
    mitk::Image* data = dynamic_cast<mitk::Image*>(m_pDataNode->GetData());

    vtkSmartPointer<vtkPolygon> polygon =
        vtkSmartPointer<vtkPolygon>::New();
    polygon->GetPointIds()->SetNumberOfIds(curvePoints->GetNumberOfPoints());
    for (int i = 0; i < curvePoints->GetNumberOfPoints(); i++)
    {
        polygon->GetPointIds()->SetId(i, i);
    }
    auto polygons = vtkSmartPointer<vtkCellArray>::New();
    polygons->InsertNextCell(polygon);
    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(curvePoints);
    polyData->SetPolys(polygons);

    
    vtkImageData* img = data->GetVtkImageData();
    double normal[3];
    positionEvent->GetSender()->GetVtkRenderer()->GetActiveCamera()->GetDirectionOfProjection(normal);

    vtkSmartPointer<vtkLinearExtrusionFilter> extrude =
        vtkSmartPointer<vtkLinearExtrusionFilter>::New();
    extrude->SetInputData(polyData);
    extrude->SetExtrusionTypeToVectorExtrusion();
    extrude->SetCapping(1);
    extrude->SetVector(normal);
    extrude->SetScaleFactor(800);

    //origin in vtkimagedata is often different from the origin in geometry
    mitk::Point3D imageOrigin;
    imageOrigin = data->GetGeometry()->GetOrigin();

    vtkSmartPointer<vtkPolyDataToImageStencil> pol2stenc =
        vtkSmartPointer<vtkPolyDataToImageStencil>::New();
    pol2stenc->SetInputConnection(extrude->GetOutputPort());
    pol2stenc->SetTolerance(0.0);
    pol2stenc->ReleaseDataFlagOn();
    pol2stenc->SetOutputOrigin(imageOrigin.GetDataPointer());
    pol2stenc->SetOutputSpacing(img->GetSpacing());
    pol2stenc->SetOutputWholeExtent(img->GetExtent());

    vtkSmartPointer<vtkImageStencil> imgstenc =
        vtkSmartPointer<vtkImageStencil>::New();
    imgstenc->SetInputData(img);
    imgstenc->SetStencilConnection(pol2stenc->GetOutputPort());
    imgstenc->SetReverseStencil(!InsideOut);
    imgstenc->SetBackgroundValue(data->GetScalarValueMin());
    imgstenc->Update();

    auto result = vtkSmartPointer<vtkImageData>::New();
    result->DeepCopy(imgstenc->GetOutput());
    return result;    
}
