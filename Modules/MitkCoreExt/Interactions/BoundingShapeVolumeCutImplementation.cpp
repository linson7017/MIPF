#include "BoundingShapeVolumeCutImplementation.h"
//mitk
#include <mitkRenderingManager.h>
#include <mitkImage.h>
//vtk
#include <vtkImageData.h>
#include <vtkImageStencil.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkPolyData.h>
#include <vtkCubeSource.h>

BoundingShapeVolumeCutImplementation::BoundingShapeVolumeCutImplementation()
{
}

BoundingShapeVolumeCutImplementation::~BoundingShapeVolumeCutImplementation()
{
}

vtkSmartPointer<vtkDataObject> BoundingShapeVolumeCutImplementation::CutImpl(vtkObject* pCutData, mitk::InteractionEvent * interactionEvent)
{
	vtkPolyData* cube = dynamic_cast<vtkPolyData*>(pCutData);
	if (!cube)
	{
		return nullptr;
	}

	mitk::Image* data = dynamic_cast<mitk::Image*>(m_pDataNode->GetData());
	vtkImageData* img = data->GetVtkImageData();
	mitk::Point3D imageOrigin;
	imageOrigin = data->GetGeometry()->GetOrigin();

	vtkSmartPointer<vtkPolyDataToImageStencil> pol2stenc =
		vtkSmartPointer<vtkPolyDataToImageStencil>::New();
	pol2stenc->SetInputData(cube);
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
