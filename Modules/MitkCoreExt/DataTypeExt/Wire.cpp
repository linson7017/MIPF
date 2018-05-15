

#include "Wire.h"
#include "mitkInteractionConst.h"
#include "mitkPointOperation.h"

#include <iomanip>
#include <mitkNumericTypes.h>

#include "qf_log.h"

mitk::Wire::Wire() : m_CalculateBoundingBox(true)
{
    this->InitializeEmpty();
    ropeSimulation = new RopeSimulation(
        100,						// 80 Particles (Masses)
        0.05f,					// Each Particle Has A Weight Of 50 Grams
        100.0f,				// springConstant In The Rope
        0.0005f,					// Normal Length Of Springs In The Rope
        0.2f,					// Spring Inner Friction Constant
        Vector3F(0, -9.81f, 0), // Gravitational Acceleration
        0.02f,					// Air Friction Constant
        100.0f,					// Ground Repel Constant
        0.2f,					// Ground Slide Friction Constant
        2.0f,					// Ground Absoption Constant
        -1.5f);
}

mitk::Wire::Wire(const Wire &other)
    : BaseData(other), m_WireSeries(other.GetWireSeriesSize()), m_CalculateBoundingBox(true)
{
    // Copy points
    for (std::size_t t = 0; t < m_WireSeries.size(); ++t)
    {
        m_WireSeries[t] = DataType::New();

        DataType::Pointer otherPts = other.GetWire(t);
        for (PointsConstIterator i = other.Begin(t); i != other.End(t); ++i)
        {
            m_WireSeries[t]->SetPoint(i.Index(), i.Value());
            PointDataType pointData;
            if (otherPts->GetPointData(i.Index(), &pointData))
            {
                m_WireSeries[t]->SetPointData(i.Index(), pointData);
            }
        }
    }
}

mitk::Wire::~Wire()
{
    this->ClearData();
}

void mitk::Wire::ClearData()
{
    m_WireSeries.clear();
    Superclass::ClearData();
}

void mitk::Wire::InitializeEmpty()
{
    m_WireSeries.resize(1);

    m_WireSeries[0] = DataType::New();
    PointDataContainer::Pointer pointData = PointDataContainer::New();
    m_WireSeries[0]->SetPointData(pointData);
    m_CalculateBoundingBox = false;

    Superclass::InitializeTimeGeometry(1);
    m_Initialized = true;

    m_EmptyPointsContainer = DataType::PointsContainer::New();
}

bool mitk::Wire::IsEmptyTimeStep(unsigned int t) const
{
    return IsInitialized() && (GetSize(t) == 0);
}

void mitk::Wire::Expand(unsigned int timeSteps)
{
    // Check if the vector is long enough to contain the new element
    // at the given position. If not, expand it with sufficient pre-initialized
    // elements.
    //
    // NOTE: This method will never REDUCE the vector size; it should only
    // be used to make sure that the vector has enough elements to include the
    // specified time step.

    unsigned int oldSize = m_WireSeries.size();

    if (timeSteps > oldSize)
    {
        Superclass::Expand(timeSteps);

        m_WireSeries.resize(timeSteps);
        for (unsigned int i = oldSize; i < timeSteps; ++i)
        {
            m_WireSeries[i] = DataType::New();
            PointDataContainer::Pointer pointData = PointDataContainer::New();
            m_WireSeries[i]->SetPointData(pointData);
        }

        // if the size changes, then compute the bounding box
        m_CalculateBoundingBox = true;

        this->InvokeEvent(WireExtendTimeRangeEvent());
    }
}

unsigned int mitk::Wire::GetWireSeriesSize() const
{
    return m_WireSeries.size();
}

int mitk::Wire::GetSize(unsigned int t) const
{
    if (t < m_WireSeries.size())
    {
        return m_WireSeries[t]->GetNumberOfPoints();
    }
    else
    {
        return 0;
    }
}

mitk::Wire::DataType::Pointer mitk::Wire::GetWire(int t) const
{
    if (t < (int)m_WireSeries.size())
    {
        return m_WireSeries[t];
    }
    else
    {
        return nullptr;
    }
}

mitk::Wire::PointsIterator mitk::Wire::Begin(int t)
{
    if (t >= 0 && t < static_cast<int>(m_WireSeries.size()))
    {
        return m_WireSeries[t]->GetPoints()->Begin();
    }
    return m_EmptyPointsContainer->End();
}

mitk::Wire::PointsConstIterator mitk::Wire::Begin(int t) const
{
    if (t >= 0 && t < static_cast<int>(m_WireSeries.size()))
    {
        return m_WireSeries[t]->GetPoints()->Begin();
    }
    return m_EmptyPointsContainer->End();
}

mitk::Wire::PointsIterator mitk::Wire::End(int t)
{
    if (t >= 0 && t < static_cast<int>(m_WireSeries.size()))
    {
        return m_WireSeries[t]->GetPoints()->End();
    }
    return m_EmptyPointsContainer->End();
}

mitk::Wire::PointsConstIterator mitk::Wire::End(int t) const
{
    if (t >= 0 && t < static_cast<int>(m_WireSeries.size()))
    {
        return m_WireSeries[t]->GetPoints()->End();
    }
    return m_EmptyPointsContainer->End();
}

mitk::Wire::PointsIterator mitk::Wire::GetMaxId(int t)
{
    if ((unsigned int)t >= m_WireSeries.size())
    {
        return m_EmptyPointsContainer->End();
    }

    return this->Begin(t) == this->End(t) ? this->End(t) : --End(t);
}

int mitk::Wire::SearchPoint(Point3D point, ScalarType distance, int t) const
{
    if (t >= (int)m_WireSeries.size())
    {
        return -1;
    }

    // Out is the point which is checked to be the searched point
    PointType out;
    out.Fill(0);
    PointType indexPoint;

    this->GetGeometry(t)->WorldToIndex(point, indexPoint);

    // Searching the first point in the Set, that is +- distance far away fro
    // the given point
    unsigned int i;
    PointsContainer::Iterator it, end;
    end = m_WireSeries[t]->GetPoints()->End();
    int bestIndex = -1;
    distance = distance * distance;

    // To correct errors from converting index to world and world to index
    if (distance == 0.0)
    {
        distance = 0.000001;
    }

    ScalarType bestDist = distance;
    ScalarType dist, tmp;

    for (it = m_WireSeries[t]->GetPoints()->Begin(), i = 0; it != end; ++it, ++i)
    {
        bool ok = m_WireSeries[t]->GetPoints()->GetElementIfIndexExists(it->Index(), &out);

        if (!ok)
        {
            return -1;
        }
        else if (indexPoint == out) // if totally equal
        {
            return it->Index();
        }

        // distance calculation
        tmp = out[0] - indexPoint[0];
        dist = tmp * tmp;
        tmp = out[1] - indexPoint[1];
        dist += tmp * tmp;
        tmp = out[2] - indexPoint[2];
        dist += tmp * tmp;

        if (dist < bestDist)
        {
            bestIndex = it->Index();
            bestDist = dist;
        }
    }
    return bestIndex;
}

mitk::Wire::PointType mitk::Wire::GetPoint(PointIdentifier id, int t) const
{
    PointType out;
    out.Fill(0);

    if ((unsigned int)t >= m_WireSeries.size())
    {
        return out;
    }

    if (m_WireSeries[t]->GetPoints()->IndexExists(id))
    {
        m_WireSeries[t]->GetPoint(id, &out);
        this->GetGeometry(t)->IndexToWorld(out, out);
        return out;
    }
    else
    {
        return out;
    }
}

bool mitk::Wire::GetPointIfExists(PointIdentifier id, PointType *point, int t) const
{
    if ((unsigned int)t >= m_WireSeries.size())
    {
        return false;
    }

    if (m_WireSeries[t]->GetPoints()->GetElementIfIndexExists(id, point))
    {
        this->GetGeometry(t)->IndexToWorld(*point, *point);
        return true;
    }
    else
    {
        return false;
    }
}

void mitk::Wire::SetPoint(PointIdentifier id, PointType point, int t)
{
    // Adapt the size of the data vector if necessary
    this->Expand(t + 1);

    mitk::Point3D indexPoint;
    this->GetGeometry(t)->WorldToIndex(point, indexPoint);
    m_WireSeries[t]->SetPoint(id, indexPoint);
    PointDataType defaultPointData;
    defaultPointData.id = id;
    defaultPointData.selected = false;
    defaultPointData.pointSpec = mitk::PTUNDEFINED;

    m_WireSeries[t]->SetPointData(id, defaultPointData);
    // boundingbox has to be computed anyway
    m_CalculateBoundingBox = true;
    this->Modified();
}

void mitk::Wire::SetPoint(PointIdentifier id, PointType point, PointSpecificationType spec, int t)
{
    // Adapt the size of the data vector if necessary
    this->Expand(t + 1);

    mitk::Point3D indexPoint;
    this->GetGeometry(t)->WorldToIndex(point, indexPoint);
    m_WireSeries[t]->SetPoint(id, indexPoint);
    PointDataType defaultPointData;
    defaultPointData.id = id;
    defaultPointData.selected = false;
    defaultPointData.pointSpec = spec;
    m_WireSeries[t]->SetPointData(id, defaultPointData);
    // boundingbox has to be computed anyway
    m_CalculateBoundingBox = true;
    this->Modified();
}

void mitk::Wire::InsertPoint(PointIdentifier id, PointType point, int t)
{
    this->InsertPoint(id, point, mitk::PTUNDEFINED, t);
}

void mitk::Wire::InsertPoint(PointIdentifier id, PointType point, PointSpecificationType spec, int t)
{
    if ((unsigned int)t < m_WireSeries.size())
    {
        mitk::Point3D indexPoint;
        mitk::BaseGeometry *tempGeometry = this->GetGeometry(t);
        if (tempGeometry == nullptr)
        {
            QF_INFO << __FILE__ << ", l." << __LINE__ << ": GetGeometry of " << t << " returned NULL!" << std::endl;
            return;
        }
        tempGeometry->WorldToIndex(point, indexPoint);
        m_WireSeries[t]->GetPoints()->InsertElement(id, indexPoint);
        PointDataType defaultPointData;
        defaultPointData.id = id;
        defaultPointData.selected = false;
        defaultPointData.pointSpec = spec;
        m_WireSeries[t]->GetPointData()->InsertElement(id, defaultPointData);

        // boundingbox has to be computed anyway
        m_CalculateBoundingBox = true;
        this->Modified();
    }
}

mitk::Wire::PointIdentifier mitk::Wire::InsertPoint(PointType point, int t)
{
    // Adapt the size of the data vector if necessary
    this->Expand(t + 1);

    PointIdentifier id = 0;
    if (m_WireSeries[t]->GetNumberOfPoints() > 0)
    {
        PointsIterator it = --End(t);
        id = it.Index();
        ++id;
    }

    mitk::Point3D indexPoint;
    this->GetGeometry(t)->WorldToIndex(point, indexPoint);
    m_WireSeries[t]->SetPoint(id, indexPoint);
    PointDataType defaultPointData;
    defaultPointData.id = id;
    defaultPointData.selected = false;
    defaultPointData.pointSpec = mitk::PTUNDEFINED;

    m_WireSeries[t]->SetPointData(id, defaultPointData);
    // boundingbox has to be computed anyway
    m_CalculateBoundingBox = true;
    this->Modified();

    return id;
}

bool mitk::Wire::RemovePointIfExists(PointIdentifier id, int t)
{
    if ((unsigned int)t < m_WireSeries.size())
    {
        DataType *pointSet = m_WireSeries[t];

        PointsContainer *points = pointSet->GetPoints();
        PointDataContainer *pdata = pointSet->GetPointData();

        bool exists = points->IndexExists(id);
        if (exists)
        {
            points->DeleteIndex(id);
            pdata->DeleteIndex(id);
            return true;
        }
    }
    return false;
}

mitk::Wire::PointsIterator mitk::Wire::RemovePointAtEnd(int t)
{
    if ((unsigned int)t < m_WireSeries.size())
    {
        DataType *pointSet = m_WireSeries[t];

        PointsContainer *points = pointSet->GetPoints();
        PointDataContainer *pdata = pointSet->GetPointData();

        PointsIterator bit = points->Begin();
        PointsIterator eit = points->End();

        if (eit != bit)
        {
            PointsContainer::ElementIdentifier id = (--eit).Index();
            points->DeleteIndex(id);
            pdata->DeleteIndex(id);
            PointsIterator eit2 = points->End();
            return --eit2;
        }
        else
        {
            return eit;
        }
    }
    return m_EmptyPointsContainer->End();
}

bool mitk::Wire::SwapPointPosition(PointIdentifier id, bool moveUpwards, int t)
{
    if (IndexExists(id, t))
    {
        PointType point = GetPoint(id, t);

        if (moveUpwards)
        { // up
            if (IndexExists(id - 1, t))
            {
                InsertPoint(id, GetPoint(id - 1, t), t);
                InsertPoint(id - 1, point, t);
                this->Modified();
                return true;
            }
        }
        else
        { // down
            if (IndexExists(id + 1, t))
            {
                InsertPoint(id, GetPoint(id + 1, t), t);
                InsertPoint(id + 1, point, t);
                this->Modified();
                return true;
            }
        }
    }
    return false;
}

bool mitk::Wire::IndexExists(int position, int t) const
{
    if ((unsigned int)t < m_WireSeries.size())
    {
        return m_WireSeries[t]->GetPoints()->IndexExists(position);
    }
    else
    {
        return false;
    }
}

bool mitk::Wire::GetSelectInfo(int position, int t) const
{
    if (this->IndexExists(position, t))
    {
        PointDataType pointData = { 0, false, PTUNDEFINED };
        m_WireSeries[t]->GetPointData(position, &pointData);
        return pointData.selected;
    }
    else
    {
        return false;
    }
}

void mitk::Wire::SetSelectInfo(int position, bool selected, int t)
{
    if (this->IndexExists(position, t))
    {
        // timeStep to ms
        TimePointType timeInMS = this->GetTimeGeometry()->TimeStepToTimePoint(t);

        // point
        Point3D point = this->GetPoint(position, t);

        std::unique_ptr<PointOperation> op;
        if (selected)
        {
            op.reset(new mitk::PointOperation(OpSELECTPOINT, timeInMS, point, position));
        }
        else
        {
            op.reset(new mitk::PointOperation(OpDESELECTPOINT, timeInMS, point, position));
        }

        this->ExecuteOperation(op.get());
    }
}

mitk::PointSpecificationType mitk::Wire::GetSpecificationTypeInfo(int position, int t) const
{
    if (this->IndexExists(position, t))
    {
        PointDataType pointData = { 0, false, PTUNDEFINED };
        m_WireSeries[t]->GetPointData(position, &pointData);
        return pointData.pointSpec;
    }
    else
    {
        return PTUNDEFINED;
    }
}

int mitk::Wire::GetNumberOfSelected(int t) const
{
    if ((unsigned int)t >= m_WireSeries.size())
    {
        return 0;
    }

    int numberOfSelected = 0;
    PointDataIterator it;
    for (it = m_WireSeries[t]->GetPointData()->Begin(); it != m_WireSeries[t]->GetPointData()->End(); it++)
    {
        if (it->Value().selected == true)
        {
            ++numberOfSelected;
        }
    }

    return numberOfSelected;
}

int mitk::Wire::SearchSelectedPoint(int t) const
{
    if ((unsigned int)t >= m_WireSeries.size())
    {
        return -1;
    }

    PointDataIterator it;
    for (it = m_WireSeries[t]->GetPointData()->Begin(); it != m_WireSeries[t]->GetPointData()->End(); it++)
    {
        if (it->Value().selected == true)
        {
            return it->Index();
        }
    }
    return -1;
}

void mitk::Wire::ExecuteOperation(Operation *operation)
{
    int timeStep = -1;

    mitkCheckOperationTypeMacro(PointOperation, operation, pointOp);

    if (pointOp)
    {
        timeStep = this->GetTimeGeometry()->TimePointToTimeStep(pointOp->GetTimeInMS());
    }

    if (timeStep < 0)
    {
        QF_ERROR << "Time step (" << timeStep << ") outside of Wire time bounds" << std::endl;
        return;
    }

    switch (operation->GetOperationType())
    {
    case OpNOTHING:
        break;

    case OpINSERT: // inserts the point at the given position and selects it.
    {
        int position = pointOp->GetIndex();

        PointType pt;
        pt.CastFrom(pointOp->GetPoint());

        if (timeStep >= (int)this->GetTimeSteps())
            this->Expand(timeStep + 1);

        // transfer from world to index coordinates
        mitk::BaseGeometry *geometry = this->GetGeometry(timeStep);
        if (geometry == nullptr)
        {
            QF_INFO << "GetGeometry returned NULL!\n";
            return;
        }
        geometry->WorldToIndex(pt, pt);

        m_WireSeries[timeStep]->GetPoints()->InsertElement(position, pt);

        PointDataType pointData = {
            static_cast<unsigned int>(pointOp->GetIndex()), pointOp->GetSelected(), pointOp->GetPointType() };

        m_WireSeries[timeStep]->GetPointData()->InsertElement(position, pointData);

        this->Modified();

        // boundingbox has to be computed
        m_CalculateBoundingBox = true;

        this->InvokeEvent(WireAddEvent());
        this->OnWireChange();
    }
    break;

    case OpMOVE: // moves the point given by index
    {
        PointType pt;
        pt.CastFrom(pointOp->GetPoint());

        // transfer from world to index coordinates
        this->GetGeometry(timeStep)->WorldToIndex(pt, pt);

        // Copy new point into container
        m_WireSeries[timeStep]->SetPoint(pointOp->GetIndex(), pt);

        // Insert a default point data object to keep the containers in sync
        // (if no point data object exists yet)
        PointDataType pointData;
        if (!m_WireSeries[timeStep]->GetPointData(pointOp->GetIndex(), &pointData))
        {
            m_WireSeries[timeStep]->SetPointData(pointOp->GetIndex(), pointData);
        }

        this->OnWireChange();

        this->Modified();

        // boundingbox has to be computed anyway
        m_CalculateBoundingBox = true;

        this->InvokeEvent(WireMoveEvent());
    }
    break;

    case OpREMOVE: // removes the point at given by position
    {
        m_WireSeries[timeStep]->GetPoints()->DeleteIndex((unsigned)pointOp->GetIndex());
        m_WireSeries[timeStep]->GetPointData()->DeleteIndex((unsigned)pointOp->GetIndex());

        this->OnWireChange();

        this->Modified();
        // boundingbox has to be computed anyway
        m_CalculateBoundingBox = true;

        this->InvokeEvent(WireRemoveEvent());
    }
    break;

    case OpSELECTPOINT: // select the given point
    {
        PointDataType pointData = { 0, false, PTUNDEFINED };
        m_WireSeries[timeStep]->GetPointData(pointOp->GetIndex(), &pointData);
        pointData.selected = true;
        m_WireSeries[timeStep]->SetPointData(pointOp->GetIndex(), pointData);
        this->Modified();
    }
    break;

    case OpDESELECTPOINT: // unselect the given point
    {
        PointDataType pointData = { 0, false, PTUNDEFINED };
        m_WireSeries[timeStep]->GetPointData(pointOp->GetIndex(), &pointData);
        pointData.selected = false;
        m_WireSeries[timeStep]->SetPointData(pointOp->GetIndex(), pointData);
        this->Modified();
    }
    break;

    case OpSETPOINTTYPE:
    {
        PointDataType pointData = { 0, false, PTUNDEFINED };
        m_WireSeries[timeStep]->GetPointData(pointOp->GetIndex(), &pointData);
        pointData.pointSpec = pointOp->GetPointType();
        m_WireSeries[timeStep]->SetPointData(pointOp->GetIndex(), pointData);
        this->Modified();
    }
    break;

    case OpMOVEPOINTUP: // swap content of point with ID pointOp->GetIndex() with the point preceding it in the
                        // container // move point position within the pointset
    {
        PointIdentifier currentID = pointOp->GetIndex();
        /* search for point with this id and point that precedes this one in the data container */
        PointsContainer::STLContainerType points = m_WireSeries[timeStep]->GetPoints()->CastToSTLContainer();
        auto it = points.find(currentID);
        if (it == points.end()) // ID not found
            break;
        if (it == points.begin()) // we are at the first element, there is no previous element
            break;

        /* get and cache current point & pointdata and previous point & pointdata */
        --it;
        PointIdentifier prevID = it->first;
        if (this->SwapPointContents(prevID, currentID, timeStep) == true)
            this->Modified();
    }
    break;
    case OpMOVEPOINTDOWN: // move point position within the pointset
    {
        PointIdentifier currentID = pointOp->GetIndex();
        /* search for point with this id and point that succeeds this one in the data container */
        PointsContainer::STLContainerType points = m_WireSeries[timeStep]->GetPoints()->CastToSTLContainer();
        auto it = points.find(currentID);
        if (it == points.end()) // ID not found
            break;
        ++it;
        if (it == points.end()) // ID is already the last element, there is no succeeding element
            break;

        /* get and cache current point & pointdata and previous point & pointdata */
        PointIdentifier nextID = it->first;
        if (this->SwapPointContents(nextID, currentID, timeStep) == true)
            this->Modified();
    }
    break;

    default:
        itkWarningMacro("mitkWire could not understrand the operation. Please check!");
        break;
    }

    // to tell the mappers, that the data is modified and has to be updated
    // only call modified if anything is done, so call in cases
    // this->Modified();

    mitk::OperationEndEvent endevent(operation);
    ((const itk::Object *)this)->InvokeEvent(endevent);

    //*todo has to be done here, cause of update-pipeline not working yet
    // As discussed lately, don't mess with the rendering from inside data structures
    // mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::Wire::UpdateOutputInformation()
{
    if (this->GetSource())
    {
        this->GetSource()->UpdateOutputInformation();
    }

    //
    // first make sure, that the associated time sliced geometry has
    // the same number of geometry 3d's as Wires are present
    //
    TimeGeometry *timeGeometry = GetTimeGeometry();
    if (timeGeometry->CountTimeSteps() != m_WireSeries.size())
    {
        itkExceptionMacro(<< "timeGeometry->CountTimeSteps() != m_WireSeries.size() -- use Initialize(timeSteps) with "
            "correct number of timeSteps!");
    }

    // This is needed to detect zero objects
    mitk::ScalarType nullpoint[] = { 0, 0, 0, 0, 0, 0 };
    BoundingBox::BoundsArrayType itkBoundsNull(nullpoint);

    //
    // Iterate over the Wires and update the Geometry
    // information of each of the items.
    //
    if (m_CalculateBoundingBox)
    {
        BoundingBox::BoundsArrayType bounds;
        for (unsigned int i = 0; i < ropeSimulation->numOfMasses; ++i)
        {
            bounds[0] = ropeSimulation->getMass(i)->pos.x < bounds[0] ? ropeSimulation->getMass(i)->pos.x : bounds[0];
            bounds[1] = ropeSimulation->getMass(i)->pos.x > bounds[1] ? ropeSimulation->getMass(i)->pos.x : bounds[1];
            bounds[2] = ropeSimulation->getMass(i)->pos.y < bounds[2] ? ropeSimulation->getMass(i)->pos.y : bounds[2];
            bounds[3] = ropeSimulation->getMass(i)->pos.y > bounds[3] ? ropeSimulation->getMass(i)->pos.y : bounds[3];
            bounds[4] = ropeSimulation->getMass(i)->pos.z < bounds[4] ? ropeSimulation->getMass(i)->pos.z : bounds[4];
            bounds[5] = ropeSimulation->getMass(i)->pos.z > bounds[5] ? ropeSimulation->getMass(i)->pos.z : bounds[5];

        }
        this->GetGeometry()->SetBounds(bounds);
        m_CalculateBoundingBox = false;
    }
    this->GetTimeGeometry()->Update();
}

void mitk::Wire::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::Wire::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    return false;
}

bool mitk::Wire::VerifyRequestedRegion()
{
    return true;
}

void mitk::Wire::SetRequestedRegion(const DataObject *)
{
}

void mitk::Wire::PrintSelf(std::ostream &os, itk::Indent indent) const
{
    Superclass::PrintSelf(os, indent);

    os << indent << "Number timesteps: " << m_WireSeries.size() << "\n";
    unsigned int i = 0;
    for (auto it = m_WireSeries.begin(); it != m_WireSeries.end(); ++it)
    {
        os << indent << "Timestep " << i++ << ": \n";
        MeshType::Pointer ps = *it;
        itk::Indent nextIndent = indent.GetNextIndent();
        ps->Print(os, nextIndent);
        MeshType::PointsContainer *points = ps->GetPoints();
        MeshType::PointDataContainer *datas = ps->GetPointData();
        MeshType::PointDataContainer::Iterator dataIterator = datas->Begin();
        for (MeshType::PointsContainer::Iterator pointIterator = points->Begin(); pointIterator != points->End();
            ++pointIterator, ++dataIterator)
        {
            os << nextIndent << "Point " << pointIterator->Index() << ": [";
            os << pointIterator->Value().GetElement(0);
            for (unsigned int i = 1; i < PointType::GetPointDimension(); ++i)
            {
                os << ", " << pointIterator->Value().GetElement(i);
            }
            os << "]";
            os << ", selected: " << dataIterator->Value().selected << ", point spec: " << dataIterator->Value().pointSpec
                << "\n";
        }
    }
}

bool mitk::Wire::SwapPointContents(PointIdentifier id1, PointIdentifier id2, int timeStep)
{
    /* search and cache contents */
    PointType p1;
    if (m_WireSeries[timeStep]->GetPoint(id1, &p1) == false)
        return false;
    PointDataType data1;
    if (m_WireSeries[timeStep]->GetPointData(id1, &data1) == false)
        return false;
    PointType p2;
    if (m_WireSeries[timeStep]->GetPoint(id2, &p2) == false)
        return false;
    PointDataType data2;
    if (m_WireSeries[timeStep]->GetPointData(id2, &data2) == false)
        return false;
    /* now swap contents */
    m_WireSeries[timeStep]->SetPoint(id1, p2);
    m_WireSeries[timeStep]->SetPointData(id1, data2);
    m_WireSeries[timeStep]->SetPoint(id2, p1);
    m_WireSeries[timeStep]->SetPointData(id2, data1);
    return true;
}

bool mitk::Wire::PointDataType::operator==(const mitk::Wire::PointDataType &other) const
{
    return id == other.id && selected == other.selected && pointSpec == other.pointSpec;
}

bool mitk::Equal(const mitk::Wire *leftHandSide,
    const mitk::Wire *rightHandSide,
    mitk::ScalarType eps,
    bool verbose,
    bool checkGeometry)
{
    if ((leftHandSide == nullptr) || (rightHandSide == nullptr))
    {
        QF_ERROR << "mitk::Equal( const mitk::Wire* leftHandSide, const mitk::Wire* rightHandSide, "
            "mitk::ScalarType eps, bool verbose ) does not work with NULL pointer input.";
        return false;
    }
    return Equal(*leftHandSide, *rightHandSide, eps, verbose, checkGeometry);
}

bool mitk::Equal(const mitk::Wire &leftHandSide,
    const mitk::Wire &rightHandSide,
    mitk::ScalarType eps,
    bool verbose,
    bool checkGeometry)
{
    bool result = true;

    // If comparing point sets from file, you must not compare the geometries, as they are not saved. In other cases, you
    // do need to check them.
    if (checkGeometry)
    {
        if (!mitk::Equal(*leftHandSide.GetGeometry(), *rightHandSide.GetGeometry(), eps, verbose))
        {
            if (verbose)
                QF_INFO << "[( Wire )] Geometries differ.";
            result = false;
        }
    }

    if (leftHandSide.GetSize() != rightHandSide.GetSize())
    {
        if (verbose)
            QF_INFO << "[( Wire )] Number of points differ.";
        result = false;
    }
    else
    {
        // if the size is equal, we compare the point values
        mitk::Point3D pointLeftHandSide;
        mitk::Point3D pointRightHandSide;

        int numberOfIncorrectPoints = 0;

        // Iterate over both pointsets in order to compare all points pair-wise
        mitk::Wire::PointsConstIterator end = leftHandSide.End();
        for (mitk::Wire::PointsConstIterator pointSetIteratorLeft = leftHandSide.Begin(),
            pointSetIteratorRight = rightHandSide.Begin();
            pointSetIteratorLeft != end;
            ++pointSetIteratorLeft, ++pointSetIteratorRight) // iterate simultaneously over both sets
        {
            pointLeftHandSide = pointSetIteratorLeft.Value();
            pointRightHandSide = pointSetIteratorRight.Value();
            if (!mitk::Equal(pointLeftHandSide, pointRightHandSide, eps, verbose))
            {
                if (verbose)
                    QF_INFO << "[( Wire )] Point values are different.";
                result = false;
                numberOfIncorrectPoints++;
            }
        }

        if ((numberOfIncorrectPoints > 0) && verbose)
        {
            QF_INFO << numberOfIncorrectPoints << " of a total of " << leftHandSide.GetSize() << " points are different.";
        }
    }
    return result;
}
