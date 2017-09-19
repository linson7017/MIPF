#include "GeometryInteractor.h"

#include "mitkInteractionPositionEvent.h"
#include "mitkMouseWheelEvent.h"

GeometryInteractor::GeometryInteractor()
{
}


GeometryInteractor::~GeometryInteractor()
{
}

void GeometryInteractor::ScaleObject(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
    const mitk::MouseWheelEvent *wheelEvent = dynamic_cast<const mitk::MouseWheelEvent *>(interactionEvent);
    int delta = wheelEvent->GetWheelDelta();
    mitk::Point3D newScale;
    newScale.Fill(delta>0?0.1:-0.1);

    mitk::Point3D anchor = GetDataNode()->GetData()->GetUpdatedGeometry()->GetCenter();
    GetDataNode()->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point X", anchor[0]);
    GetDataNode()->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point Y", anchor[1]);
    GetDataNode()->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point Z", anchor[2]);

    this->ScaleGeometry(newScale, this->GetUpdatedTimeGeometry(interactionEvent));
}

void GeometryInteractor::RotateObject(mitk::StateMachineAction * sma, mitk::InteractionEvent * ie)
{
    mitk::Point3D anchor = GetDataNode()->GetData()->GetUpdatedGeometry()->GetCenter();
    GetDataNode()->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point X", anchor[0]);
    GetDataNode()->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point Y", anchor[1]);
    GetDataNode()->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point Z", anchor[2]);

    AffineBaseDataInteractor3D::RotateObject(sma, ie);
}
