#include "OrientationMarkerVtkMapper3D.h"


#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkGlyph3D.h>
#include <vtkDistanceToCamera.h>
#include <vtkPointData.h>

#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkRendererCollection.h>

#include <mitkCameraController.h>

namespace mitk
{
    OrientationMarkerVtkMapper3D::LocalStorage::LocalStorage()
    {
        m_orientationActor = vtkSmartPointer<vtkActor>::New();
        m_orientationMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        m_orientationActor->SetMapper(m_orientationMapper);


        

    }


OrientationMarkerVtkMapper3D::OrientationMarkerVtkMapper3D()
{
}


OrientationMarkerVtkMapper3D::~OrientationMarkerVtkMapper3D()
{
}


const mitk::Surface *OrientationMarkerVtkMapper3D::GetInput() const
{
    return static_cast<Surface *>(GetDataNode()->GetData());
}


vtkProp* OrientationMarkerVtkMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
    LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
    return ls->m_orientationActor;
}

double OrientationMarkerVtkMapper3D::ComputeScale(const double position[3], vtkRenderer *renderer)
{
    // Find the cursor scale factor such that 1 data unit length
    // equals 1 screen pixel at the cursor's distance from the camera.
    // Start by computing the height of the window at the cursor position.
    double worldHeight = 1.0;
    vtkCamera *camera = renderer->GetActiveCamera();
    if (camera->GetParallelProjection())
    {
        worldHeight = 2 * camera->GetParallelScale();
    }
    else
    {
        vtkMatrix4x4 *matrix = camera->GetViewTransformMatrix();
        // Get a 3x3 matrix with the camera orientation
        double cvz[3];
        cvz[0] = matrix->GetElement(2, 0);
        cvz[1] = matrix->GetElement(2, 1);
        cvz[2] = matrix->GetElement(2, 2);

        double cameraPosition[3];
        camera->GetPosition(cameraPosition);

        double v[3];
        v[0] = cameraPosition[0] - position[0];
        v[1] = cameraPosition[1] - position[1];
        v[2] = cameraPosition[2] - position[2];

        worldHeight = 2 * (vtkMath::Dot(v, cvz)
            * tan(0.5*camera->GetViewAngle()/57.296));
    }

    // Compare world height to window height.
    int windowHeight = renderer->GetSize()[1];
    double scale = 1.0;
    if (windowHeight > 0)
    {
        scale = worldHeight / windowHeight;
    }

    return scale;
}

void OrientationMarkerVtkMapper3D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
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
    if (polyData->GetPointData() && polyData->GetPointData()->HasArray("Color"))
    {
        polyData->GetPointData()->SetActiveScalars("Color");
        ls->m_orientationMapper->SetColorModeToDirectScalars();
    }

    ls->m_orientationMapper->SetInputData(polyData);

    //ApplyProperties(renderer);

    mitk::Point2D bottomLeftScreen;
    bottomLeftScreen.SetElement(0, 30);
    bottomLeftScreen.SetElement(1, 30);
    mitk::Point3D bottomLeftWorld;
    renderer->DisplayToWorld(bottomLeftScreen, bottomLeftWorld);

    ls->m_orientationActor->SetPosition(bottomLeftWorld.GetDataPointer());
    ls->m_orientationActor->SetScale(ComputeScale(bottomLeftWorld.GetDataPointer(), renderer->GetVtkRenderer())*0.5);
}


} //end namspace mitk
