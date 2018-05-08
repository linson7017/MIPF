#include "VesselSegmentation.h"

//vtk
#include "vtkImageCast.h"
#include "vtkMath.h"
#include "vtkSmartPointer.h"
#include "vtkImageThreshold.h"
#include "vtkImageShiftScale.h"
#include "vtkImageMathematics.h"
#include "vtkImageGaussianSmooth.h"
#include "vtkReverseSense.h"
#include "vtkCleanPolyData.h"
#include "vtkTriangleFilter.h"
#include "vtkDecimatePro.h"
#include "vtkSplineFilter.h"
#include "vtkTransform.h"
#include "vtkPolyDataNormals.h"
#include "vtkImageThreshold.h"
#include "vtkMarchingCubes.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkStripper.h"
#include "vtkLinearSubdivisionFilter.h"

//VMTK
#include "vtkvmtkFastMarchingUpwindGradientImageFilter.h"
#include "vtkvmtkGeodesicActiveContourLevelSetImageFilter.h"
#include "vtkvmtkCenterlineBranchExtractor.h"
#include "vtkvmtkPolyDataCenterlines.h"
#include "vtkvmtkPolyBallModeller.h"
#include "vtkvmtkCapPolyData.h"


VesselSegmentation::VesselSegmentation()
{
}


VesselSegmentation::~VesselSegmentation()
{
}

void VesselSegmentation::SegmentVessel(vtkImageData* pInput, vtkImageData* pOutput,
    double dLowerThreshold, double dHigherThreshold, vtkIdList*  pSourceSeedIds, vtkIdList* pTargetSeedIds, int iInitializeMode)
{
    vtkImageCast* ImageCast = vtkImageCast::New();
    ImageCast->SetInputData(pInput);
    ImageCast->SetOutputScalarTypeToFloat();
    ImageCast->Update();
   
    auto initializedImage = vtkSmartPointer<vtkImageData>::New();
    SegmentationInitialize(ImageCast->GetOutput(), initializedImage.GetPointer(), dLowerThreshold, dHigherThreshold, pSourceSeedIds, pTargetSeedIds, iInitializeMode);

    auto levelSets = vtkSmartPointer<vtkvmtkGeodesicActiveContourLevelSetImageFilter>::New();
    levelSets->SetFeatureImage(ImageCast->GetOutput());
    levelSets->SetDerivativeSigma(0.0);
    levelSets->SetAutoGenerateSpeedAdvection(1);
    levelSets->SetPropagationScaling(100);
    levelSets->SetCurvatureScaling(70);
    levelSets->SetAdvectionScaling(100);
    levelSets->SetInputData(initializedImage);
    levelSets->SetNumberOfIterations(10);
    levelSets->SetIsoSurfaceValue(0.0);
    levelSets->SetMaximumRMSError(1E-20);
    levelSets->SetInterpolateSurfaceLocation(1);
    levelSets->SetUseImageSpacing(1);
    levelSets->Update();

    vtkImageGaussianSmooth *gaussian = vtkImageGaussianSmooth::New();
    gaussian->SetInputData(levelSets->GetOutput());
    gaussian->SetDimensionality(3);
    gaussian->SetRadiusFactor(0.49);
    gaussian->SetStandardDeviation(3, 3, 3);
    gaussian->ReleaseDataFlagOn();
    gaussian->UpdateInformation();
    gaussian->Update();

    pOutput->DeepCopy(gaussian->GetOutput());
}

void VesselSegmentation::SegmentVessel(vtkImageData* pInput, vtkPolyData* pOutput,
    double dLowerThreshold, double dHigherThreshold, vtkIdList*  pSourceSeedIds, vtkIdList* pTargetSeedIds, int iInitializeMode)
{
    auto outputImage = vtkSmartPointer<vtkImageData>::New();
    SegmentVessel(pInput, outputImage.Get(), dLowerThreshold, dHigherThreshold, pSourceSeedIds, pTargetSeedIds, iInitializeMode);
    GenerateVesselSurface(outputImage.GetPointer(), pOutput, 1.0);
}

void VesselSegmentation::GenerateVesselSurface(vtkImageData* pInput, vtkPolyData* pOutput, double dThreshold)
{
    vtkMarchingCubes *marchingCubes = vtkMarchingCubes::New();
    marchingCubes->SetInputData(pInput);
    marchingCubes->SetValue(0, dThreshold);
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

    vtkTransformPolyDataFilter* transformer = vtkTransformPolyDataFilter::New();
    transformer->SetInputData(correctedOutput);
    transformer->SetTransform(transformIJKtoRAS);
    transformer->Update();

    vtkPolyDataNormals* normals = vtkPolyDataNormals::New();
    normals->ComputePointNormalsOn();
    normals->SetInputData(transformer->GetOutput());
    normals->SetFeatureAngle(60);
    normals->SetSplitting(1);
    normals->Update();

    vtkStripper* stripper = vtkStripper::New();
    stripper->SetInputData(normals->GetOutput());
    stripper->Update();
    stripper->GetOutput()->Modified();

    pOutput->DeepCopy(stripper->GetOutput());

    stripper->Delete();
    transformer->Delete();
    marchingCubes->Delete();
    reverser->Delete();
    transformIJKtoRAS->Delete();
    normals->Delete();
}

void VesselSegmentation::SegmentationInitialize(vtkImageData* pInput, vtkImageData* pOutput,
    double dLowerThreshold, double dHigherThreshold, vtkIdList*  pSourceSeedIds, vtkIdList* pTargetSeedIds, int mode)
{
    //auto cast = vtkSmartPointer<vtkImageCast>::New();
    //cast->SetInputData(pInput);
    //cast->SetOutputScalarTypeToFloat();
    //cast->Update();

    double* scalarRange = pInput->GetScalarRange();
    int* imageDimensions = pInput->GetDimensions();
    int maxImageDimensions = vtkMath::Max(imageDimensions[0], vtkMath::Max(imageDimensions[1], imageDimensions[2]));

    auto threshold = vtkSmartPointer<vtkImageThreshold>::New();
    threshold->SetInputData(pInput);
    threshold->ThresholdBetween(dLowerThreshold, dHigherThreshold);
    threshold->ReplaceInOff();
    threshold->ReplaceOutOn();
    threshold->SetOutValue(scalarRange[0] - scalarRange[1]);
    threshold->Update();

   scalarRange = threshold->GetOutput()->GetScalarRange();

   auto shiftScale = vtkSmartPointer<vtkImageShiftScale>::New();
   shiftScale->SetInputData(threshold->GetOutput());
   shiftScale->SetShift(-scalarRange[0]);
   shiftScale->SetScale(1 / (scalarRange[1] - scalarRange[0]));
   shiftScale->Update();

   auto fastMarching = vtkSmartPointer<vtkvmtkFastMarchingUpwindGradientImageFilter>::New();
   fastMarching->SetInputData(shiftScale->GetOutput());
   fastMarching->SetSeeds(pSourceSeedIds);
   fastMarching->GenerateGradientImageOn();
   fastMarching->SetTargetOffset(100.0);
   fastMarching->SetTargets(pTargetSeedIds);
   fastMarching->SetTargetReachedModeToNoTargets();
   fastMarching->Update();

   auto subtract = vtkSmartPointer<vtkImageMathematics>::New();
   subtract->SetInputData(fastMarching->GetOutput());
   subtract->SetOperationToAddConstant();
   subtract->SetConstantC(-fastMarching->GetTargetValue());
   subtract->Update();

   pOutput->DeepCopy(subtract->GetOutput());
}

void VesselSegmentation::CapSurface(vtkPolyData* input, vtkPolyData*output)
{
    vtkSmartPointer<vtkCleanPolyData>  surfaceCleaner = vtkSmartPointer<vtkCleanPolyData>::New();
    surfaceCleaner->SetInputData(input);
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

    vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
    normals->SetInputData(subdiv->GetOutput());
    normals->SetAutoOrientNormals(1);
    normals->SetFlipNormals(0);
    normals->SetConsistency(1);
    normals->SplittingOff();
    normals->Update();

    vtkSmartPointer<vtkvmtkCapPolyData> surfaceCapper = vtkSmartPointer<vtkvmtkCapPolyData>::New();
    surfaceCapper->SetInputData(normals->GetOutput());
    surfaceCapper->SetDisplacement(0.0);
    surfaceCapper->SetInPlaneDisplacement(0.0);
    surfaceCapper->Update();

    output->DeepCopy(surfaceCapper->GetOutput());
}
