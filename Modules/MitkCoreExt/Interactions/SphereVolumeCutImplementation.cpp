#include "SphereVolumeCutImplementation.h"

#include <mitkImage.h>

#include <vtkPolyData.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencil.h>

SphereVolumeCutImplementation::SphereVolumeCutImplementation()
{
}

SphereVolumeCutImplementation::~SphereVolumeCutImplementation()
{
}

vtkSmartPointer<vtkDataObject> SphereVolumeCutImplementation::CutImpl(vtkObject * pCutData, mitk::InteractionEvent * interactionEvent)
{
	vtkPolyData* sphere = dynamic_cast<vtkPolyData*>(pCutData);
	if (!sphere)
	{
		return nullptr;
	}
	mitk::Image* data = dynamic_cast<mitk::Image*>(m_pDataNode->GetData());
	if (!data)
	{
		return nullptr;
	}
	vtkImageData* img = data->GetVtkImageData();
	mitk::Point3D imageOrigin;
	imageOrigin = data->GetGeometry()->GetOrigin();

	vtkSmartPointer<vtkPolyDataToImageStencil> pol2stenc =
		vtkSmartPointer<vtkPolyDataToImageStencil>::New();
	pol2stenc->SetInputData(sphere);
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
    CutFinishedEvent.Send(result);
	return result;
}
