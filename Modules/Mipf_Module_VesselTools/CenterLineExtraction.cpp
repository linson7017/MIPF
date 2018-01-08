#include "CenterLineExtraction.h"

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
#include "vtkMath.h"
#include "vtkImageData.h"             
#include "vtkMarchingCubes.h"

//vmkt
#include "vmtk/vtkvmtkPolyDataCenterlines.h"
#include "vmtk/vtkvmtkPolyDataNetworkExtraction.h"
#include "vmtk/vtkvmtkCapPolyData.h"
#include "vmtk/vtkvmtkPolyBall.h"

#include "vmtk/vtkvmtkCenterlineBranchExtractor.h"
#include "vmtk/vtkvmtkPolyDataCenterlines.h"
#include "vmtk/vtkvmtkPolyBallModeller.h"
#include "vmtk/vtkvmtkCapPolyData.h"


CenterLineExtraction::CenterLineExtraction()
{
}


CenterLineExtraction::~CenterLineExtraction()
{
}

int CenterLineExtraction::FoundMinimumIndex(std::vector<double>& v)
{
    double minValue = DBL_MAX;
    double minIndex = 0;
    int i = 0;
    for (std::vector<double>::iterator it = v.begin(); it != v.end(); it++)
    {
        if (*it < minValue)
        {
            minValue = *it;
            minIndex = i;
        }
        i++;
    }
    return minIndex;
}

template <typename T>
void RemoveIndex(std::vector<T>& v, int index)
{
    if (v.size() < 2)
    {
        return;
    }
    int i = 0;
    for (std::vector<T>::iterator it = v.begin(); it != v.end(); it++)
    {
        if (i == index)
        {
            v.erase(it);
        }
        i++;
    }
}

void CenterLineExtraction::ReconstructTubularSurfaceByCenterLine(vtkPolyData* pCenterLine, vtkPolyData* pOutputData)
{
    if (!pCenterLine||!pOutputData)
    {
        return;
    }

    int ModelDim[3] = { 64,64,64 };
    auto modeller = vtkSmartPointer<vtkvmtkPolyBallModeller>::New();

    modeller->SetInputData(pCenterLine);
    modeller->SetRadiusArrayName("Radius");
    modeller->UsePolyBallLineOn();
    modeller->SetSampleDimensions(ModelDim);
    modeller->SetNegateFunction(0);
    modeller->Update();

    auto InMarching = vtkSmartPointer<vtkImageData>::New();
    InMarching->DeepCopy(modeller->GetOutput());
    InMarching->Modified();

    auto marchingCubes = vtkSmartPointer<vtkMarchingCubes>::New();
    marchingCubes->SetInputData(InMarching);
    marchingCubes->SetValue(0, 1);
    marchingCubes->Update();

    pOutputData->DeepCopy(marchingCubes->GetOutput());
}


void CenterLineExtraction::ExtractCenterLineNetwork(vtkPolyData* pInput, double* vStartPoint, vtkPolyData* pOutputNetwork, vtkPoints* pOutputEndpoints, vtkPolyData* pOutputVoronoi)
{
    auto preparedModel = vtkSmartPointer<vtkPolyData>::New();
    auto model = vtkSmartPointer<vtkPolyData>::New();
    auto voronoi = vtkSmartPointer<vtkPolyData>::New();

    PrepareModel(pInput, preparedModel);
    DecimateSurface(preparedModel, model);
    OpenSurfaceAtPoint(model, model, vStartPoint);
    ExtractNetwork(model, pOutputNetwork);

    auto clippedSurface = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> endpoints = vtkSmartPointer<vtkPoints>::New();
    ClipSurfaceAtEndPoints(pOutputNetwork, pInput, clippedSurface, endpoints.Get());
    if (pOutputEndpoints)
    {
        pOutputEndpoints->DeepCopy(endpoints);
    }

    return;


    double sourcePoint[3] = { 0, 0, 0 };

    std::vector<double> distancesToSeed;
    std::vector<double*> targetPoints;


    for (int i = 0; i < endpoints->GetNumberOfPoints(); i++)
    {
        auto currentPoint = endpoints->GetPoint(i);
        auto currentDistanceToSeed = sqrt(pow((currentPoint[0] - vStartPoint[0]), 2) +
            pow((currentPoint[1] - vStartPoint[1]), 2) +
            pow((currentPoint[2] - vStartPoint[2]), 2));

        targetPoints.push_back(currentPoint);
        distancesToSeed.push_back(currentDistanceToSeed);
    }

    int holePointIndex = FoundMinimumIndex(distancesToSeed);
    RemoveIndex<double>(distancesToSeed, holePointIndex);
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
    ComputeCenterlines(preparedModel, sourceIdList, targetIdList, pOutputNetwork, voronoi);

    std::cout << "Points Number: " << pOutputNetwork->GetNumberOfPoints();
    if (pOutputVoronoi)
    {
        pOutputVoronoi->DeepCopy(voronoi);
    }
}

void CenterLineExtraction::PrepareModel(vtkPolyData* polyData, vtkPolyData* outputPolyData)
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

void CenterLineExtraction::DecimateSurface(vtkPolyData* polyData, vtkPolyData* outputPolyData)
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

void CenterLineExtraction::OpenSurfaceAtPoint(vtkPolyData* polyData, vtkPolyData* outputPolyData, double* seed)
{
    float someradius = 1.0;

    vtkSmartPointer<vtkPointLocator> pointLocator = vtkSmartPointer<vtkPointLocator>::New();
    pointLocator->SetDataSet(polyData);
    pointLocator->BuildLocator();

    if (seed)
    {
        vtkIdType id = pointLocator->FindClosestPoint(seed);
        seed = polyData->GetPoint(id);
    }
    else
    {
        seed = polyData->GetPoint(0);
    }
    

    vtkSmartPointer<vtkSphere> sphere = vtkSmartPointer<vtkSphere>::New();
    sphere->SetCenter(seed[0], seed[1], seed[2]);
    sphere->SetRadius(someradius);

    vtkSmartPointer<vtkClipPolyData> clip = vtkSmartPointer<vtkClipPolyData>::New();
    clip->SetInputData(polyData);
    clip->SetClipFunction(sphere);
    clip->Update();

    outputPolyData->DeepCopy(clip->GetOutput());
}

void CenterLineExtraction::ExtractNetwork(vtkPolyData* polyData, vtkPolyData* outputPolyData)
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

void CenterLineExtraction::ClipSurfaceAtEndPoints(vtkPolyData* networkPolyData, vtkPolyData* surfacePolyData, vtkPolyData* outputPolyData, vtkPoints* endPoints)
{
    vtkSmartPointer<vtkCleanPolyData> cleaner = vtkSmartPointer < vtkCleanPolyData>::New();
    cleaner->SetInputData(networkPolyData);
    cleaner->Update();
    vtkPolyData* network = cleaner->GetOutput();
    network->BuildCells();
    network->BuildLinks(0);
    vtkSmartPointer<vtkIdList> endpointIds = vtkSmartPointer<vtkIdList>::New();

    vtkDataArray* radiusArray = network->GetPointData()->GetArray("Radius");

    vtkSmartPointer<vtkPolyData> endpoints = vtkSmartPointer<vtkPolyData>::New();
    vtkPoints* endpointsPoints = endPoints;
    vtkSmartPointer<vtkDoubleArray> endpointsRadius = vtkSmartPointer<vtkDoubleArray>::New();
    endpointsRadius->SetName("Radius");
    endpoints->SetPoints(endpointsPoints);
    endpoints->GetPointData()->AddArray(endpointsRadius);

    double radiusFactor = 1.2;
    double minRadius = 0.01;

    for (int i = 0; i < network->GetNumberOfCells(); i++)
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
                radius = vtkMath::Max(radius, minRadius);
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
                radius = vtkMath::Max(radius, minRadius);
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

}

void CenterLineExtraction::ComputeCenterlines(vtkPolyData* polyData, vtkIdList* inletSeedIds, vtkIdList* outletSeedIds,
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
