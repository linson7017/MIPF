#include "NeedleVtkMapper2D.h"

#include "mitkSurface.h"
#include "mitkDataNode.h"
#include "mitkLine.h"

#include "vtkPolyData.h"
#include "vtkActor2D.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkProperty2D.h"
#include "vtkLineSource.h"
#include "vtkLine.h"
#include "vtkCursor2D.h"
#include "vtkRegularPolygonSource.h"
#include "vtkMath.h"

namespace mitk
{ 

NeedleVtkMapper2D::LocalStorage::LocalStorage()
{
    m_lineActor = vtkSmartPointer<vtkActor2D>::New();
    m_lineBackActor = vtkSmartPointer<vtkActor2D>::New();
    m_crossHairActor = vtkSmartPointer<vtkActor2D>::New();
    m_intersectActor = vtkSmartPointer<vtkActor2D>::New();
    m_extensionActor = vtkSmartPointer<vtkActor2D>::New();

    m_lineMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    m_lineBackMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    m_crosshairMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    m_intersectMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    m_extensionMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();

    m_Assembly = vtkSmartPointer<vtkPropAssembly>::New();

    m_lineActor->SetMapper(m_lineMapper);
    m_lineBackActor->SetMapper(m_lineBackMapper);
    m_crossHairActor->SetMapper(m_crosshairMapper);
    m_intersectActor->SetMapper(m_intersectMapper);
    m_extensionActor->SetMapper(m_extensionMapper);

    m_lineBackActor->GetProperty()->SetLineStipplePattern(0x0f0f);


    m_Assembly->AddPart(m_lineActor);
    m_Assembly->AddPart(m_lineBackActor);
    m_Assembly->AddPart(m_crossHairActor);
    m_Assembly->AddPart(m_intersectActor);
    m_Assembly->AddPart(m_extensionActor);

    vtkCoordinate *tcoord = vtkCoordinate::New();
    tcoord->SetCoordinateSystemToWorld();
    m_extensionMapper->SetTransformCoordinate(tcoord);
    tcoord->Delete();

}

NeedleVtkMapper2D::NeedleVtkMapper2D()
{
}


NeedleVtkMapper2D::~NeedleVtkMapper2D()
{
}

const mitk::Surface *NeedleVtkMapper2D::GetInput() const
{
    return static_cast<Surface *>(GetDataNode()->GetData());
}


vtkProp* NeedleVtkMapper2D::GetVtkProp(mitk::BaseRenderer *renderer)
{
    LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
    return ls->m_Assembly;
}

bool PointOnLineSegment(const mitk::Point3D& point, const mitk::Point3D& p1, const mitk::Point3D& p2)
{
    mitk::Vector3D v1 = p1 - point;
    mitk::Vector3D v2 = p2 - point;
    v1.Normalize();
    v2.Normalize();
    return v1*v2 < 0;
}

bool PointAbovePlane(const mitk::Point3D& point,const mitk::Point3D& pointOnPlane, mitk::Vector3D& normal)
{
    mitk::Vector3D v = point - pointOnPlane;
    v.Normalize();
    normal.Normalize();
    return v* normal > 0;
}

void NeedleVtkMapper2D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
    renderer->GetRenderWindow()->LineSmoothingOn();
    LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
    ls->UpdateGenerateDataTime();

    Surface::Pointer input = const_cast<Surface*>(this->GetInput());
    mitk::Vector3D needleDirection = input->GetUpdatedGeometry()->GetAxisVector(2);  
    mitk::Vector3D needleWide = input->GetGeometry()->GetAxisVector(0);
    mitk::Point3D needleOrigin = input->GetGeometry()->GetOrigin();  
    mitk::Point3D needleEnd = needleOrigin - needleDirection;
    double length = (needleOrigin - needleEnd).GetVnlVector().magnitude();
    mitk::Point2D needleOrigin2d, needleEnd2d;

    mitk::PlaneGeometry::ConstPointer planeGeometry = renderer->GetCurrentWorldGeometry2D();

    //judge if intersect
    mitk::Point3D intersectPoint;
    mitk::Line3D  needleLine(needleOrigin, needleDirection);
    bool  intersect = false;
    bool originAbovePlane = true;
    bool endAbovePlane = true;
    bool  superposition = false;
    if (planeGeometry->IntersectionPoint(needleLine, intersectPoint))
    {
        intersect = PointOnLineSegment(intersectPoint, needleOrigin, needleEnd);
    }
    else
    {
        /*if (planeGeometry->IsOnPlane(needleLine))
        {
            superposition = true;
        }*/

        mitk::Vector3D spacing = planeGeometry->GetSpacing();
        double relax = vtkMath::Max(spacing.GetElement(0), vtkMath::Max(spacing.GetElement(1), spacing.GetElement(2)));
        double distanceOrigin = planeGeometry->Distance(needleOrigin);
        double distanceEnd = planeGeometry->Distance(needleEnd);
        if (distanceOrigin < relax&&distanceEnd < relax)
        {
            superposition = true;
        }
    }
    originAbovePlane = PointAbovePlane(needleOrigin, planeGeometry->GetCenter(), planeGeometry->GetNormal());
    endAbovePlane = PointAbovePlane(needleEnd, planeGeometry->GetCenter(), planeGeometry->GetNormal());

    planeGeometry->Project(needleEnd, needleEnd);
    planeGeometry->Project(needleOrigin, needleOrigin);
    renderer->WorldToDisplay(needleEnd, needleEnd2d);
    renderer->WorldToDisplay(needleOrigin, needleOrigin2d);

    //draw line
    if (intersect)
    {
        mitk::Point2D intersectPoint2d;
        planeGeometry->Project(intersectPoint, intersectPoint);
        renderer->WorldToDisplay(intersectPoint, intersectPoint2d);

        vtkSmartPointer<vtkLineSource> line1 =
            vtkSmartPointer<vtkLineSource>::New();
        line1->SetPoint1(needleEnd2d.GetDataPointer());
        line1->SetPoint2(intersectPoint2d.GetDataPointer());
        line1->Update();

        vtkSmartPointer<vtkLineSource> line2 =
            vtkSmartPointer<vtkLineSource>::New();
        line2->SetPoint1(intersectPoint2d.GetDataPointer());
        line2->SetPoint2(needleOrigin2d.GetDataPointer());
        line2->Update();

        if (originAbovePlane)
        {
            ls->m_lineMapper->SetInputData(line2->GetOutput());
            ls->m_lineBackMapper->SetInputData(line1->GetOutput());
        }
        else
        {
            ls->m_lineMapper->SetInputData(line1->GetOutput());
            ls->m_lineBackMapper->SetInputData(line2->GetOutput());
        }
        auto intersect = vtkSmartPointer<vtkPolyData>::New();
        CreateIntersect2D(intersectPoint2d, intersect.Get());
        ls->m_intersectMapper->SetInputData(intersect);
    }
    else
    {
        vtkSmartPointer<vtkLineSource> lineSource =
            vtkSmartPointer<vtkLineSource>::New();
        lineSource->SetPoint1(needleEnd2d.GetDataPointer());
        lineSource->SetPoint2(needleOrigin2d.GetDataPointer());
        lineSource->Update();
        if (originAbovePlane&&endAbovePlane)
        {
            ls->m_lineMapper->SetInputData(lineSource->GetOutput());
            ls->m_lineBackMapper->SetInputData(nullptr);
        }
        else
        {
            ls->m_lineBackMapper->SetInputData(lineSource->GetOutput());
            ls->m_lineMapper->SetInputData(nullptr);
        }     
        ls->m_intersectMapper->SetInputData(nullptr);
    }

     //draw cross
    auto cross = vtkSmartPointer<vtkPolyData>::New();
    CreateCross2D(needleOrigin2d, cross.Get(), 5);
     ls->m_crosshairMapper->SetInputData(cross);

     float color[3];
     double intersectColor[] = { 1.0,1.0,0.0 };
     double superpositionColor[] = { 0.0, 0.0, 1.0 };
     double extensionColor[] = { 1.0,1.0,0.0 };
     float superpositionLineWidth = 2.0;
     float defaultLineWidth = 1.0;
     if (GetDataNode()->GetColor(color))
     {
         if (superposition)
         {
             ls->m_lineActor->GetProperty()->SetColor(superpositionColor);
             ls->m_lineActor->GetProperty()->SetLineWidth(superpositionLineWidth);
         }
         else
         {
             ls->m_lineActor->GetProperty()->SetColor(color[0], color[1], color[2]);
             ls->m_lineBackActor->GetProperty()->SetColor(color[0], color[1], color[2]);
             ls->m_lineActor->GetProperty()->SetLineWidth(defaultLineWidth);
         }
         ls->m_crossHairActor->GetProperty()->SetColor(color[0], color[1], color[2]);
         ls->m_intersectActor->GetProperty()->SetColor(intersectColor);
     }

     //draw extension line
     auto extensionCord = vtkSmartPointer<vtkPolyData>::New();
     mitk::Vector3D directionProj = needleOrigin - needleEnd;
     directionProj.Normalize();
     CreateExtensionCord(needleOrigin, directionProj, planeGeometry->GetNormal(),length, extensionCord.Get());
     ls->m_extensionMapper->SetInputData(extensionCord);
     ls->m_extensionActor->GetProperty()->SetColor(extensionColor);
}

void NeedleVtkMapper2D::CreateExtensionCord(const mitk::Point3D& startPosition, const mitk::Vector3D& direction, const mitk::Vector3D& normal, double length, vtkPolyData* pOutput)
{
    mitk::Vector3D vDirection;
    vDirection.SetElement(0,direction[1] * normal[2] - direction[2] * normal[1]);
    vDirection.SetElement(1, direction[2] * normal[0] - direction[0] * normal[2]);
    vDirection.SetElement(2, direction[0] * normal[1] - direction[1] * normal[0]);

    vDirection.Normalize();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    double step = 5.0;
    int numOfStick = vtkMath::Ceil(length / 5.0);
    vtkIdType startID, endID;
    for (int i=0;i<numOfStick;i++)
    {
        mitk::Point3D temp0 =  startPosition + direction*step*i;
        mitk::Point3D temp1 = temp0 + vDirection*(i%2?2.0:4.0);
        vtkIdType id0 =  points->InsertNextPoint(temp0.GetDataPointer());
        vtkIdType id1 = points->InsertNextPoint(temp1.GetDataPointer());
        vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, id0);
        line->GetPointIds()->SetId(1, id1);
        lines->InsertNextCell(line);

        if (i==0)
        {
            startID = id0;
        }
        if (i== numOfStick-1)
        {
            endID = id0;
        }
    }
    vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, startID);
    line->GetPointIds()->SetId(1, endID);
    lines->InsertNextCell(line);

    pOutput->SetPoints(points);
    pOutput->SetLines(lines);

}

void NeedleVtkMapper2D::CreateIntersect2D(const mitk::Point2D& intersectPosition, vtkPolyData* pOutput, double radius)
{
    auto source = vtkSmartPointer<vtkRegularPolygonSource>::New();
    source->SetCenter(intersectPosition.GetElement(0), intersectPosition.GetElement(1), 0);
    source->SetNumberOfSides(radius*10);
    source->SetRadius(radius);
    source->Update();
    pOutput->DeepCopy(source->GetOutput());

}

void NeedleVtkMapper2D::CreateCross2D(const mitk::Point2D& crossPosition, vtkPolyData* pOutput, double size)
{
    vtkSmartPointer<vtkPoints> crossPoints = vtkSmartPointer<vtkPoints>::New();
    
    Vector2D horz, vert;
    horz[0] = size;
    horz[1] = 0;
    vert[0] = 0;
    vert[1] = size;
    mitk::Point2D temp;
    temp = crossPosition - horz;
    crossPoints->InsertNextPoint(temp[0], temp[1], 0);
    temp = crossPosition + horz;
    crossPoints->InsertNextPoint(temp[0], temp[1], 0);
    temp = crossPosition - vert;
    crossPoints->InsertNextPoint(temp[0], temp[1], 0);
    temp = crossPosition + vert;
    crossPoints->InsertNextPoint(temp[0], temp[1], 0);

    pOutput->SetPoints(crossPoints.Get());

    vtkSmartPointer<vtkLine> line0 =
        vtkSmartPointer<vtkLine>::New();
    line0->GetPointIds()->SetId(0, 0);
    line0->GetPointIds()->SetId(1, 1);

    vtkSmartPointer<vtkLine> line1 =
        vtkSmartPointer<vtkLine>::New();
    line1->GetPointIds()->SetId(0, 2);
    line1->GetPointIds()->SetId(1, 3);

    vtkSmartPointer<vtkCellArray> lines =
        vtkSmartPointer<vtkCellArray>::New();
    lines->InsertNextCell(line0);
    lines->InsertNextCell(line1);
    pOutput->SetLines(lines);
}

} //end namspace mitk