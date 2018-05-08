#include "WireMouldingView.h"
#include "mitkPointSet.h"
#include "mitkVtkScalarModeProperty.h"
#include "mitkLookupTable.h"
#include "mitkLookupTableProperty.h"

#include "vtkLookupTable.h"

//vtkPolydata
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkAppendPolyData.h>
#include <vtkArrowSource.h>
#include <vtkMath.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkMutableDirectedGraph.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkTransform.h>
#include <vtkSphereSource.h>
#include<vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkGenericCell.h>
#include <vtkCell.h>
#include <vtkPolyLine.h>
#include <vtkTubeFilter.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkKochanekSpline.h>
#include <vtkFillHolesFilter.h>
#include <vtkPlane.h>
#include <vtkClipPolyData.h>
#include <vtkFeatureEdges.h>
#include <vtkStripper.h>
#include <vtkCleanPolyData.h>
#include <vtkTriangleFilter.h>
#include <vtkLinearSubdivisionFilter.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkvmtkCapPolyData.h>
#include <ctkSliderWidget.h>
#include "Rendering/ColoredSurfaceVtkMapper.h"


#include "Core/IQF_ObjectFactory.h"
#include "VesselTools/IQF_VesselSegmentationTool.h"

WireMouldingView::WireMouldingView()
{
    m_addvanceLength = 0.5;
    m_bendingAngle = 10.0;
    m_wireRadius = 0.1;
    m_invert = false;
}


WireMouldingView::~WireMouldingView()
{
}

void WireMouldingView::CreateView()
{
    m_ui.setupUi(this);

    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(CreateSurfacePredicate());
    connect(m_ui.ApplyBtn, &QPushButton::clicked, this, &WireMouldingView::Apply);


    m_ui.TensionSlider->setRange(-1.0, 1.0);
    m_ui.ContinuitySlider->setRange(-1.0, 1.0);
    m_ui.BiasSlider->setRange(-1.0, 1.0);
    m_ui.TensionSlider->setSingleStep(0.1);
    m_ui.ContinuitySlider->setSingleStep(0.1);
    m_ui.BiasSlider->setSingleStep(0.1);
    m_ui.TensionSlider->setTracking(false);
    m_ui.ContinuitySlider->setTracking(false);
    m_ui.BiasSlider->setTracking(false);
    connect(m_ui.TensionSlider, SIGNAL(valueChanged(double)), this, SLOT(SplineParameterChanged(double)));
    connect(m_ui.ContinuitySlider, SIGNAL(valueChanged(double)), this, SLOT(SplineParameterChanged(double)));
    connect(m_ui.BiasSlider, SIGNAL(valueChanged(double)), this, SLOT(SplineParameterChanged(double)));


    m_cellLocator = vtkSmartPointer<vtkCellLocator>::New();
    m_pointLocator = vtkSmartPointer<vtkPointLocator>::New();
    m_enclosedPoints = vtkSmartPointer<vtkSelectEnclosedPoints>::New();
    m_capCellLocator = vtkSmartPointer<vtkCellLocator>::New();
    m_capedVessel = vtkSmartPointer<vtkPolyData>::New();
}

vtkPolyData* TransformArrow(double* startPoint,double* endPoint,vtkPolyData* output)
{
    vtkSmartPointer<vtkArrowSource> arrowSource =
        vtkSmartPointer<vtkArrowSource>::New();
    arrowSource->Update();
    double normalizedX[3];
    double normalizedY[3];
    double normalizedZ[3];

    // The X axis is a vector from start to end
    vtkMath::Subtract(endPoint, startPoint, normalizedX);
    double length = vtkMath::Norm(normalizedX);
    vtkMath::Normalize(normalizedX);

    // The Z axis is an arbitrary vector cross X
    double arbitrary[3];
    arbitrary[0] = vtkMath::Random(-10, 10);
    arbitrary[1] = vtkMath::Random(-10, 10);
    arbitrary[2] = vtkMath::Random(-10, 10);
    vtkMath::Cross(normalizedX, arbitrary, normalizedZ);
    vtkMath::Normalize(normalizedZ);

    // The Y axis is Z cross X
    vtkMath::Cross(normalizedZ, normalizedX, normalizedY);
    vtkSmartPointer<vtkMatrix4x4> matrix =
        vtkSmartPointer<vtkMatrix4x4>::New();

    // Create the direction cosine matrix
    matrix->Identity();
    for (unsigned int i = 0; i < 3; i++)
    {
        matrix->SetElement(i, 0, normalizedX[i]);
        matrix->SetElement(i, 1, normalizedY[i]);
        matrix->SetElement(i, 2, normalizedZ[i]);
    }

    // Apply the transforms
    vtkSmartPointer<vtkTransform> transform =
        vtkSmartPointer<vtkTransform>::New();
    transform->Translate(startPoint);
    transform->Concatenate(matrix);
    transform->Scale(0.3, 0.3, 0.3);

    // Transform the polydata
    vtkSmartPointer<vtkTransformPolyDataFilter> transformPD =
        vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformPD->SetTransform(transform);
    transformPD->SetInputData(arrowSource->GetOutput());
    transformPD->Update();

    
    output->DeepCopy(transformPD->GetOutput());
    return output;
}

bool WireMouldingView::isCollided(vtkCellLocator* locator, vtkPolyData* polydata, mitk::Point3D& current, mitk::Point3D& pre,
    mitk::Point3D& o_collisionPoint, mitk::Vector3D& o_normal)     const
{
    double tn[] = { 0.0,0.0,0.0 };
    o_normal = mitk::Vector3D(tn);
    o_collisionPoint = current;
    //Find the closest points to TestPoint
    mitk::Vector3D n = pre - current;
    n.Normalize();
    mitk::Point3D point0 = pre + n*m_addvanceLength/2.0;
    mitk::Point3D point1 = current - n*m_addvanceLength / 2.0;
    vtkIdType cellId;
    int subId;
    auto  cellIDs = vtkSmartPointer<vtkIdList>::New();
    double t = 0.0;
    double tolerance = 0.001;
    double pline[3] = { 0, 0, 0 };
    double pcoords[3] = { 0, 0, 0 };
    auto cell = vtkSmartPointer<vtkGenericCell>::New();

    if (locator->IntersectWithLine(point0.GetDataPointer(), point1.GetDataPointer(), tolerance, t, pline, pcoords, subId, cellId, cell) != 0)
    {
        // Enters here if there is intersection
        o_collisionPoint = pline;
        //calculate normals
        vtkFloatArray* normalDataFloat = vtkFloatArray::SafeDownCast(polydata->GetCellData()->GetArray("Normals"));
        if (normalDataFloat)
        {
            double* normal = normalDataFloat->GetTuple(cellId);
            o_normal= normal;
            o_normal.Normalize();
        }
        return true;
    }
    else
    {
        return false;
    }
}


void WireMouldingView::RotateVector(double angle, mitk::Vector3D& rotateAxis, mitk::Vector3D& o_vector)
{
    double rotatedNormal[3];
    auto rotateTransform = vtkSmartPointer<vtkTransform>::New();
    rotateTransform->RotateWXYZ(angle, rotateAxis.GetDataPointer());
    rotateTransform->TransformVector(o_vector.GetDataPointer(), rotatedNormal);
    o_vector = rotatedNormal;

}

void WireMouldingView::RotateVector(double angle, double* rotateAxis, double* o_vector)
{
    auto rotateTransform = vtkSmartPointer<vtkTransform>::New();
    rotateTransform->RotateWXYZ(angle, rotateAxis);
    rotateTransform->TransformVector(o_vector, o_vector);
}

bool WireMouldingView::CaculateFanIntersect(mitk::Point3D& center, mitk::Vector3D& startDirection, double radius, mitk::Vector3D& normal, mitk::Point3D& intersectPoint)
{
    auto helperPolydata = vtkSmartPointer<vtkPolyData>::New();
    auto helperPoints = vtkSmartPointer<vtkPoints>::New();
    auto helperCells = vtkSmartPointer<vtkCellArray>::New();
    auto polyLine = vtkSmartPointer<vtkPolyLine>::New();

    int subId;
    vtkIdType cellId;
    auto  cellIDs = vtkSmartPointer<vtkIdList>::New();
    double t = 0.0;
    double tolerance = 0.001;
    double pline[3] = { 0, 0, 0 };
    double pcoords[3] = { 0, 0, 0 };
    auto cell = vtkSmartPointer<vtkGenericCell>::New();
    mitk::Vector3D tempDirection = startDirection;
    mitk::Point3D current = center + tempDirection*radius;
    RotateVector(5, normal, tempDirection);
    tempDirection.Normalize();
    mitk::Point3D next = center + tempDirection*radius;
    int time = 0;
    while(m_cellLocator->IntersectWithLine(
        current.GetDataPointer(), next.GetDataPointer(), tolerance, t, pline, pcoords, subId, cellId, cell)==0)
    {
        if (time>18)
        {
            return false;
        }
        time++;
        current = next;
        RotateVector(5, normal, tempDirection);
        tempDirection.Normalize();
        next = center + tempDirection*radius;
        helperPoints->InsertNextPoint(next.GetDataPointer());
    }   
    polyLine->GetPointIds()->SetNumberOfIds(time);
    for (unsigned int i = 0; i < time; i++)
    {
        polyLine->GetPointIds()->SetId(i, i);
    }
    helperCells->InsertNextCell(polyLine);
    helperPolydata->SetPoints(helperPoints);
    helperPolydata->SetLines(helperCells);
   // ImportVtkPolyData(helperPolydata.Get(), "helper fan")->SetColor(1.0, 0.0, 1.0);

    intersectPoint = pline;
    return true;
    
}

bool WireMouldingView::IsInside(double* p, vtkImageData* img)
{
    double spacing[3];
    double origin[3];
    img->GetSpacing(spacing);
    img->GetOrigin(origin);
    int i = round((p[0] - origin[0]) / spacing[0]);
    int j = round((p[1] - origin[1]) / spacing[1]);
    int k = round((p[2] - origin[2]) / spacing[2]);

    return static_cast<unsigned char>(img->GetScalarComponentAsFloat(i, j, k, 0))!=0;

}

//can only used for closed surface
bool WireMouldingView::IsInside(double*p)
{
    return m_enclosedPoints->IsInsideSurface(p);
}

bool WireMouldingView::GetNearestCenterLinePointAndDirection(const mitk::Point3D& p, mitk::Point3D& nearest, mitk::Vector3D& direct, double& radius)
{
    vtkIdType nearestPointID = m_pointLocator->FindClosestPoint(p.GetDataPointer());
    double* nearestPoint = m_centerLinePointsPolyData->GetPoint(nearestPointID);
    vtkDoubleArray* directionArray = vtkDoubleArray::SafeDownCast(m_centerLinePointsPolyData->GetPointData()->GetArray("Directions"));
    if (!directionArray)
    {
        return false;
    }
    double* dir = directionArray->GetTuple(nearestPointID);
    if (!nearestPoint || !dir)
    {
        return false;
    }
    nearest = nearestPoint;
    direct = dir;
    vtkDoubleArray* radiusDataFloat = vtkDoubleArray::SafeDownCast(m_centerLinePointsPolyData->GetPointData()->GetArray("Radius"));
    if (radiusDataFloat)
    {
        radius = radiusDataFloat->GetTuple1(nearestPointID);
    }
    if (!m_invert)
    {
        direct = -direct;
    }

    direct.Normalize();

    return true;
}


double GetSigmoid(double value,double beta,double alpha)
{
    return 1/(1 + exp(-(value - beta) / alpha));
}

bool WireMouldingView::Advance(const mitk::Point3D& currentPoint, const mitk::Vector3D& direction, mitk::Point3D& nextPoint)
{
    mitk::Vector3D centerLineDirection;
    mitk::Point3D nearestPoint;
    double nearestRadius;
    if (!GetNearestCenterLinePointAndDirection(currentPoint, nearestPoint, centerLineDirection, nearestRadius))
    {
        return false;
    }
   //calculate the rotate angle accroding to the nearest centerline point
    double dis = (nearestPoint - currentPoint).GetNorm();
    if (nearestRadius<0)
    {
        nearestRadius = 2.0;
    }
    double delta = abs(nearestRadius-dis)/nearestRadius;
    double angle = vtkMath::AngleBetweenVectors(direction.GetDataPointer(), centerLineDirection.GetDataPointer());
    angle = m_bendingAngle*GetSigmoid(angle/vtkMath::Pi(), 0.5, 0.1)+ m_bendingAngle* GetSigmoid(delta, 0.8, 0.05);
    angle = m_addvanceLength*angle;

    //rotate the direction to the centerline
    double rotateAxis[3];
    vtkMath::Cross(direction.GetDataPointer(), centerLineDirection.GetDataPointer(), rotateAxis);
    double rotateDirection[3];
    auto rotateTransform = vtkSmartPointer<vtkTransform>::New();
    rotateTransform->RotateWXYZ(angle, rotateAxis);
    rotateTransform->TransformVector(direction.GetDataPointer(), rotateDirection);
    vtkMath::Normalize(rotateDirection);

    //refresh the next point
    nextPoint = currentPoint + mitk::Vector3D(rotateDirection) *m_addvanceLength;
    return true;
}

void WireMouldingView::ProcessCollision(mitk::Point3D& current, mitk::Point3D& next, mitk::Point3D& collided, mitk::Vector3D& normal)
{
    mitk::Vector3D direction = next - current;
    direction.Normalize();
    mitk::Vector3D rotateDirection = direction - normal*vtkMath::Dot(direction.GetDataPointer(), normal.GetDataPointer());
    rotateDirection.Normalize();

    mitk::Point3D ncp;   //nearest centerline point
    mitk::Vector3D ncpd; //  nearest centerline point direction
    double nr;
    GetNearestCenterLinePointAndDirection(current, ncp, ncpd, nr);

    double ra[3];
    if (vtkMath::Dot(rotateDirection.GetDataPointer(),ncpd.GetDataPointer())<0)
    {
        vtkMath::Cross(rotateDirection.GetDataPointer(), normal.GetDataPointer(), ra);
    }
    else
    {
        vtkMath::Cross(normal.GetDataPointer(), rotateDirection.GetDataPointer(), ra);
    }
    mitk::Vector3D rotateAxis = ra;
    rotateAxis.Normalize();
    
    mitk::Point3D outputPoint = next;
    if (CaculateFanIntersect(current, direction, m_addvanceLength, rotateAxis, outputPoint))
    {
        next = outputPoint -  m_wireRadius*normal;
    }
    
}

void WireMouldingView::ExtractNeckProfile(double* planePoint, double* planeNormal, vtkPolyData* vesselData)
{
    vtkSmartPointer<vtkPlane> plane =
        vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(0, 0, 0);
    plane->SetNormal(1.0, -1.0, -1.0);

    vtkSmartPointer<vtkClipPolyData> clipper =
        vtkSmartPointer<vtkClipPolyData>::New();
    clipper->SetInputData(vesselData);
    clipper->SetClipFunction(plane);
    clipper->SetValue(0);
    clipper->Update();

    vtkSmartPointer<vtkFeatureEdges> boundaryEdges =
        vtkSmartPointer<vtkFeatureEdges>::New();
    boundaryEdges->SetInputData(clipper->GetOutput());
    boundaryEdges->BoundaryEdgesOn();
    boundaryEdges->FeatureEdgesOff();
    boundaryEdges->NonManifoldEdgesOff();
    boundaryEdges->ManifoldEdgesOff();

    vtkSmartPointer<vtkStripper> boundaryStrips =
        vtkSmartPointer<vtkStripper>::New();
    boundaryStrips->SetInputConnection(boundaryEdges->GetOutputPort());
    boundaryStrips->Update();

    // Change the polylines into polygons
    vtkSmartPointer<vtkPolyData> boundaryPoly =
        vtkSmartPointer<vtkPolyData>::New();
    boundaryPoly->SetPoints(boundaryStrips->GetOutput()->GetPoints());
    boundaryPoly->SetPolys(boundaryStrips->GetOutput()->GetLines());
}

void WireMouldingView::Moulde(const mitk::Point3D& start, const mitk::Vector3D& direct, double length, WirePointsType& pathPoints)
{
    pathPoints.clear();
    mitk::Point3D currentPoint = start;
    mitk::Vector3D direction = direct;
    direction.Normalize();
    mitk::Point3D nextPoint;
    
    
    int numOfInsertedPoint = 0;
    bool bCollided = false;
    while (numOfInsertedPoint<(length / m_addvanceLength))
    {
        pathPoints.push_back(WirePoint(currentPoint.GetDataPointer()));
        pathPoints.at(pathPoints.size() - 1).isContact = bCollided;
        bCollided = false;

        Advance(currentPoint, direction, nextPoint);
        
        mitk::Point3D collidedPoint;
        mitk::Vector3D vesselNormal;
        if (isCollided(m_cellLocator, m_vessel->GetVtkPolyData(), currentPoint, nextPoint, collidedPoint, vesselNormal))
        {
            
            pathPoints.at(pathPoints.size() - 1).CaculateContactAngle(vesselNormal.GetDataPointer());
            bCollided = true;
            ProcessCollision(currentPoint, nextPoint, collidedPoint, vesselNormal);
            /*auto sphere = vtkSmartPointer<vtkSphereSource>::New();
            sphere->SetCenter(collidedPoint.GetDataPointer());
            sphere->SetRadius(0.2);
            sphere->Update();
            ImportVtkPolyData(sphere->GetOutput(), "collidedPoint")->SetColor(1.0, 0.0, 0.0);*/
        }
       /* else if (isCollided(m_capCellLocator, m_capedVessel.Get(), currentPoint, nextPoint, collidedPoint, vesselNormal))
        {
            MITK_INFO << "getout!";
            break;
        }*/

        //auto sphere = vtkSmartPointer<vtkSphereSource>::New();
        //sphere->SetCenter(nextPoint.GetDataPointer());
        //sphere->SetRadius(m_wireRadius);
        //sphere->Update();
        //appendFilter->AddInputData(sphere->GetOutput());

        

        mitk::Vector3D curDirection = nextPoint - currentPoint;
        curDirection.Normalize();
        pathPoints.at(pathPoints.size() - 1).SetDirection(curDirection.GetDataPointer());
        if (pathPoints.size()>1)
        {
            pathPoints.at(pathPoints.size() - 1).dK = abs(vtkMath::AngleBetweenVectors(direction.GetDataPointer(), curDirection.GetDataPointer()));
        }
        currentPoint = nextPoint;
        direction = curDirection;
        numOfInsertedPoint++;
        MITK_INFO << "Tracked Point:" << numOfInsertedPoint;
    }
    
}

/*
wire structure at the top
           dir[0]             dir[1]     dir[i - 1]        dir[i]
wire[0]------ > wire[1]------->...------ > wire[i]------ > wire[i + 1]
*/
void WireMouldingView::Bend(double bendRatio, double length, WirePointsType& wire)
{
    int benNum = length / m_addvanceLength;
    for (int i = benNum; i >= 1; i--)
    {
        /*if (!wire[i].isContact)
        {
            continue;
        }*/
        double angle = (bendRatio - 1)*wire[i].dK * 180 / vtkMath::Pi();
        double rotateAxis[3];
        vtkMath::Cross(wire[i - 1].direction,wire[i].direction, rotateAxis);
        vtkMath::Normalize(rotateAxis);
        for (int j=i-1;j>=0;j--)
        {
            double v[] = { wire[j].position[0] - wire[i].position[0] ,
                wire[j].position[1] - wire[i].position[1] ,
                wire[j].position[2] - wire[i].position[2] };
            RotateVector(angle, rotateAxis, v);
            wire[j].position[0] = wire[i].position[0] + v[0];
            wire[j].position[1] = wire[i].position[1] + v[1];
            wire[j].position[2] = wire[i].position[2] + v[2];

            double dir[] = { wire[j + 1].position[0] - wire[j].position[0],
                wire[j + 1].position[1] - wire[j].position[1] ,
                wire[j + 1].position[2] - wire[j].position[2] };
            vtkMath::Normalize(dir);
            wire[j].SetDirection(dir);
        }
    }
}

void WireMouldingView::Smooth(double angleThreshold, WirePointsType& wire)
{
    double rt = angleThreshold*vtkMath::Pi() / 180.0;
    for (int i=1;i<wire.size()-1;i++)
    {
        if (wire[i].dK>rt)
        {
            double t = 0.5;
            mitk::Vector3D p0 = wire[i - 1].position;
            mitk::Vector3D p1 = wire[i ].position;
            mitk::Vector3D p2 = wire[i +1].position;
            mitk::Vector3D np = (1 - t)*(1 - t)*p0 + 2 * t*(1 - t)*p1 + t*t*p2;
            wire[i].SetPosition(np.GetDataPointer());
        }
    }
}

void WireMouldingView::Apply()
{
    mitk::PointSet* ljlxPS = dynamic_cast<mitk::PointSet*>(GetDataStorage()->GetNamedNode("ljlx")->GetData());
    m_ljPoint = ljlxPS->GetPoint(1);
    m_lxPoint = ljlxPS->GetPoint(3);
    mitk::PointSet* rkPS = dynamic_cast<mitk::PointSet*>(GetDataStorage()->GetNamedNode("rk")->GetData());
    m_entryPoint = rkPS->GetPoint(0);
    m_entryDirectionPoint = rkPS->GetPoint(1);

    m_centerLine = dynamic_cast<mitk::Surface*>(GetDataStorage()->GetNamedNode("tumorVesselCenterline")->GetData());
    vtkPoints* points = m_centerLine->GetVtkPolyData()->GetPoints();

    m_centerLinePointsPolyData = vtkSmartPointer<vtkPolyData>::New();
    auto centerLinePoints = vtkSmartPointer<vtkPoints>::New();
    centerLinePoints->SetNumberOfPoints(points->GetNumberOfPoints());
    
    //caculate centerline directions
    vtkSmartPointer<vtkDoubleArray> directionArray = vtkSmartPointer<vtkDoubleArray>::New();
    directionArray->SetName("Directions");
    directionArray->SetNumberOfComponents(3);
    vtkSmartPointer<vtkDoubleArray> radiusArray = vtkSmartPointer<vtkDoubleArray>::New();
    radiusArray->SetName("Radius");
    radiusArray->SetNumberOfComponents(1);

    vtkDoubleArray* radiusDataDouble = vtkDoubleArray::SafeDownCast(m_centerLine->GetVtkPolyData()->GetPointData()->GetArray("Radius"));
    bool hasRadiurArray = radiusDataDouble ? true : false;
    double ptDirection[3] = { 0,0,0 };
    vtkIdType id = 0;
    id = centerLinePoints->InsertNextPoint(points->GetPoint(0));
    directionArray->InsertTuple(id,ptDirection);
    double centerLineLength = 0.0;
    for (vtkIdType i = 1; i < points->GetNumberOfPoints(); i++)
    {
        double p1[3], p2[3];
        points->GetPoint(i-1, p1);
        points->GetPoint(i, p2);
        ptDirection[0] = p1[0] - p2[0];
        ptDirection[1] = p1[1] - p2[1];
        ptDirection[2] = p1[2] - p2[2];
        id = centerLinePoints->InsertNextPoint(p2);
        vtkMath::Normalize(ptDirection);
        directionArray->InsertTuple(id,ptDirection);
        if (hasRadiurArray)
        {
            radiusArray->InsertTuple1(id, radiusDataDouble->GetTuple1(i));
        } 

        centerLineLength += sqrt(vtkMath::Distance2BetweenPoints(p1, p2));
    }
    m_centerLinePointsPolyData->GetPointData()->AddArray(directionArray);
    if (hasRadiurArray)
    {
        m_centerLinePointsPolyData->GetPointData()->AddArray(radiusArray);
    }
    m_centerLinePointsPolyData->SetPoints(centerLinePoints);

   // m_enclosedPoints->Complete();
    m_vessel = dynamic_cast<mitk::Surface*>(GetDataStorage()->GetNamedNode("tumorVessel")->GetData());
    //if vessel does not has normals, generate cell normals
    vtkFloatArray* normalDataFloat = vtkFloatArray::SafeDownCast(m_vessel->GetVtkPolyData()->GetCellData()->GetArray("Normals"));
    if (!normalDataFloat)
    {
       auto normalGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
        normalGenerator->SetInputData(m_vessel->GetVtkPolyData());
        normalGenerator->ComputePointNormalsOff();
        normalGenerator->ComputeCellNormalsOn();
        normalGenerator->Update();
    }
   /* vtkSmartPointer<vtkFeatureEdges> featureEdges =
        vtkSmartPointer<vtkFeatureEdges>::New();
    featureEdges->FeatureEdgesOff();
    featureEdges->BoundaryEdgesOn();
    featureEdges->NonManifoldEdgesOn();
    featureEdges->SetInputData(m_vessel->GetVtkPolyData());
    featureEdges->Update();*/

    //close surface
  /*  IQF_ObjectFactory* pObjectFactory = (IQF_ObjectFactory*)m_pMain->GetInterfacePtr(QF_Core_ObjectFactory);
    IQF_VesselSegmentationTool* pVesselSegTool = (IQF_VesselSegmentationTool*)pObjectFactory->CreateObject("Object_ID_VesselSegmentationTool");

    pVesselSegTool->CapSurface(m_vessel->GetVtkPolyData(), m_capedVessel.Get());
    pVesselSegTool->Release();
    m_capCellLocator->SetDataSet(m_capedVessel);
    m_capCellLocator->BuildLocator();
    m_enclosedPoints->Initialize(m_capedVessel);
    ImportVtkPolyData(m_capedVessel.Get(), "caped surface");*/
    //return;


    m_intersect = dynamic_cast<mitk::Surface*>(GetDataStorage()->GetNamedNode("intersect")->GetData());

  //  m_mask = dynamic_cast<mitk::Image*>(GetDataStorage()->GetNamedNode("tumorVesselSegmentedData")->GetData());
    

    m_cellLocator->SetDataSet(m_vessel->GetVtkPolyData());
    m_cellLocator->BuildLocator();

    m_pointLocator->SetDataSet(m_centerLinePointsPolyData);
    m_pointLocator->BuildLocator();


    mitk::Point3D currentPoint;
    mitk::Vector3D direction;
    m_invert = m_ui.InvertCB->isChecked();
    if (m_invert)
    {
        currentPoint = m_ljPoint;
        direction = m_ljPoint - m_lxPoint;
        direction.Normalize();
        Moulde(currentPoint, direction,centerLineLength, m_returnPathPoints);

    }
    else
    {
        currentPoint = m_entryPoint;
        direction = m_entryDirectionPoint - m_entryPoint;
        direction.Normalize();
        Moulde(currentPoint, direction, centerLineLength, m_enterPathPoints);
    }

    auto wireAppendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
    auto wirePolyData = vtkSmartPointer<vtkPolyData>::New();
    m_pathPoints = vtkSmartPointer<vtkPoints>::New();
    auto wirePolyLine = vtkSmartPointer<vtkPolyLine>::New();
    auto wireCells = vtkSmartPointer<vtkCellArray>::New();
    auto collidedPoints = vtkSmartPointer<vtkPoints>::New();

    WirePointsType path = m_invert ? m_returnPathPoints : m_enterPathPoints;
    auto wireEnergy = vtkSmartPointer<vtkFloatArray>::New();
    wireEnergy->SetName("Curvature");//
    wireEnergy->SetNumberOfTuples(path.size());


    if (m_ui.BendBox->isChecked())
    {
        Bend(m_ui.SpringRatioLE->text().toDouble(), m_ui.BendLengthLE->text().toDouble(), path);
    }
    if (m_ui.SmoothCB->isChecked())
    {
        Smooth(15.0, path);
    }
    DisplayDirectedWire(path,"directedPath");
    for (int i = 0; i < path.size(); i++)
    {
        m_pathPoints->InsertNextPoint(path[i].position);
        wirePolyLine->GetPointIds()->InsertNextId(i);
        wireEnergy->InsertNextTuple1(path[i].dK);
        if (path[i].isContact)
        {
            collidedPoints->InsertNextPoint(path[i].position);
        }
        auto sphere = vtkSmartPointer<vtkSphereSource>::New();
        sphere->SetCenter(path[i].position);
        sphere->SetRadius(m_wireRadius);
        sphere->Update();
        wireAppendFilter->AddInputData(sphere->GetOutput());
    }
    wireCells->InsertNextCell(wirePolyLine);
    wirePolyData->SetPoints(m_pathPoints);
    wirePolyData->SetLines(wireCells);
    wirePolyData->GetPointData()->AddArray(wireEnergy);
    wirePolyData->GetPointData()->SetActiveScalars("Energy");


    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colors->SetName("Colors");//为该数组起名为"Colors"
    colors->SetNumberOfComponents(3);
    colors->SetNumberOfTuples(path.size());
    for (int i = 0; i < path.size(); i++)
    {
        colors->InsertTuple3(i,
            255*path[i].isContact,
            0,
            255 * (1- path[i].isContact));
    }
    wirePolyData->GetPointData()->AddArray(colors);
    

    vtkSmartPointer<vtkTubeFilter> tubeFilter =
        vtkSmartPointer<vtkTubeFilter>::New();
    tubeFilter->SetInputData(wirePolyData);
    tubeFilter->SetRadius(m_wireRadius); //default is .5
    tubeFilter->SetNumberOfSides(path.size());
    tubeFilter->Update();
    wireAppendFilter->Update();
    mitk::DataNode::Pointer tubeNode = ImportVtkPolyData(tubeFilter->GetOutput(), "path");

    mitk::ColoredSurfaceVtkMapper::Pointer colorArrayMapper3D = mitk::ColoredSurfaceVtkMapper::New();
    tubeNode->SetMapper(mitk::BaseRenderer::Standard3D, colorArrayMapper3D);
    tubeNode->SetBoolProperty("scalar visibility", true);
    mitk::VtkScalarModeProperty::Pointer scalarModeProperty = mitk::VtkScalarModeProperty::New();
    scalarModeProperty->SetScalarModeToPointFieldData();
    tubeNode->SetProperty("scalar mode", scalarModeProperty);
    tubeNode->SetStringProperty("ColorArray", "Colors");
    tubeNode->SetDoubleProperty("ScalarsRangeMinimum", 0);
    tubeNode->SetDoubleProperty("ScalarsRangeMaximum", 180);
    tubeNode->Update();

   //Spline path
    double Tension;
    double Continuity;
    double Bias;
    vtkSmartPointer<vtkKochanekSpline> xSpline =
        vtkSmartPointer<vtkKochanekSpline>::New();
    vtkSmartPointer<vtkKochanekSpline> ySpline =
        vtkSmartPointer<vtkKochanekSpline>::New();
    vtkSmartPointer<vtkKochanekSpline> zSpline =
        vtkSmartPointer<vtkKochanekSpline>::New();
    vtkSmartPointer<vtkParametricSpline> spline =
        vtkSmartPointer<vtkParametricSpline>::New();
    spline->SetXSpline(xSpline);
    spline->SetYSpline(ySpline);
    spline->SetZSpline(zSpline);
    spline->SetPoints(m_pathPoints);
    vtkSmartPointer<vtkParametricFunctionSource> functionSource =
        vtkSmartPointer<vtkParametricFunctionSource>::New();
    functionSource->SetParametricFunction(spline);
    functionSource->SetUResolution(10 * m_pathPoints->GetNumberOfPoints());
    functionSource->Update();

    vtkSmartPointer<vtkTubeFilter> splingTubeFilter =
        vtkSmartPointer<vtkTubeFilter>::New();
    splingTubeFilter->SetInputData(functionSource->GetOutput());
    splingTubeFilter->SetRadius(m_wireRadius); //default is .5
    splingTubeFilter->SetNumberOfSides(m_pathPoints->GetNumberOfPoints());
    splingTubeFilter->Update();
    ImportVtkPolyData(splingTubeFilter->GetOutput(), "Splined Path")->SetColor(0.0, 0.0, 1.0);

    ImportVtkPolyData(wireAppendFilter->GetOutput(), "path point")->SetColor(1.0, 1.0, 0.0);

    RequestRenderWindowUpdate();
    //center direction line
    /*vtkPoints* points = centerLine->GetVtkPolyData()->GetPoints();
    if (points)
    {
        auto appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
        double startPoint[3];
        double endPoint[3];
        for (int i = 0; i < points->GetNumberOfPoints(); i++)
        {
            points->GetPoint(i, startPoint);
            points->GetPoint(i + 1, endPoint);
            auto output = vtkSmartPointer<vtkPolyData>::New();
            TransformArrow(startPoint, endPoint, output.Get());
            appendFilter->AddInputData(output);
        }
        appendFilter->Update();
        ImportVtkPolyData(appendFilter->GetOutput(), "arrows");
    }*/

}           

void WireMouldingView::DisplayDirectedWire( WirePointsType& wire, const char* name)
{
        auto appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
        double startPoint[3];
        double endPoint[3];
        for (int i = 0; i < wire.size()-1; i++)
        {
            auto output = vtkSmartPointer<vtkPolyData>::New();
            TransformArrow(wire[i].position, wire[i+1].position, output.Get());
            appendFilter->AddInputData(output);
        }
        appendFilter->Update();
        ImportVtkPolyData(appendFilter->GetOutput(), name);
}

void WireMouldingView::SplineParameterChanged(double value)
{
    mitk::DataNode* node = GetDataStorage()->GetNamedNode("Splined Path");
    if (!node)
    {
        return;
    }
    mitk::Surface* surface = dynamic_cast<mitk::Surface*>(node->GetData());

    double Tension = m_ui.TensionSlider->value();
    double Continuity = m_ui.ContinuitySlider->value();
    double Bias = m_ui.BiasSlider->value();
    vtkSmartPointer<vtkKochanekSpline> xSpline =
        vtkSmartPointer<vtkKochanekSpline>::New();
    vtkSmartPointer<vtkKochanekSpline> ySpline =
        vtkSmartPointer<vtkKochanekSpline>::New();
    vtkSmartPointer<vtkKochanekSpline> zSpline =
        vtkSmartPointer<vtkKochanekSpline>::New();
    vtkSmartPointer<vtkParametricSpline> spline =
        vtkSmartPointer<vtkParametricSpline>::New();
    xSpline->SetDefaultTension(Tension);
    ySpline->SetDefaultTension(Tension);
    zSpline->SetDefaultTension(Tension);
    xSpline->SetDefaultContinuity(Continuity);
    ySpline->SetDefaultContinuity(Continuity);
    zSpline->SetDefaultContinuity(Continuity);
    xSpline->SetDefaultBias(Bias);
    ySpline->SetDefaultBias(Bias);
    zSpline->SetDefaultBias(Bias);

    spline->SetXSpline(xSpline);
    spline->SetYSpline(ySpline);
    spline->SetZSpline(zSpline);
    spline->SetPoints(m_pathPoints);
    vtkSmartPointer<vtkParametricFunctionSource> functionSource =
        vtkSmartPointer<vtkParametricFunctionSource>::New();
    functionSource->SetParametricFunction(spline);
    functionSource->SetUResolution(10 * m_pathPoints->GetNumberOfPoints());
    functionSource->SetVResolution(10 * m_pathPoints->GetNumberOfPoints());
    functionSource->SetWResolution(10 * m_pathPoints->GetNumberOfPoints());
    functionSource->Update();

    vtkSmartPointer<vtkTubeFilter> splingTubeFilter =
        vtkSmartPointer<vtkTubeFilter>::New();
    splingTubeFilter->SetInputData(functionSource->GetOutput());
    splingTubeFilter->SetRadius(m_wireRadius); //default is .5
    splingTubeFilter->SetNumberOfSides(m_pathPoints->GetNumberOfPoints());
    splingTubeFilter->Update();

    surface->SetVtkPolyData(splingTubeFilter->GetOutput());
    RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
}
