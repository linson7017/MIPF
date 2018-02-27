#include "SphereSurfaceCutImplementation.h"

#include <mitkSurface.h>

#include <vtkPolyData.h>
#include <vtkSphere.h>
#include <vtkClipPolyData.h>

SphereSurfaceCutImplementation::SphereSurfaceCutImplementation()
{
}

SphereSurfaceCutImplementation::~SphereSurfaceCutImplementation()
{
}

vtkSmartPointer<vtkDataObject> SphereSurfaceCutImplementation::CutImpl(vtkObject * pCutData, mitk::InteractionEvent * interactionEvent)
{
	vtkPolyData* sphere = dynamic_cast<vtkPolyData*>(pCutData);
	if (!sphere)
	{
		return nullptr;
	}
	mitk::Surface* pSurfaceData = dynamic_cast<mitk::Surface*>(m_pDataNode->GetData());
	vtkSmartPointer<vtkPolyData> surfacePolyData = pSurfaceData->GetVtkPolyData();

    vtkSmartPointer<vtkSphere> clipFunction = vtkSmartPointer<vtkSphere>::New();
    clipFunction->SetCenter(sphere->GetCenter());
    double r = (sphere->GetBounds()[1] - sphere->GetBounds()[0]) / 2;
    clipFunction->SetRadius(r);

    vtkSmartPointer<vtkClipPolyData> clipper =
        vtkSmartPointer<vtkClipPolyData>::New();
    clipper->SetInputData(surfacePolyData);
    clipper->SetClipFunction(clipFunction);
    clipper->SetInsideOut(InsideOut);
    clipper->Update();

	auto result = vtkSmartPointer<vtkPolyData>::New();
	result->DeepCopy(clipper->GetOutput());
	return result;
}
