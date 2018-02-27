#include "FreehandSurfaceCutImplementation.h"

#include "mitkInteractionPositionEvent.h"
#include "mitkSurface.h"

#include <vtkImplicitSelectionLoop.h>
#include <vtkClipPolyData.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

FreehandSurfaceCutImplementation::FreehandSurfaceCutImplementation()
{
}

FreehandSurfaceCutImplementation::~FreehandSurfaceCutImplementation()
{

}

vtkSmartPointer<vtkDataObject> FreehandSurfaceCutImplementation::CutImpl(vtkObject* pCutData, mitk::InteractionEvent * interactionEvent)
{
    vtkSmartPointer<vtkObject> ll = pCutData;
    vtkPoints* curvePoints = dynamic_cast<vtkPoints*>(pCutData);
    mitk::InteractionPositionEvent *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
    if (!positionEvent||!curvePoints)
    {
        return nullptr;
    }
    auto clipFunction = vtkSmartPointer<vtkImplicitSelectionLoop>::New();
    clipFunction->SetLoop(curvePoints);

    double eyeNormal[] = { 0.0,0.0,1.0 };
    positionEvent->GetSender()->GetVtkRenderer()->GetActiveCamera()->GetDirectionOfProjection(eyeNormal);
    clipFunction->SetNormal(eyeNormal);
    // clipFunction->AutomaticNormalGenerationOn();

    mitk::Surface* pSurfaceData = dynamic_cast<mitk::Surface*>(m_pDataNode->GetData());
    auto clipper =
        vtkSmartPointer<vtkClipPolyData>::New();
    clipper->SetInputData(pSurfaceData->GetVtkPolyData());
    clipper->SetClipFunction(clipFunction);
    clipper->SetInsideOut(InsideOut);
    clipper->Update();


    auto result = vtkSmartPointer<vtkPolyData>::New();
    result->DeepCopy(clipper->GetOutput());
    return result;

}
