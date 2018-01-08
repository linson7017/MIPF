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

vtkDataObject* FreehandSurfaceCutImplementation::GetDataObject()
{
    mitk::Surface* pData = dynamic_cast<mitk::Surface*>(m_pDataNode->GetData());
    if (pData)
    {
        return pData->GetVtkPolyData();
    }
    else
    {
        return nullptr;
    }
}

vtkSmartPointer<vtkDataObject> FreehandSurfaceCutImplementation::GetCopyOfDataObject()
{
    mitk::Surface* pData = dynamic_cast<mitk::Surface*>(m_pDataNode->GetData());
    if (pData)
    {
        auto copy = vtkSmartPointer<vtkPolyData>::New();
        copy->DeepCopy(pData->GetVtkPolyData());
        return copy;
    }
    else
    {
        return nullptr;
    }
}

void FreehandSurfaceCutImplementation::Refresh()
{
        mitk::Surface* pData = dynamic_cast<mitk::Surface*>(m_pDataNode->GetData());
        auto poly = dynamic_cast<vtkPolyData*>(TopOfUndo().Get());
        if (pData&&poly)
        {
            pData->GetVtkPolyData()->DeepCopy(poly);
            mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
}

void FreehandSurfaceCutImplementation::Cut(vtkObject* pCutData, mitk::InteractionEvent * interactionEvent)
{
    vtkPoints* curvePoints = dynamic_cast<vtkPoints*>(pCutData);
    mitk::InteractionPositionEvent *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
    if (!positionEvent||!curvePoints)
    {
        return;
    }
    auto clipFunction = vtkSmartPointer<vtkImplicitSelectionLoop>::New();
    clipFunction->SetLoop(curvePoints);

    double eyeNormal[] = { 0.0,0.0,1.0 };
    positionEvent->GetSender()->GetVtkRenderer()->GetActiveCamera()->GetDirectionOfProjection(eyeNormal);
    clipFunction->SetNormal(eyeNormal);
    // clipFunction->AutomaticNormalGenerationOn();

    mitk::Surface* pSurfaceData = dynamic_cast<mitk::Surface*>(m_pDataNode->GetData());
    vtkSmartPointer<vtkClipPolyData> clipper =
        vtkSmartPointer<vtkClipPolyData>::New();
    clipper->SetInputData(pSurfaceData->GetVtkPolyData());
    clipper->SetClipFunction(clipFunction);
    clipper->SetInsideOut(InsideOut);

    clipper->Update();
    pSurfaceData->GetVtkPolyData()->DeepCopy(clipper->GetOutput());

    ClearRedo();
    m_undoList.push(clipper->GetOutput());

}
