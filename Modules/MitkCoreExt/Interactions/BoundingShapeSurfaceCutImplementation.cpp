#include "BoundingShapeSurfaceCutImplementation.h"

#include <mitkSurface.h>
#include <mitkRenderingManager.h>

#include <vtkImplicitPolyDataDistance.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkClipPolyData.h>
#include <vtkBox.h>

BoundingShapeSurfaceCutImplementation::BoundingShapeSurfaceCutImplementation()
{
}

BoundingShapeSurfaceCutImplementation::~BoundingShapeSurfaceCutImplementation()
{
}

vtkSmartPointer<vtkDataObject> BoundingShapeSurfaceCutImplementation::CutImpl(vtkObject* pCutData, mitk::InteractionEvent * interactionEvent)
{
	vtkPolyData* cube = dynamic_cast<vtkPolyData*>(pCutData);
	if (!cube)
	{
		return nullptr;
	}

//  //Any vtkPolyData
// 	vtkSmartPointer<vtkImplicitPolyDataDistance> implicitPolyDataDistance =
// 		vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
// 	implicitPolyDataDistance->SetInput(cube);
// 
// 	vtkSmartPointer<vtkFloatArray> signedDistances =
// 		vtkSmartPointer<vtkFloatArray>::New();
// 	signedDistances->SetNumberOfComponents(1);
// 	signedDistances->SetName("SignedDistances");
// 
// 	mitk::Surface* pSurfaceData = dynamic_cast<mitk::Surface*>(m_pDataNode->GetData());
// 	vtkSmartPointer<vtkPolyData> surfacePolyData = pSurfaceData->GetVtkPolyData();
// 
// 	for (vtkIdType pointId = 0; pointId < surfacePolyData->GetNumberOfPoints(); ++pointId)
// 	{
// 		double p[3];
// 		surfacePolyData->GetPoint(pointId, p);
// 		double signedDistance = implicitPolyDataDistance->EvaluateFunction(p);
// 		signedDistances->InsertNextValue(signedDistance);
// 	}
// 	surfacePolyData->GetPointData()->SetScalars(signedDistances);
// 
// 	vtkSmartPointer<vtkClipPolyData> clipper =
// 		vtkSmartPointer<vtkClipPolyData>::New();
// 	clipper->SetInputData(surfacePolyData);
// 	clipper->SetInsideOut(InsideOut);
// 	clipper->SetValue(0.0);
// 	clipper->GenerateClippedOutputOn();
// 	clipper->Update();

 //vtkBox
	mitk::Surface* pSurfaceData = dynamic_cast<mitk::Surface*>(m_pDataNode->GetData());
	vtkSmartPointer<vtkPolyData> surfacePolyData = pSurfaceData->GetVtkPolyData();

	vtkSmartPointer<vtkBox> clipFunction = vtkSmartPointer<vtkBox>::New();
	clipFunction->SetBounds(cube->GetBounds());

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