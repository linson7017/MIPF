#include "GuideWireMoulding.h"

//vtk
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkMath.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkTransform.h>
#include<vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkGenericCell.h>
#include <vtkCell.h>
#include <vtkPolyLine.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkKochanekSpline.h>
#include <vtkPolyDataNormals.h>

GuideWireMoulding::GuideWireMoulding()
{
    m_addvanceLength = 0.5;
    m_bendingAngle = 15.0;
    m_wireRadius = 0.1;
    m_invert = false;
    m_bInitialized = false;
    m_bendFactor = 0.0;
    m_cellLocator = vtkSmartPointer<vtkCellLocator>::New();
    m_pointLocator = vtkSmartPointer<vtkPointLocator>::New();
}


GuideWireMoulding::~GuideWireMoulding()
{
}

void GuideWireMoulding::SetSurface(vtkPolyData* pSurface)
{
    if (!m_vessel)
    {
        m_vessel = vtkSmartPointer<vtkPolyData>::New();
    }
    m_vessel->DeepCopy(pSurface);
    m_bInitialized = false;
}
void GuideWireMoulding::SetCenterLine(vtkPolyData* pCenterLine, bool bDirectionInvert)
{
    if (!m_centerLine)
    {
        m_centerLine = vtkSmartPointer<vtkPolyData>::New();
    }
    m_centerLine->DeepCopy(pCenterLine);
    m_invert = bDirectionInvert;
    m_bInitialized = false;
}

void GuideWireMoulding::SetAdvanceStep(double dStepLength)
{
    m_addvanceLength = dStepLength;
}
void GuideWireMoulding::SetGuideWireRadius(double dRadius)
{
    m_wireRadius = dRadius;
}

void GuideWireMoulding::SetBendFactor(double dBendFactor)
{
    m_bendFactor = dBendFactor;
}

void GuideWireMoulding::Initialize()
{
    InitializeCenterLine();
    InitializeSurface();
    m_bInitialized = true;
}

void GuideWireMoulding::InitializeCenterLine()
{
    vtkPoints* points = m_centerLine->GetPoints();
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
    vtkDoubleArray* radiusDataDouble = vtkDoubleArray::SafeDownCast(m_centerLine->GetPointData()->GetArray("Radius"));
    bool hasRadiurArray = radiusDataDouble ? true : false;

    double centerLineLength = 0.0;
    itk::Vector<double> ptDirection;
    for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
    {
        double p1[3], p2[3];
        points->GetPoint(i, p1);
        if (i<points->GetNumberOfPoints()-1)
        {
            points->GetPoint(i + 1, p2);
            ptDirection[0] = p2[0] - p1[0];
            ptDirection[1] = p2[1] - p1[1];
            ptDirection[2] = p2[2] - p1[2];
            ptDirection.Normalize();
            centerLineLength += sqrt(vtkMath::Distance2BetweenPoints(p1, p2));
        }
        vtkIdType id = centerLinePoints->InsertNextPoint(p1);
        directionArray->InsertTuple(id, ptDirection.GetDataPointer());
        if (hasRadiurArray)
        {
            radiusArray->InsertTuple1(id, radiusDataDouble->GetTuple1(i));
        }
    }
    m_centerLinePointsPolyData->GetPointData()->AddArray(directionArray);
    if (hasRadiurArray)
    {
        m_centerLinePointsPolyData->GetPointData()->AddArray(radiusArray);
    }
    m_centerLinePointsPolyData->SetPoints(centerLinePoints);

    m_pointLocator->SetDataSet(m_centerLinePointsPolyData);
    m_pointLocator->BuildLocator();
}

void GuideWireMoulding::InitializeSurface()
{
    vtkFloatArray* normalDataFloat = vtkFloatArray::SafeDownCast(m_vessel->GetCellData()->GetArray("Normals"));
    if (!normalDataFloat)
    {
        auto normalGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
        normalGenerator->SetInputData(m_vessel);
        normalGenerator->ComputePointNormalsOff();
        normalGenerator->ComputeCellNormalsOn();
        normalGenerator->Update();
    }
    m_cellLocator->SetDataSet(m_vessel);
    m_cellLocator->BuildLocator();
}


void GuideWireMoulding::Simulate(double* pEntryPoint, double* pAdvanceDirection, double dLength, WirePointsType& pathPoints)
{
    if (!m_bInitialized)
    {
        std::cout << "Initialize Guidewire Simulation !" << std::endl;
        Initialize();
    }
    pathPoints.clear();
    itk::Point<double> currentPoint = pEntryPoint;
    itk::Vector<double> direction = pAdvanceDirection;
    direction.Normalize();
    itk::Point<double> nextPoint;

    int numOfInsertedPoint = 0;
    bool bCollided = false;
    while (numOfInsertedPoint<(dLength / m_addvanceLength))
    {
        pathPoints.push_back(WirePoint(currentPoint.GetDataPointer()));
        pathPoints.at(pathPoints.size() - 1).isContact = bCollided;
        bCollided = false;

        Advance(currentPoint, direction, nextPoint);

        itk::Point<double> collidedPoint;
        itk::Vector<double> vesselNormal;
        if (isCollided(m_cellLocator, m_vessel, currentPoint, nextPoint, collidedPoint, vesselNormal))
        {
            //pathPoints.at(pathPoints.size() - 1).CaculateContactAngle(vesselNormal.GetDataPointer());
            bCollided = true;
            ProcessCollision(currentPoint, nextPoint, collidedPoint, vesselNormal);
        }
        itk::Vector<double> curDirection = nextPoint - currentPoint;
        curDirection.Normalize();
        pathPoints.at(pathPoints.size() - 1).SetDirection(curDirection.GetDataPointer());
        if (pathPoints.size()>1)
        {
            pathPoints.at(pathPoints.size() - 1).dK = abs(vtkMath::AngleBetweenVectors(direction.GetDataPointer(), curDirection.GetDataPointer()));
        }
        currentPoint = nextPoint;
        direction = curDirection;
        numOfInsertedPoint++;       
        std::cout << "Tracked Point:" << numOfInsertedPoint << std::endl;
      //  MITK_INFO << "Tracked Point:" << numOfInsertedPoint;
    }

}

double GetSigmoid(double value, double beta, double alpha)
{
    return 1 / (1 + exp(-(value - beta) / alpha));
}


bool GuideWireMoulding::GetNearestCenterLinePointAndDirection(const itk::Point<double>& p, itk::Point<double>& nearest, itk::Vector<double>& direct, double& radius)
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
    if (m_invert)
    {
        direct = -direct;
    }
    direct.Normalize();

    return true;
}

bool GuideWireMoulding::CaculateFanIntersect(itk::Point<double>& center, itk::Vector<double>& startDirection, double radius, itk::Vector<double>& normal, itk::Point<double>& intersectPoint)
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
    itk::Vector<double> tempDirection = startDirection;
    itk::Point<double> current = center + tempDirection*radius;
    RotateVector(5, normal, tempDirection);
    tempDirection.Normalize();
    itk::Point<double> next = center + tempDirection*radius;
    int time = 0;
    while (m_cellLocator->IntersectWithLine(
        current.GetDataPointer(), next.GetDataPointer(), tolerance, t, pline, pcoords, subId, cellId, cell) == 0)
    {
        if (time > 18)
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

    intersectPoint = pline;
    return true;

}

bool GuideWireMoulding::isCollided(vtkCellLocator* locator, vtkPolyData* polydata, itk::Point<double>& current, itk::Point<double>& pre,
    itk::Point<double>& o_collisionPoint, itk::Vector<double>& o_normal)     const
{
    double tn[] = { 0.0,0.0,0.0 };
    o_normal = itk::Vector<double>(tn);
    o_collisionPoint = current;
    //Find the closest points to TestPoint
    itk::Vector<double> n = pre - current;
    n.Normalize();
    itk::Point<double> point0 = pre + n*m_addvanceLength / 2.0;
    itk::Point<double> point1 = current - n*m_addvanceLength / 2.0;
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
        //get normals
        vtkFloatArray* normalDataFloat = vtkFloatArray::SafeDownCast(polydata->GetCellData()->GetArray("Normals"));
        if (normalDataFloat)
        {
            double* normal = normalDataFloat->GetTuple(cellId);
            o_normal = normal;
            o_normal.Normalize();
        }
        return true;
    }
    else
    {
        return false;
    }
}


/*
wire structure at the top
            dir[0]             dir[1]     dir[i - 1]        dir[i]
wire[0]------ > wire[1]------->...------ > wire[i]------ > wire[i + 1]
*/
void GuideWireMoulding::Moulde(double dReleaseRatio, double dMouldeLength, WirePointsType& pathPoints)
{
    int benNum = dMouldeLength / m_addvanceLength;
    for (int i = benNum; i >= 1; i--)
    {
        /*if (!wire[i].isContact)
        {
        continue;
        }*/
        double angle = (dReleaseRatio - 1)*pathPoints[i].dK * 180 / vtkMath::Pi();
        double rotateAxis[3];
        vtkMath::Cross(pathPoints[i].direction, pathPoints[i - 1].direction, rotateAxis);
        vtkMath::Normalize(rotateAxis);
        for (int j = i - 1; j >= 0; j--)
        {
            double v[] = { pathPoints[j].position[0] - pathPoints[i].position[0] ,
                pathPoints[j].position[1] - pathPoints[i].position[1] ,
                pathPoints[j].position[2] - pathPoints[i].position[2] };
            RotateVector(angle, rotateAxis, v);
            pathPoints[j].position[0] = pathPoints[i].position[0] + v[0];
            pathPoints[j].position[1] = pathPoints[i].position[1] + v[1];
            pathPoints[j].position[2] = pathPoints[i].position[2] + v[2];

            double dir[] = { pathPoints[j + 1].position[0] - pathPoints[j].position[0],
                pathPoints[j + 1].position[1] - pathPoints[j].position[1] ,
                pathPoints[j + 1].position[2] - pathPoints[j].position[2] };
            vtkMath::Normalize(dir);
            pathPoints[j].SetDirection(dir);
        }
    }
}

void GuideWireMoulding::Smooth(double dAngleThreshold, WirePointsType& pathPoints)
{
    double rt = dAngleThreshold*vtkMath::Pi() / 180.0;
    for (int i = 1; i < pathPoints.size() - 1; i++)
    {
        if (pathPoints[i].dK > rt)
        {
            double t = 0.5;
            itk::Vector<double> p0 = pathPoints[i - 1].position;
            itk::Vector<double> p1 = pathPoints[i].position;
            itk::Vector<double> p2 = pathPoints[i + 1].position;
            itk::Vector<double> np = (1 - t)*(1 - t)*p0 + 2 * t*(1 - t)*p1 + t*t*p2;
            pathPoints[i].SetPosition(np.GetDataPointer());
        }
    }
}

void GuideWireMoulding::RotateVector(double angle, itk::Vector<double>& rotateAxis, itk::Vector<double>& o_vector)
{
    double rotatedNormal[3];
    auto rotateTransform = vtkSmartPointer<vtkTransform>::New();
    rotateTransform->RotateWXYZ(angle, rotateAxis.GetDataPointer());
    rotateTransform->TransformVector(o_vector.GetDataPointer(), rotatedNormal);
    o_vector = rotatedNormal;

}

void GuideWireMoulding::RotateVector(double angle, double* rotateAxis, double* o_vector)
{
    auto rotateTransform = vtkSmartPointer<vtkTransform>::New();
    rotateTransform->RotateWXYZ(angle, rotateAxis);
    rotateTransform->TransformVector(o_vector, o_vector);
}

void GuideWireMoulding::ProcessCollision(itk::Point<double>& current, itk::Point<double>& next, itk::Point<double>& collided, itk::Vector<double>& normal)
{
    itk::Vector<double> direction = next - current;
    direction.Normalize();
    itk::Vector<double> rotateDirection = direction - normal*vtkMath::Dot(direction.GetDataPointer(), normal.GetDataPointer());
    rotateDirection.Normalize();

    itk::Point<double> ncp;   //nearest centerline point
    itk::Vector<double> ncpd; //  nearest centerline point direction
    double nr;
    GetNearestCenterLinePointAndDirection(current, ncp, ncpd, nr);

    double ra[3];
    if (vtkMath::Dot(rotateDirection.GetDataPointer(), ncpd.GetDataPointer()) < 0)
    {
        vtkMath::Cross(rotateDirection.GetDataPointer(), normal.GetDataPointer(), ra);
    }
    else
    {
        vtkMath::Cross(normal.GetDataPointer(), rotateDirection.GetDataPointer(), ra);
    }
    itk::Vector<double> rotateAxis = ra;
    rotateAxis.Normalize();

    itk::Point<double> outputPoint = next;
    if (CaculateFanIntersect(current, direction, m_addvanceLength, rotateAxis, outputPoint))
    {
        next = outputPoint - m_wireRadius*normal;
    }

}

bool GuideWireMoulding::Advance(const itk::Point<double>& currentPoint, const itk::Vector<double>& direction, itk::Point<double>& nextPoint)
{
    itk::Vector<double> centerLineDirection;
    itk::Point<double> nearestPoint;
    double nearestRadius;
    if (!GetNearestCenterLinePointAndDirection(currentPoint, nearestPoint, centerLineDirection, nearestRadius))
    {
        return false;
    }
    //calculate the rotate angle accroding to the nearest centerline point
    double dis = (nearestPoint - currentPoint).GetNorm();
    if (nearestRadius < 0)
    {
        nearestRadius = 2.0;
    }
    double delta = abs(nearestRadius - dis) / nearestRadius;
    double angle = vtkMath::AngleBetweenVectors(direction.GetDataPointer(), centerLineDirection.GetDataPointer());
    angle = m_bendingAngle*GetSigmoid(angle / vtkMath::Pi(), 0.5, 0.1) + m_bendingAngle* GetSigmoid(delta, 0.8, 0.05);
    angle = m_addvanceLength*angle*(1.0-m_bendFactor);

    //rotate the direction to the centerline
    double rotateAxis[3];
    vtkMath::Cross(direction.GetDataPointer(), centerLineDirection.GetDataPointer(), rotateAxis);
    double rotateDirection[3];
    auto rotateTransform = vtkSmartPointer<vtkTransform>::New();
    rotateTransform->RotateWXYZ(angle, rotateAxis);
    rotateTransform->TransformVector(direction.GetDataPointer(), rotateDirection);
    vtkMath::Normalize(rotateDirection);

    //refresh the next point
    nextPoint = currentPoint + itk::Vector<double>(rotateDirection) *m_addvanceLength;
    return true;
}
