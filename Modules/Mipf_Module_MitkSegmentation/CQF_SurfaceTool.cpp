#include "CQF_SurfaceTool.h"

//mitk
#include "mitkManualSegmentationToSurfaceFilter.h"
#include "mitkImageToSurfaceFilter.h"
#include "mitkDataNode.h"
#include "mitkSurfaceToImageFilter.h"
#include "mitkImageReadAccessor.h"

//vtk
#include "vtkPolyDataConnectivityFilter.h"
#include "vtkWindowedSincPolyDataFilter.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkPolyDataNormals.h"
#include "vtkDecimatePro.h"
#include "vtkQuadricDecimation.h"


CQF_SurfaceTool::CQF_SurfaceTool()
{
}


CQF_SurfaceTool::~CQF_SurfaceTool()
{
}

bool CQF_SurfaceTool::ExtractSurface(mitk::Image* pImage, vtkPolyData* pOutput, int iSmooth, bool bLargestConnected)
{
    if (!pImage||!pOutput)
    {
        return false;
    }
    bool applyMedian = false;
    bool decimateMesh = true;
    unsigned int medianKernelSize = 3;
    float gaussianSD = 1.5;
    float reductionRate = 0.5;
    bool smooth = false;
    if (iSmooth>0)
    {
        applyMedian = true;
        gaussianSD = 1.73205;
        smooth = true;
    }

    mitk::ManualSegmentationToSurfaceFilter::Pointer surfaceFilter = mitk::ManualSegmentationToSurfaceFilter::New();
    surfaceFilter->SetInput(pImage);
    surfaceFilter->SetThreshold(0.5); // expects binary image with zeros and ones

    surfaceFilter->SetUseGaussianImageSmooth(smooth); // apply gaussian to thresholded image ?
    surfaceFilter->SetSmooth(smooth);
    if (smooth)
    {
        surfaceFilter->InterpolationOn();
        surfaceFilter->SetGaussianStandardDeviation(gaussianSD);
    }

    surfaceFilter->SetMedianFilter3D(applyMedian); // apply median to segmentation before marching cubes ?
    if (applyMedian)
    {
        surfaceFilter->SetMedianKernelSize(
            medianKernelSize, medianKernelSize, medianKernelSize); // apply median to segmentation before marching cubes
    }

    // fix to avoid vtk warnings see bug #5390
    if (pImage->GetDimension() > 3)
        decimateMesh = false;

    if (decimateMesh)
    {
        surfaceFilter->SetDecimate(mitk::ImageToSurfaceFilter::QuadricDecimation);
        surfaceFilter->SetTargetReduction(reductionRate);
    }
    else
    {
        surfaceFilter->SetDecimate(mitk::ImageToSurfaceFilter::NoDecimation);
    }

    try
    {
        surfaceFilter->UpdateLargestPossibleRegion();
    }
    catch (std::exception e)
    {
        std::cout << "Extract Failed!" << std::endl;
        return false;
    }


    // calculate normals for nicer display
    pOutput->DeepCopy(surfaceFilter->GetOutput()->GetVtkPolyData());

    if (!pOutput)
        throw std::logic_error("Could not create polygon model");

    pOutput->SetVerts(0);
    pOutput->SetLines(0);

    //largest connected ios
    if (bLargestConnected)
    {
        vtkSmartPointer<vtkPolyDataConnectivityFilter> confilter =
            vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
        confilter->SetInputData(pOutput);
        confilter->SetExtractionModeToLargestRegion();
        confilter->Update();
        pOutput->DeepCopy(confilter->GetOutput());
    }

    if (smooth)
    {
        vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoother =
            vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
        smoother->SetInputData(pOutput);
        smoother->SetNumberOfIterations(iSmooth);
        smoother->BoundarySmoothingOff();
        smoother->FeatureEdgeSmoothingOff();
        smoother->SetFeatureAngle(60.0);
        smoother->SetPassBand(.001);
        smoother->NonManifoldSmoothingOn();
        smoother->NormalizeCoordinatesOn();
        smoother->Update();
        pOutput->DeepCopy(smoother->GetOutput());
    }

    if (smooth || applyMedian || decimateMesh)
    {
        vtkPolyDataNormals *normalsGen = vtkPolyDataNormals::New();

        normalsGen->AutoOrientNormalsOn();
        normalsGen->FlipNormalsOff();
        normalsGen->SetInputData(pOutput);
        normalsGen->Update();
        pOutput->DeepCopy(normalsGen->GetOutput());
        normalsGen->Delete();
    }

    return true;
}

bool CQF_SurfaceTool::ExtractSurface(mitk::DataNode* pNode, vtkPolyData* pOutput, int iSmooth, bool bLargestConnected)
{
    mitk::Image* image = dynamic_cast<mitk::Image*>(pNode->GetData());
    if (image)
    {
        return ExtractSurface(image, pOutput, iSmooth, bLargestConnected);
    }
    else
    {
        return false;
    }
}

void CQF_SurfaceTool::SmoothSurface(vtkPolyData* pInput, vtkPolyData* pOutput, int iSmooth, bool bLargestConnected)
{
    if (bLargestConnected)
    {
        vtkSmartPointer<vtkPolyDataConnectivityFilter> confilter =
            vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
        confilter->SetInputData(pInput);
        confilter->SetExtractionModeToLargestRegion();
        confilter->Update();
        pInput->DeepCopy(confilter->GetOutput());
    }

    if (iSmooth>0)
    {
        vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoother =
            vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
        smoother->SetInputData(pInput);
        smoother->SetNumberOfIterations(iSmooth);
        smoother->BoundarySmoothingOff();
        smoother->FeatureEdgeSmoothingOff();
        smoother->SetFeatureAngle(60.0);
        smoother->SetPassBand(.001);
        smoother->NonManifoldSmoothingOn();
        smoother->NormalizeCoordinatesOn();
        smoother->Update();

        vtkPolyDataNormals *normalsGen = vtkPolyDataNormals::New();
        normalsGen->AutoOrientNormalsOn();
        normalsGen->FlipNormalsOff();
        normalsGen->SetInputData(smoother->GetOutput());
        normalsGen->Update();
        pOutput->DeepCopy(normalsGen->GetOutput());
        normalsGen->Delete();
    }
}

void CQF_SurfaceTool::SmoothTubeSurface(vtkPolyData* pInput, vtkPolyData* pOutput, int iSmooth, bool bLargestConnected)
{
    if (bLargestConnected)
    {
        vtkSmartPointer<vtkPolyDataConnectivityFilter> confilter =
            vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
        confilter->SetInputData(pInput);
        confilter->SetExtractionModeToLargestRegion();
        confilter->Update();
        pInput->DeepCopy(confilter->GetOutput());
    }

    if (iSmooth > 0)
    {
        vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoother =
            vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
        smoother->SetInputData(pInput);
        smoother->SetNumberOfIterations(iSmooth);
        smoother->BoundarySmoothingOn();
        smoother->FeatureEdgeSmoothingOff();
        smoother->SetPassBand(0.1);
        smoother->NormalizeCoordinatesOn();
        smoother->Update();

        vtkPolyDataNormals *normalsGen = vtkPolyDataNormals::New();
        normalsGen->AutoOrientNormalsOn();
        normalsGen->FlipNormalsOff();
        normalsGen->ConsistencyOn();
        normalsGen->ComputeCellNormalsOff();
        normalsGen->SetInputData(smoother->GetOutput());
        normalsGen->Update();
        pOutput->DeepCopy(normalsGen->GetOutput());
        normalsGen->Delete();
    }
}

void CQF_SurfaceTool::SmoothTubeSurfaceUsingLaplace(vtkPolyData* pInput, vtkPolyData* pOutput, int iSmooth, bool bLargestConnected)
{
    if (bLargestConnected)
    {
        vtkSmartPointer<vtkPolyDataConnectivityFilter> confilter =
            vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
        confilter->SetInputData(pInput);
        confilter->SetExtractionModeToLargestRegion();
        confilter->Update();
        pInput->DeepCopy(confilter->GetOutput());
    }

    if (iSmooth > 0)
    {
        vtkSmartPointer<vtkSmoothPolyDataFilter> smoother =
            vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
        smoother->SetInputData(pInput);
        smoother->SetNumberOfIterations(iSmooth);
        smoother->SetRelaxationFactor(0.01);
        smoother->Update();

        vtkPolyDataNormals *normalsGen = vtkPolyDataNormals::New();
        normalsGen->AutoOrientNormalsOn();
        normalsGen->FlipNormalsOff();
        normalsGen->ConsistencyOn();
        normalsGen->ComputeCellNormalsOff();
        normalsGen->SetInputData(smoother->GetOutput());
        normalsGen->Update();
        pOutput->DeepCopy(normalsGen->GetOutput());
        normalsGen->Delete();
    }
}

void CQF_SurfaceTool::ConvertSurfaceToImage(mitk::Surface* surface, mitk::Image* referenceImage, mitk::Image* output)
{
    mitk::SurfaceToImageFilter::Pointer s2iFilter = mitk::SurfaceToImageFilter::New();
    //输出为二值图像
    s2iFilter->MakeOutputBinaryOn();
    if (output->GetPixelType().GetComponentType() == itk::ImageIOBase::USHORT)
        s2iFilter->SetUShortBinaryPixelType(true);
    //输入mitk::Surface数据
    s2iFilter->SetInput(surface);

    //重要，设置面数据图像对应的mitk::Image数据
    s2iFilter->SetImage(referenceImage);
    s2iFilter->Update();

    mitk::Image::Pointer newSeg = s2iFilter->GetOutput();

    mitk::ImageReadAccessor readAccess(newSeg, newSeg->GetVolumeData());
    const void *cPointer = readAccess.GetData();

    if (output && cPointer)
    {
        output->SetVolume(cPointer);
    }
    else
    {
        return;
    }
}

void CQF_SurfaceTool::SimplifySurfaceMesh(vtkPolyData* pInput, vtkPolyData* pOutput, double dRate, int mode)
{
    if (!pInput ||!pOutput)
    {
        return;
    }
    if (mode == 0)
    {
        vtkDecimatePro *decimate = vtkDecimatePro::New();
        decimate->SplittingOff();
        decimate->SetErrorIsAbsolute(5);
        decimate->SetFeatureAngle(30);
        decimate->PreserveTopologyOn();
        decimate->BoundaryVertexDeletionOff();
        decimate->SetDegree(10); // std-value is 25!

        decimate->SetInputData(pInput); // RC++
        decimate->SetTargetReduction(dRate);
        decimate->SetMaximumError(0.002);
        decimate->Update();

        pOutput->DeepCopy(decimate->GetOutput());
        decimate->Delete();
    }
    else
    {
        vtkQuadricDecimation *decimate = vtkQuadricDecimation::New();
        decimate->SetTargetReduction(dRate);

        decimate->SetInputData(pInput);
        decimate->Update();
        pOutput->DeepCopy(decimate->GetOutput());

        decimate->Delete();
    }
}
