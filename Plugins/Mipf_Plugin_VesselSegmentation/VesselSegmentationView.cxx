#include "VesselSegmentationView.h"
#include "iqf_main.h"
#include "Res/R.h"

 //VTK
#include "vtkPolyDataConnectivityFilter.h"
#include "vtkWindowedSincPolyDataFilter.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkImageGaussianSmooth.h"
#include "vtkLinearSubdivisionFilter.h"
#include "vtkImageMathematics.h"
#include "vtkPolyDataNormals.h"
#include "vtkImageThreshold.h"
#include "vtkImageShiftScale.h"
#include "vtkMarchingCubes.h"
#include "vtkReverseSense.h"
#include "vtkCleanPolyData.h"
#include "vtkTriangleFilter.h"
#include "vtkDecimatePro.h"
#include "vtkSplineFilter.h"
#include "vtkTransform.h"
#include "vtkStripper.h"
#include "vtkImageCast.h"

//ITK
#include <itkConnectedThresholdImageFilter.h>
#include <itkGeodesicActiveContourLevelSetImageFilter.h>

//VMTK
#include "vtkvmtkFastMarchingUpwindGradientImageFilter.h"
#include "vtkvmtkGeodesicActiveContourLevelSetImageFilter.h"
#include "vtkvmtkCenterlineBranchExtractor.h"
#include "vtkvmtkPolyDataCenterlines.h"
#include "vtkvmtkPolyBallModeller.h"
#include "vtkvmtkCapPolyData.h"

//common
#include "MitkSegmentation/IQF_MitkSurfaceTool.h"
#include "VesselTools/IQF_VesselSegmentationTool.h"
#include "Core/IQF_ObjectFactory.h"


VesselSegmentationView::VesselSegmentationView() :MitkPluginView()
{
    //m_pMain->Attach(this);
    m_SourcePointSetNode = NULL;
    m_SourcePointSet = NULL;
    m_SourcePointSetNode_re = NULL;
    m_SourcePointSet_re = NULL;

    m_TargetPointSetNode_re = NULL;
    m_TargetPointSet_re = NULL;
    newNode = NULL;
}

void VesselSegmentationView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "") == 0)
    {
        //do what you want for the message
    }
}

void VesselSegmentationView::CreateView()
{
    ui.setupUi(this);
    

    connect(ui.pbtnCreateSmoothSurface, SIGNAL(clicked()), this, SLOT(OnCreateSmoothSurface()));
    connect(ui.pbtnCreateVesselSurfaceFromMask, SIGNAL(clicked()), this, SLOT(OnCreateVesselSurfaceFromMask()));

    ui.cmbbxImageSelector->SetDataStorage(GetDataStorage());
    ui.cmbbxImageSelector->SetPredicate(mitk::NodePredicateDataType::New("Image"));

    ui.cmbbxSurfaceSelector->SetDataStorage(GetDataStorage());
    ui.cmbbxSurfaceSelector->SetPredicate(mitk::NodePredicateDataType::New("Surface"));


    ui.lstPoints->SetMultiWidget(m_pMitkRenderWindow->GetMitkStdMultiWidget());
    ui.lstPoints_3->SetMultiWidget(m_pMitkRenderWindow->GetMitkStdMultiWidget());
    ui.lstPoints_4->SetMultiWidget(m_pMitkRenderWindow->GetMitkStdMultiWidget());
    InitializePointListWidget();
}

void VesselSegmentationView::InitializePointListWidget()
{
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    mitk::DataNode* tempNode;
    if (m_SourcePointSet.IsNull())  m_SourcePointSet = mitk::PointSet::New();
    if (m_SourcePointSetNode.IsNull())
    {
        m_SourcePointSetNode = mitk::DataNode::New();
        m_SourcePointSetNode->SetData(m_SourcePointSet);
        m_SourcePointSetNode->SetName("seed points for Source");
        m_SourcePointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true));
        m_SourcePointSetNode->SetProperty("lable", mitk::StringProperty::New("Point"));
        m_SourcePointSetNode->SetProperty("layer", mitk::IntProperty::New(100));
    }
    tempNode = NULL;
    tempNode = GetDataStorage()->GetNamedNode("seed points for Source");
    if (tempNode == NULL)
    {
        GetDataStorage()->Add(m_SourcePointSetNode);
        ui.lstPoints->SetPointSetNode(m_SourcePointSetNode);
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (m_SourcePointSet_re.IsNull())  m_SourcePointSet_re = mitk::PointSet::New();
    if (m_SourcePointSetNode_re.IsNull())
    {
        m_SourcePointSetNode_re = mitk::DataNode::New();
        m_SourcePointSetNode_re->SetData(m_SourcePointSet_re);
        m_SourcePointSetNode_re->SetName("seed points for Source_re");
        m_SourcePointSetNode_re->SetProperty("helper object", mitk::BoolProperty::New(true));
        m_SourcePointSetNode_re->SetProperty("layer", mitk::IntProperty::New(100));
    }
    tempNode = NULL;
    tempNode = GetDataStorage()->GetNamedNode("seed points for Source_re");
    if (tempNode == NULL)
    {
        GetDataStorage()->Add(m_SourcePointSetNode_re);
        ui.lstPoints_3->SetPointSetNode(m_SourcePointSetNode_re);
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (m_TargetPointSet_re.IsNull())  m_TargetPointSet_re = mitk::PointSet::New();
    if (m_TargetPointSetNode_re.IsNull())
    {
        m_TargetPointSetNode_re = mitk::DataNode::New();
        m_TargetPointSetNode_re->SetData(m_TargetPointSet_re);
        m_TargetPointSetNode_re->SetName("seed points for Target_re");
        m_TargetPointSetNode_re->SetProperty("helper object", mitk::BoolProperty::New(true));
        m_TargetPointSetNode_re->SetProperty("layer", mitk::IntProperty::New(100));
    }
    tempNode = NULL;
    tempNode = GetDataStorage()->GetNamedNode("seed points for Target_re");
    if (tempNode == NULL)
    {
        GetDataStorage()->Add(m_TargetPointSetNode_re);
        ui.lstPoints_4->SetPointSetNode(m_TargetPointSetNode_re);
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
void VesselSegmentationView::OnCurrentSelectedDataNode(mitk::DataNode* node, int type)
{


    if (node != NULL)
    {
        //m_SelectedDataNode = node;
        m_SelectedDataNodeType = type;
    }
}

void VesselSegmentationView::OnCreateVesselSurfaceFromMask()
{
    

    mitk::DataNode::Pointer node = ui.cmbbxImageSelector->GetSelectedNode();

    mitk::Image* imageseed = (mitk::Image*)(node->GetData());

    vtkImageData* image = imageseed->GetVtkImageData();

    vtkIdList*  sourceSeedIds = vtkIdList::New();
    vtkIdList*  targetSeedIds = vtkIdList::New();

    mitk::BaseGeometry* imageGeometry = imageseed->GetGeometry();

    mitk::Image::IndexType seedIndex;

    for (mitk::PointSet::PointsConstIterator pointsIterator = m_SourcePointSet->GetPointSet()->GetPoints()->Begin();
        pointsIterator != m_SourcePointSet->GetPointSet()->GetPoints()->End();
        ++pointsIterator)
    {

        imageGeometry->WorldToIndex(pointsIterator.Value(), seedIndex);

        long* PtrSeed = (long*)seedIndex.GetIndex();

        int ASeed[3] = { (int)*PtrSeed,(int)*(PtrSeed + 1),(int)*(PtrSeed + 2) };
        sourceSeedIds->InsertNextId(image->ComputePointId(ASeed));
    }

    double Thmin, Thmax;
    Thmin = ui.spbxLowerThreshold->value();
    Thmax = ui.spbxUpperThreshold->value();

    IQF_ObjectFactory* pObjectFactory = (IQF_ObjectFactory*)m_pMain->GetInterfacePtr(QF_Core_ObjectFactory);
    if (pObjectFactory)
    {
        IQF_VesselSegmentationTool* pVesselSegTool = (IQF_VesselSegmentationTool*)pObjectFactory->CreateObject("VesselSegmentation");
        auto outputSurface = vtkSmartPointer<vtkPolyData>::New();
        pVesselSegTool->SegmentVessel(image, outputSurface.GetPointer(), Thmin, Thmax, sourceSeedIds, targetSeedIds);

        vtkPolyDataConnectivityFilter* connectivityFilter = vtkPolyDataConnectivityFilter::New();
        connectivityFilter->SetInputData(outputSurface);
        connectivityFilter->SetExtractionModeToLargestRegion();
        connectivityFilter->Update();
        outputSurface = connectivityFilter->GetOutput();
        outputSurface->Modified();

        mitk::Surface::Pointer Surbase3 = mitk::Surface::New();
        Surbase3->GetGeometry()->SetOrigin(imageseed->GetGeometry()->GetOrigin());
        Surbase3->SetVtkPolyData(outputSurface);
        Surbase3->Update();

        //convert surface to image
        IQF_MitkSurfaceTool* pSurfaceTool = (IQF_MitkSurfaceTool*)m_pMain->GetInterfacePtr(QF_MitkSurface_Tool);
        if (pSurfaceTool)
        {
            //add vessel image
            GetDataStorage()->Remove(GetDataStorage()->GetNamedNode(Object_ID_VesselSegmentationTool));
            mitk::DataNode::Pointer vesselImageNode = mitk::DataNode::New();
            mitk::Image::Pointer vesselImage = mitk::Image::New();
            vesselImage->Initialize(imageseed);
            pSurfaceTool->ConvertSurfaceToImage(Surbase3.GetPointer(), imageseed, vesselImage.GetPointer());
            vesselImageNode->SetData(vesselImage);
            vesselImageNode->SetColor(1.0, 0.0, 0.0);
            vesselImageNode->SetOpacity(0.5);
            vesselImageNode->SetName("Vessel Image");
            GetDataStorage()->Add(vesselImageNode);
        }

        GetDataStorage()->Remove(GetDataStorage()->GetNamedNode("Vessel"));
        mitk::DataNode::Pointer outnode3 = mitk::DataNode::New();
        outnode3->SetData(Surbase3);
        outnode3->SetProperty("name", mitk::StringProperty::New("Vessel"));
        outnode3->SetProperty("color", mitk::ColorProperty::New(1.0, 1.0, 1.0));
        outnode3->Update();

        GetDataStorage()->Add(outnode3);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void VesselSegmentationView::OnCreateSmoothSurface()
{
    

    mitk::DataNode::Pointer node = ui.cmbbxSurfaceSelector->GetSelectedNode();
    mitk::Surface* ImageSurface = dynamic_cast<mitk::Surface*>(node->GetData());

    //vtkCleanPolyData* surfaceCleaner = vtkCleanPolyData::New();
    //surfaceCleaner->SetInput(ImageSurface->GetVtkPolyData());
    //surfaceCleaner->Update();

    //vtkTriangleFilter* surfaceTriangulator = vtkTriangleFilter::New();
    //surfaceTriangulator->SetInput(surfaceCleaner->GetOutput());
    //surfaceTriangulator->PassLinesOff();
    //surfaceTriangulator->PassVertsOff();
    //surfaceTriangulator->Update();

    //vtkLinearSubdivisionFilter* subdiv = vtkLinearSubdivisionFilter::New();
    //subdiv->SetInput(surfaceTriangulator->GetOutput());
    //subdiv->SetNumberOfSubdivisions(1);
    //subdiv->Update();

    //vtkWindowedSincPolyDataFilter* smooth = vtkWindowedSincPolyDataFilter::New();
    //smooth->SetInput(subdiv->GetOutput());
    //smooth->SetNumberOfIterations(20);
    //smooth->SetPassBand(0.1);
    //smooth->SetBoundarySmoothing(1);
    //smooth->Update();

    //vtkPolyDataNormals* normals = vtkPolyDataNormals::New();
    //normals->SetInput(smooth->GetOutput());
    //normals->SetAutoOrientNormals(1);
    //normals->SetFlipNormals(0);
    //normals->SetConsistency(1);
    //normals->SplittingOff();
    //normals->Update();

    //vtkvmtkCapPolyData* surfaceCapper = vtkvmtkCapPolyData::New();
    //surfaceCapper->SetInput(normals->GetOutput());
    //surfaceCapper->SetDisplacement(0.0);
    //surfaceCapper->SetInPlaneDisplacement(0.0);
    //surfaceCapper->Update();

    //vtkPolyData* polyDataNew = vtkPolyData::New();
    //polyDataNew->DeepCopy(surfaceCapper->GetOutput());
    ////polyDataNew->DeepCopy(ImageSurface->GetVtkPolyData());
    //polyDataNew->Update();

    /////////////////////////////////////////
    vtkIdList*  sourceSeedIds = vtkIdList::New();
    vtkIdList*  targetSeedIds = vtkIdList::New();

    for (mitk::PointSet::PointsConstIterator pointsIterator = m_SourcePointSet_re->GetPointSet()->GetPoints()->Begin(); // really nice syntax to get an interator for all points
        pointsIterator != m_SourcePointSet_re->GetPointSet()->GetPoints()->End();
        ++pointsIterator)
    {
        mitk::Point3D seed = pointsIterator.Value();
        MITK_INFO << seed << std::endl;
        float* sed = (float*)seed.Begin();
        sourceSeedIds->InsertNextId(ImageSurface->GetVtkPolyData()->FindPoint((double)*sed, (double)*(sed + 1), (double)*(sed + 2)));
    }

    for (mitk::PointSet::PointsConstIterator pointsIterator2 = m_TargetPointSet_re->GetPointSet()->GetPoints()->Begin(); // really nice syntax to get an interator for all points
        pointsIterator2 != m_TargetPointSet_re->GetPointSet()->GetPoints()->End();
        ++pointsIterator2)
    {
        mitk::Point3D seed = pointsIterator2.Value();
        MITK_INFO << seed << std::endl;
        float* sed = (float*)seed.Begin();
        targetSeedIds->InsertNextId(ImageSurface->GetVtkPolyData()->FindPoint((double)*sed, (double)*(sed + 1), (double)*(sed + 2)));
    }


    vtkPolyData* PloyCenter = ComputeCenterLine(ImageSurface->GetVtkPolyData(), sourceSeedIds, targetSeedIds);

    vtkvmtkCenterlineBranchExtractor* BranchExtractor = vtkvmtkCenterlineBranchExtractor::New();
    BranchExtractor->SetInputData(PloyCenter);
    BranchExtractor->SetBlankingArrayName("Blanking");
    BranchExtractor->SetRadiusArrayName("MaximumInscribedSphereRadius");
    BranchExtractor->SetGroupIdsArrayName("GroupIds");
    BranchExtractor->SetCenterlineIdsArrayName("CenterlineIds");
    BranchExtractor->SetTractIdsArrayName("TractIds");
    BranchExtractor->Update();

    vtkPolyData* OutBranchPoly = vtkPolyData::New();
    OutBranchPoly->DeepCopy(BranchExtractor->GetOutput());
    OutBranchPoly->Modified();

    int ModelDim[3] = { 64,64,64 };
    //  double Bounds[6]={1.0,1.0,1.0,1.0,1.0,1.0};
    vtkvmtkPolyBallModeller* modeller = vtkvmtkPolyBallModeller::New();

    modeller->SetInputData(OutBranchPoly);
    modeller->SetRadiusArrayName("MaximumInscribedSphereRadius");
    modeller->UsePolyBallLineOn();
    modeller->SetSampleDimensions(ModelDim);
    //	modeller->SetModelBounds(Bounds);
    modeller->SetNegateFunction(0);
    modeller->Update();

    vtkImageData* InMarching = vtkImageData::New();
    InMarching->DeepCopy(modeller->GetOutput());
    InMarching->Modified();

    vtkMarchingCubes* marchingCubes = vtkMarchingCubes::New();
    marchingCubes->SetInputData(InMarching);
    marchingCubes->SetValue(0, 1);
    marchingCubes->Update();


    mitk::Surface::Pointer Surbasemodel = mitk::Surface::New();
    Surbasemodel->SetVtkPolyData(marchingCubes->GetOutput());
    Surbasemodel->GetGeometry()->SetOrigin(ImageSurface->GetGeometry()->GetOrigin());
    Surbasemodel->Update();


    mitk::DataNode::Pointer outnodemodel = mitk::DataNode::New();
    outnodemodel->SetData(Surbasemodel);
    outnodemodel->SetProperty("name", mitk::StringProperty::New("VesselSmooth"));
    outnodemodel->SetProperty("color", mitk::ColorProperty::New(1.0, 0.0, 0.0));
    outnodemodel->Update();

    GetDataStorage()->Add(outnodemodel);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    //surfaceCleaner->Delete();
    //surfaceTriangulator->Delete();
    //subdiv->Delete();
    //smooth->Delete();
    //normals->Delete();
    //surfaceCapper->Delete();
    //polyDataNew->Delete();
    PloyCenter->Delete();
    BranchExtractor->Delete();
    OutBranchPoly->Delete();
    modeller->Delete();
    InMarching->Delete();
    marchingCubes->Delete();
}

vtkImageData* VesselSegmentationView::ExecuteFM(vtkImageData* image, double lowerThreshold, double higherThreshold, vtkIdList*  sourceSeedIds, vtkIdList*targetSeedIds)
{
    double*scalarRange = image->GetScalarRange();
    int* imageDimensions = image->GetDimensions();
    int maxImageDimensions = *imageDimensions;
    for (int i = 0; i < 2; i++)
    {
        if (*imageDimensions < *(imageDimensions + 1))
            maxImageDimensions = *(imageDimensions + 1);

        imageDimensions++;
    }

    vtkImageThreshold* threshold = vtkImageThreshold::New();

    threshold->SetInputData(image);
    threshold->ThresholdBetween(lowerThreshold, higherThreshold);
    threshold->ReplaceInOff();
    threshold->ReplaceOutOn();
    threshold->SetOutValue(scalarRange[0] - scalarRange[1]);
    threshold->Update();

    vtkImageData* thresholdedImage = threshold->GetOutput();

    scalarRange = thresholdedImage->GetScalarRange();

    vtkImageShiftScale* shiftScale = vtkImageShiftScale::New();
    shiftScale->SetInputData(thresholdedImage);
    shiftScale->SetShift(-scalarRange[0]);
    shiftScale->SetScale(1 / (scalarRange[1] - scalarRange[0]));
    shiftScale->Update();

    vtkImageData* speedImage = shiftScale->GetOutput();

    vtkvmtkFastMarchingUpwindGradientImageFilter* fastMarching = vtkvmtkFastMarchingUpwindGradientImageFilter::New();
    fastMarching->SetInputData(speedImage);
    fastMarching->SetSeeds(sourceSeedIds);
    // fastMarching->GenerateGradientImageOff();
    fastMarching->GenerateGradientImageOn();
    fastMarching->SetTargetOffset(100.0);
    fastMarching->SetTargets(targetSeedIds);
    fastMarching->SetTargetReachedModeToNoTargets();
    fastMarching->Update();

    vtkImageMathematics* subtract = vtkImageMathematics::New();
    subtract->SetInputData(fastMarching->GetOutput());
    subtract->SetOperationToAddConstant();
    subtract->SetConstantC(-fastMarching->GetTargetValue());
    subtract->Update();

    vtkImageData*  outVolumeData = vtkImageData::New();
    outVolumeData->DeepCopy(subtract->GetOutput());
    outVolumeData->Modified();

    threshold->Delete();
    //thresholdedImage->Delete();
    shiftScale->Delete();
    //speedImage->Delete();
    fastMarching->Delete();
    subtract->Delete();

    return outVolumeData;
}

vtkPolyData* VesselSegmentationView::MarchingCubes(vtkImageData* image, double threshold)
{
    vtkMarchingCubes *marchingCubes = vtkMarchingCubes::New();
    marchingCubes->SetInputData(image);
    marchingCubes->SetValue(0, threshold);
    marchingCubes->ComputeScalarsOn();
    marchingCubes->ComputeGradientsOn();
    marchingCubes->ComputeNormalsOn();
    marchingCubes->Update();

    vtkReverseSense* reverser = vtkReverseSense::New();
    reverser->SetInputData(marchingCubes->GetOutput());
    reverser->ReverseNormalsOn();
    reverser->Update();

    vtkPolyData* correctedOutput = reverser->GetOutput();


    vtkTransform* transformIJKtoRAS = vtkTransform::New();
    //transformIJKtoRAS->SetMatrix(Matrix);

    vtkTransformPolyDataFilter* transformer = vtkTransformPolyDataFilter::New();
    transformer->SetInputData(correctedOutput);
    transformer->SetTransform(transformIJKtoRAS);
    transformer->Update();

    vtkPolyDataNormals* normals = vtkPolyDataNormals::New();
    normals->ComputePointNormalsOn();
    normals->SetInputData(transformer->GetOutput());
    //normals->SetInput(correctedOutput);
    normals->SetFeatureAngle(60);
    normals->SetSplitting(1);
    normals->Update();

    vtkStripper* stripper = vtkStripper::New();
    stripper->SetInputData(normals->GetOutput());
    stripper->Update();
    stripper->GetOutput()->Modified();


    vtkPolyData* result = vtkPolyData::New();
    result->DeepCopy(stripper->GetOutput());
    result->Modified();
    stripper->Delete();
    transformer->Delete();
    marchingCubes->Delete();
    reverser->Delete();
    transformIJKtoRAS->Delete();
    normals->Delete();
    return result;
}

vtkPolyData* VesselSegmentationView::ComputeCenterLine(vtkPolyData* CenterIndata, vtkIdList*  inletSeedIds, vtkIdList*  outletSeedIds)
{
    vtkvmtkPolyDataCenterlines*centerlineFilter = vtkvmtkPolyDataCenterlines::New();
    centerlineFilter->SetInputData(CenterIndata);
    centerlineFilter->SetSourceSeedIds(inletSeedIds);
    centerlineFilter->SetTargetSeedIds(outletSeedIds);
    centerlineFilter->SetRadiusArrayName("MaximumInscribedSphereRadius");
    centerlineFilter->SetCostFunction("1/R");
    centerlineFilter->SetFlipNormals(0);
    centerlineFilter->SetAppendEndPointsToCenterlines(0);
    centerlineFilter->SetSimplifyVoronoi(true);
    centerlineFilter->SetCenterlineResampling(0);
    centerlineFilter->SetResamplingStepLength(1.0);
    centerlineFilter->Update();

    vtkCleanPolyData* cleaner = vtkCleanPolyData::New();
    cleaner->SetInputData(centerlineFilter->GetOutput());
    cleaner->Update();

    double Length = (cleaner->GetOutput()->GetLength()) / 100.0;

    vtkSplineFilter* splineFilter = vtkSplineFilter::New();
    splineFilter->SetInputData(cleaner->GetOutput());
    splineFilter->SetSubdivideToLength();
    splineFilter->SetLength(Length);
    splineFilter->Update();


    vtkPolyData* polyOutDataNew = vtkPolyData::New();
    polyOutDataNew->DeepCopy(splineFilter->GetOutput());
    polyOutDataNew->Modified();

    return polyOutDataNew;
}