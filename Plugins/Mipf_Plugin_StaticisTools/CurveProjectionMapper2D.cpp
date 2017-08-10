#include "CurveProjectionMapper2D.h"

#include "mitkImage.h"
#include "mitkDataNode.h"

#include "vtkPolyData.h"

using namespace mitk;


CurveProjectionMapper2D::CurveProjectionMapper2D()
{
    int x = 0;
}


CurveProjectionMapper2D::~CurveProjectionMapper2D()
{
}

void CurveProjectionMapper2D::Paint(BaseRenderer *renderer)
{
    //bool visible = true;
    //GetDataNode()->GetVisibility(visible, renderer, "visible");

    //if (!visible)
   //     return;
    

    mitk::Image* image = dynamic_cast<mitk::Image*>(GetDataNode()->GetData());

    if (!image)
    {
        return;
    }

    mitk::Vector3D spacing = image->GetGeometry()->GetSpacing();
    const mitk::BaseGeometry::BoundingBoxType* box = image->GetGeometry()->GetBoundingBox();
    int la = 0;


}

void CurveProjectionMapper2D::InitGLSL()
{
    
}
