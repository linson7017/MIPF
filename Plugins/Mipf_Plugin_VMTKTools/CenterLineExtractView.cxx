#include "CenterLineExtractView.h"
#include "iqf_main.h"
#include "Res/R.h"


#include "QmitkPointListWidget.h"

//vtk
#include "vtkCleanPolyData.h"
#include "vtkTriangleFilter.h"
#include "vtkLinearSubdivisionFilter.h"
#include "vtkWindowedSincPolyDataFilter.h"
#include "vtkPolyDataNormals.h"
#include "vtkDecimatePro.h"
#include "vtkPointLocator.h"
#include "vtkSphere.h"
#include "vtkClipPolyData.h"
#include "vtkPointData.h"
#include "vtkDoubleArray.h"
#include "vtkCell.h"
#include "vtkPolyDataConnectivityFilter.h"
#include "vtkSphereSource.h"

//vmkt
#include "vmtk/vtkvmtkPolyDataCenterlines.h"
#include "vmtk/vtkvmtkPolyDataNetworkExtraction.h"
#include "vmtk/vtkvmtkCapPolyData.h"
#include "vmtk/vtkvmtkPolyBall.h"

#include "MitkStd/IQF_MitkPointList.h"


CenterLineExtractView::CenterLineExtractView() :MitkPluginView()
{

}

void CenterLineExtractView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "") == 0)
    {
        //do what you want for the message
    }
}

void CenterLineExtractView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    auto surfaceType = mitk::TNodePredicateDataType<mitk::Surface>::New();
    m_ui.DataSelector->SetPredicate(surfaceType.GetPointer());

    //seed point 
    m_pPointSetNode = mitk::DataNode::New();
    m_pPointSet = mitk::PointSet::New();
    m_pPointSetNode->SetData(m_pPointSet);
    m_pPointSetNode->SetName("seed points for tracking");
    m_pPointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true));
    m_pPointSetNode->SetProperty("layer", mitk::IntProperty::New(1024));
    GetDataStorage()->Add(m_pPointSetNode);
  
    m_ui.SeedWidget->SetPointSetNode(m_pPointSetNode);
    m_ui.SeedWidget->SetMultiWidget(m_pMitkRenderWindow->GetMitkStdMultiWidget());

    //end point
    IQF_PointListFactory* pFactory = (IQF_PointListFactory*)m_pMain->GetInterfacePtr(QF_MitkStd_PointListFactory);
    if (!pFactory)  return;
    m_pEndPointList = pFactory->CreatePointList();
    m_pEndPointList->Initialize();
    m_pEndPointSetNode = mitk::DataNode::New();
    m_pEndPointList->CreateNewPointSetNode(m_pEndPointSetNode);

    m_ui.EndPointList->SetMultiWidget(m_pMitkRenderWindow->GetMitkStdMultiWidget());
    dynamic_cast<QmitkPointListModel *>(m_ui.EndPointList->model())->SetPointSetNode(m_pEndPointSetNode);

    connect(m_ui.ApplyBtn, SIGNAL(clicked()), this, SLOT(Extract()));
    connect(m_ui.SelectEndPointBtn, SIGNAL(clicked(bool)), this, SLOT(SelectEndPoint(bool)));
    
}

void CenterLineExtractView::ConvertVTKPointsToMitkPointSet(vtkPoints* vps, mitk::PointSet* mps)
{
    for (int i=0;i<vps->GetNumberOfPoints();i++)
    {
        mitk::Point3D mp;
        auto currentPoint = vps->GetPoint(i);
        mp.SetElement(0, currentPoint[0]);
        mp.SetElement(1, currentPoint[1]);
        mp.SetElement(2, currentPoint[2]);
        mps->InsertPoint(i,mp);
    }
}

int CenterLineExtractView::FoundMinimumIndex(std::vector<double>& v)
{
    double minValue = DBL_MAX;
    double minIndex = 0;
    int i = 0;
    for (std::vector<double>::iterator it = v.begin(); it!= v.end(); it++)
    {
           if (*it<minValue)
           {
               minValue = *it;
               minIndex = i;
           }
           i++;
    }
    std::cout << "Minimum Distance:" << minValue << std::endl;
    return minIndex;
}

template <typename T>
void RemoveIndex(std::vector<T>& v, int index)
{
    if (v.size()<2)
    {
        return;
    }
    int i = 0;
    for (std::vector<T>::iterator it = v.begin(); it != v.end(); it++)
    {
        if (i==index)
        {
            v.erase(it);
        }
        i++;
    }
}

void CenterLineExtractView::Extract()
{
    vtkPolyData* polyData = dynamic_cast<mitk::Surface*>(m_ui.DataSelector->GetSelectedNode()->GetData())->GetVtkPolyData();

    auto preparedModel = vtkSmartPointer<vtkPolyData>::New();
    auto model = vtkSmartPointer<vtkPolyData>::New();
    auto network = vtkSmartPointer<vtkPolyData>::New();
    auto voronoi = vtkSmartPointer<vtkPolyData>::New();

    double  currentCoordinatesRAS[3] = {0.0,0.0,0.0};
    currentCoordinatesRAS[0] = m_pPointSet->GetPoint(0).GetElement(0);
    currentCoordinatesRAS[1] = m_pPointSet->GetPoint(0).GetElement(1);
    currentCoordinatesRAS[2] = m_pPointSet->GetPoint(0).GetElement(2);

    PrepareModel(polyData, preparedModel);
    DecimateSurface(preparedModel, model);
    OpenSurfaceAtPoint(model, model, currentCoordinatesRAS);
    ExtractNetwork(model, network);

    auto temp = vtkSmartPointer<vtkPolyData>::New();
    temp->DeepCopy(network);
    mitk::DataNode::Pointer tempDataNode = mitk::DataNode::New();
    mitk::Surface::Pointer tempSurface = mitk::Surface::New();
    tempSurface->SetVtkPolyData(temp);
    tempDataNode->SetData(tempSurface);
    tempDataNode->SetName("Temp");
    tempDataNode->SetColor(0.0, 1.0, 0.0);
    GetDataStorage()->Add(tempDataNode, m_ui.DataSelector->GetSelectedNode());

    auto clippedSurface = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> endpoints = ClipSurfaceAtEndPoints(network,polyData,clippedSurface);
    ConvertVTKPointsToMitkPointSet(endpoints, m_pEndPointList->GetPointSet());
    m_pEndPointSetNode->Modified();
    //m_pEndPointSet->;


    double sourcePoint[3] = { 0, 0, 0 };

    std::vector<double> distancesToSeed;
    std::vector<double*> targetPoints;
    

    for (int i=0;i<endpoints->GetNumberOfPoints();i++)
    {
        auto currentPoint = endpoints->GetPoint(i);
        auto currentDistanceToSeed = sqrt(pow((currentPoint[0] - currentCoordinatesRAS[0]), 2) +
            pow((currentPoint[1] - currentCoordinatesRAS[1]), 2) +
            pow((currentPoint[2] - currentCoordinatesRAS[2]), 2));

        targetPoints.push_back(currentPoint);
        distancesToSeed.push_back(currentDistanceToSeed);
    }

    int holePointIndex = FoundMinimumIndex(distancesToSeed);
    RemoveIndex<double>(distancesToSeed,holePointIndex);
    RemoveIndex<double*>(targetPoints, holePointIndex);

    int sourcePointIndex = FoundMinimumIndex(distancesToSeed);
    sourcePoint[0] = targetPoints[sourcePointIndex][0];
    sourcePoint[1] = targetPoints[sourcePointIndex][1];
    sourcePoint[2] = targetPoints[sourcePointIndex][2];
    RemoveIndex<double>(distancesToSeed, sourcePointIndex);
    RemoveIndex<double*>(targetPoints, sourcePointIndex);

    auto sourceIdList = vtkSmartPointer<vtkIdList>::New();
    auto targetIdList = vtkSmartPointer<vtkIdList>::New();

    auto pointLocator = vtkSmartPointer<vtkPointLocator>::New();
    pointLocator->SetDataSet(preparedModel);
    pointLocator->BuildLocator();

    vtkIdType sourceId = pointLocator->FindClosestPoint(sourcePoint);
    sourceIdList->InsertNextId(sourceId);

    for (int p = 0; p < targetPoints.size(); p++)
    {
        vtkIdType id = pointLocator->FindClosestPoint(targetPoints.at(p));
        targetIdList->InsertNextId(id);
    }
        
    ComputeCenterlines(preparedModel, sourceIdList, targetIdList, network, voronoi);

    mitk::DataNode::Pointer networkDataNode = mitk::DataNode::New();
    mitk::Surface::Pointer networkSurface = mitk::Surface::New();
    networkSurface->SetVtkPolyData(network);
    networkDataNode->SetData(networkSurface);
    networkDataNode->SetName("Center Lines");
    networkDataNode->SetColor(0.0, 1.0, 0.0);
    GetDataStorage()->Add(networkDataNode, m_ui.DataSelector->GetSelectedNode());


    /*mitk::DataNode::Pointer voronoiDataNode = mitk::DataNode::New();
    mitk::Surface::Pointer voronoiSurface = mitk::Surface::New();
    voronoiSurface->SetVtkPolyData(voronoi);
    voronoiDataNode->SetData(voronoiSurface);
    voronoiDataNode->SetName("Voronoi");
    voronoiDataNode->SetColor(0.0, 1.0, 0.0);
    GetDataStorage()->Add(voronoiDataNode, m_ui.DataSelector->GetSelectedNode());*/

    RequestRenderWindowUpdate();
}

void CenterLineExtractView::PrepareModel( vtkPolyData* polyData, vtkPolyData* outputPolyData)
{
    float capDisplacement = 0.0;

    vtkSmartPointer<vtkCleanPolyData>  surfaceCleaner = vtkSmartPointer<vtkCleanPolyData>::New();
    surfaceCleaner->SetInputData(polyData);
    surfaceCleaner->Update();

    vtkSmartPointer<vtkTriangleFilter> surfaceTriangulator = vtkSmartPointer<vtkTriangleFilter>::New();
    surfaceTriangulator->SetInputData(surfaceCleaner->GetOutput());
    surfaceTriangulator->PassLinesOff();
    surfaceTriangulator->PassVertsOff();
    surfaceTriangulator->Update();

    vtkSmartPointer<vtkLinearSubdivisionFilter> subdiv = vtkSmartPointer<vtkLinearSubdivisionFilter>::New();
    subdiv->SetInputData(surfaceTriangulator->GetOutput());
    subdiv->SetNumberOfSubdivisions(1);
    subdiv->Update();

    vtkSmartPointer<vtkWindowedSincPolyDataFilter> smooth = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
    smooth->SetInputData(subdiv->GetOutput());
    smooth->SetNumberOfIterations(20);
    smooth->SetPassBand(0.1);
    smooth->SetBoundarySmoothing(1);
    smooth->Update();

    vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
    normals->SetInputData(smooth->GetOutput());
    normals->SetAutoOrientNormals(1);
    normals->SetFlipNormals(0);
    normals->SetConsistency(1);
    normals->SplittingOff();
    normals->Update();

    vtkSmartPointer<vtkvmtkCapPolyData> surfaceCapper = vtkSmartPointer<vtkvmtkCapPolyData>::New();
    surfaceCapper->SetInputData(normals->GetOutput());
    surfaceCapper->SetDisplacement(capDisplacement);
    surfaceCapper->SetInPlaneDisplacement(capDisplacement);
    surfaceCapper->Update();

    outputPolyData->DeepCopy(surfaceCapper->GetOutput());
}

void CenterLineExtractView::DecimateSurface(vtkPolyData* polyData, vtkPolyData* outputPolyData)
{
    vtkSmartPointer<vtkDecimatePro> decimationFilter = vtkSmartPointer<vtkDecimatePro>::New();
    decimationFilter->SetInputData(polyData);
    decimationFilter->SetTargetReduction(0.99);
    decimationFilter->SetBoundaryVertexDeletion(0);
    decimationFilter->PreserveTopologyOn();
    decimationFilter->Update();

    vtkSmartPointer<vtkCleanPolyData> cleaner = vtkSmartPointer<vtkCleanPolyData>::New();
    cleaner->SetInputData(decimationFilter->GetOutput());
    cleaner->Update();

    vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
    triangleFilter->SetInputData(cleaner->GetOutput());
    triangleFilter->Update();

    outputPolyData->DeepCopy(triangleFilter->GetOutput());

}

void CenterLineExtractView::OpenSurfaceAtPoint(vtkPolyData* polyData, vtkPolyData* outputPolyData, double* seed)
{
    float someradius = 1.0;

    vtkSmartPointer<vtkPointLocator> pointLocator = vtkSmartPointer<vtkPointLocator>::New();
    pointLocator->SetDataSet(polyData);
    pointLocator->BuildLocator();

    vtkIdType id = pointLocator->FindClosestPoint(seed);

    seed = polyData->GetPoint(id);

    vtkSmartPointer<vtkSphere> sphere = vtkSmartPointer<vtkSphere>::New();
    sphere->SetCenter(seed[0], seed[1], seed[2]);
    sphere->SetRadius(someradius);

    vtkSmartPointer<vtkClipPolyData> clip = vtkSmartPointer<vtkClipPolyData>::New();
    clip->SetInputData(polyData);
    clip->SetClipFunction(sphere);
    clip->Update();

    outputPolyData->DeepCopy(clip->GetOutput());
}

void CenterLineExtractView::ExtractNetwork( vtkPolyData* polyData, vtkPolyData* outputPolyData)
{
    std::string radiusArrayName = "Radius";
    std::string topologyArrayName = "Topology";
    std::string marksArrayName = "Marks";

    vtkSmartPointer<vtkvmtkPolyDataNetworkExtraction> networkExtraction =
        vtkSmartPointer < vtkvmtkPolyDataNetworkExtraction>::New();
    networkExtraction->SetInputData(polyData);
    networkExtraction->SetAdvancementRatio(1.05);
    networkExtraction->SetRadiusArrayName(radiusArrayName.c_str());
    networkExtraction->SetTopologyArrayName(topologyArrayName.c_str());
    networkExtraction->SetMarksArrayName(marksArrayName.c_str());
    networkExtraction->Update();

    outputPolyData->DeepCopy(networkExtraction->GetOutput());
}

vtkSmartPointer<vtkPoints> CenterLineExtractView::ClipSurfaceAtEndPoints(vtkPolyData* networkPolyData, vtkPolyData* surfacePolyData, vtkPolyData* outputPolyData)
{
    vtkSmartPointer<vtkCleanPolyData> cleaner = vtkSmartPointer < vtkCleanPolyData>::New();
    cleaner->SetInputData(networkPolyData);
    cleaner->Update();
    vtkPolyData* network = cleaner->GetOutput();
    network->BuildCells();
    network->BuildLinks(0);
    vtkSmartPointer<vtkIdList> endpointIds = vtkSmartPointer<vtkIdList>::New();

    vtkDataArray* radiusArray =  network->GetPointData()->GetArray("Radius");

    vtkSmartPointer<vtkPolyData> endpoints = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> endpointsPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkDoubleArray> endpointsRadius = vtkSmartPointer<vtkDoubleArray>::New();
    endpointsRadius->SetName("Radius");
    endpoints->SetPoints(endpointsPoints);
    endpoints->GetPointData()->AddArray(endpointsRadius);

    double radiusFactor = 1.2;
    double minRadius = 0.01;

    for (int i=0; i<network->GetNumberOfCells(); i++)
    {
        auto numberOfCellPoints = network->GetCell(i)->GetNumberOfPoints();
        auto pointId0 = network->GetCell(i)->GetPointId(0);
        auto pointId1 = network->GetCell(i)->GetPointId(numberOfCellPoints - 1);

        auto pointCells = vtkSmartPointer<vtkIdList>::New();
        network->GetPointCells(pointId0, pointCells);
        auto numberOfEndpoints = endpointIds->GetNumberOfIds();
        if (pointCells->GetNumberOfIds() == 1)
        {
            auto pointId = endpointIds->InsertUniqueId(pointId0);
            if (pointId == numberOfEndpoints)
            {
                auto point = network->GetPoint(pointId0);
                auto radius = radiusArray->GetVariantValue(pointId0).ToDouble();
                radius = max(radius, minRadius);
                endpointsPoints->InsertNextPoint(point);
                endpointsRadius->InsertNextValue(radiusFactor * radius);
            }
        }

        //auto pointCells = vtkSmartPointer<vtkIdList>::New();
        pointCells->Initialize();
        network->GetPointCells(pointId1, pointCells);
        numberOfEndpoints = endpointIds->GetNumberOfIds();
        if (pointCells->GetNumberOfIds() == 1)
        {
            auto pointId = endpointIds->InsertUniqueId(pointId1);
            if (pointId == numberOfEndpoints)
            {
                auto point = network->GetPoint(pointId1);
                auto radius = radiusArray->GetVariantValue(pointId0).ToDouble();
                radius = max(radius, minRadius);
                endpointsPoints->InsertNextPoint(point);
            }
        }
              
    }
    vtkSmartPointer<vtkvmtkPolyBall> polyBall = vtkSmartPointer<vtkvmtkPolyBall>::New();
    polyBall->SetInput(endpoints);
    polyBall->SetPolyBallRadiusArrayName("Radius");

    vtkSmartPointer<vtkClipPolyData> clipper = vtkSmartPointer<vtkClipPolyData>::New();
    clipper->SetInputData(surfacePolyData);
    clipper->SetClipFunction(polyBall);
    clipper->Update();

    vtkSmartPointer<vtkPolyDataConnectivityFilter> connectivityFilter = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
    connectivityFilter->SetInputData(clipper->GetOutput());
    connectivityFilter->ColorRegionsOff();
    connectivityFilter->SetExtractionModeToLargestRegion();
    connectivityFilter->Update();

    outputPolyData->DeepCopy(connectivityFilter->GetOutput());
    std::cout << "End Points Size:" << endpointsPoints->GetNumberOfPoints() << std::endl;
    return endpointsPoints;

}

void CenterLineExtractView::ComputeCenterlines(vtkPolyData* polyData, vtkIdList* inletSeedIds, vtkIdList* outletSeedIds,
    vtkPolyData* outPolyData, vtkPolyData* outPolyData2)
{
    int flipNormals = 0;
    auto radiusArrayName = "Radius";
    auto costFunction = "1/R";


    auto centerlineFilter = vtkSmartPointer<vtkvmtkPolyDataCenterlines>::New();
    centerlineFilter->SetInputData(polyData);
    centerlineFilter->SetSourceSeedIds(inletSeedIds);
    centerlineFilter->SetTargetSeedIds(outletSeedIds);
    centerlineFilter->SetRadiusArrayName(radiusArrayName);
    centerlineFilter->SetCostFunction(costFunction);
    centerlineFilter->SetFlipNormals(flipNormals);
    centerlineFilter->SetAppendEndPointsToCenterlines(0);
    centerlineFilter->SetSimplifyVoronoi(0);
    centerlineFilter->SetCenterlineResampling(0);
    centerlineFilter->SetResamplingStepLength(1.0);
    centerlineFilter->Update();


    outPolyData->DeepCopy(centerlineFilter->GetOutput());
    outPolyData2->DeepCopy(centerlineFilter->GetVoronoiDiagram());

}

void CenterLineExtractView::SelectEndPoint(bool bSelecting)
{
    m_pEndPointList->AddPoint(bSelecting);
}