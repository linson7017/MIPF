#include "CQF_CVAlgorithms.h"

#include "vtkPolyData.h"
#include "vtkImageData.h"
#include "vtkSmartPointer.h"
#include "vtkImageThreshold.h"
#include "vtkDiscreteMarchingCubes.h"
#include "vtkWindowedSincPolyDataFilter.h"
#include "vtkImageResample.h"
#include "vtkImageThresholdConnectivity.h"
#include "vtkImageGaussianSmooth.h"

#include <itkBinaryFillholeImageFilter.h>

#include "mathutil.h"

#include "ITKVTK_Helpers.h"


CQF_CVAlgorithms::CQF_CVAlgorithms()
{
}


CQF_CVAlgorithms::~CQF_CVAlgorithms()
{
}

void CQF_CVAlgorithms::GenerateVesselSurface(vtkImageData* pInputImage, vtkPoints* pSeeds, double* dvThreshold, vtkPolyData* pOutput, int iImageType, bool bNeedRecalculateSegmentation )
{
    if (!pInputImage||!pOutput||!pSeeds)
    {
        return;
    }
    auto segmentedData = vtkSmartPointer<vtkImageData>::New();
    SegmentVessels(pInputImage, pSeeds, dvThreshold,segmentedData, iImageType,bNeedRecalculateSegmentation);
    auto discreteSurface  = vtkSmartPointer<vtkDiscreteMarchingCubes>::New();
    discreteSurface->SetInputData(segmentedData);
    discreteSurface->GenerateValues(1, 1, 1);
    discreteSurface->Update();
    vtkPolyData* polyData = discreteSurface->GetOutput();
    if (polyData->GetNumberOfPoints() == 0)
    {
        std::cerr << "Segment Error!";
        return;
    }
    auto smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
    smoother->SetInputConnection(discreteSurface->GetOutputPort());
    smoother->Update();
    pOutput->DeepCopy(smoother->GetOutput());
}


void CQF_CVAlgorithms::SegmentVessels(vtkImageData* pInputImage, vtkPoints* pSeeds, double* dvThreshold, vtkImageData* pOutputImage, int iImageType,bool bNeedRecalculateSegmentation)
{
    if (bNeedRecalculateSegmentation&&pInputImage)
    {    
        if (pSeeds->GetNumberOfPoints() == 0)
        {
            if (iImageType == 1)
            {
                auto enhancedVesselData = vtkSmartPointer<vtkImageData>::New();
                VesselEnhance(pInputImage, enhancedVesselData,dvThreshold);

                auto simpleThreshold = vtkSmartPointer<vtkImageThreshold>::New();
                simpleThreshold->ReplaceInOn();
                simpleThreshold->SetInValue(1);
                simpleThreshold->ReplaceOutOn();
                simpleThreshold->SetOutValue(0);
                simpleThreshold->SetInputData(enhancedVesselData);
                simpleThreshold->ThresholdBetween(dvThreshold[0], dvThreshold[1]);
                simpleThreshold->Update();

                auto itkImage = Int3DImageType::New();
                ITKVTKHelpers::ConvertVTKImageToITKImage<Int3DImageType>(simpleThreshold->GetOutput(), itkImage);

                auto imfill = itk::BinaryFillholeImageFilter<Int3DImageType>::New();
                imfill->SetInput(itkImage);
                imfill->Update();

                ITKVTKHelpers::ConvertITKImageToVTKImage(imfill->GetOutput(), pOutputImage);
            }
            else
            {
                int* dims = pInputImage->GetDimensions();
                float* inputdataPointer = static_cast<float*>(pInputImage->GetScalarPointer());
                double maxvalue = -1000000;
                int index;
                double point[3];
                for (int i = 0; i < dims[0] * dims[1] * dims[2]; i++)
                {
                    if (inputdataPointer[i] > maxvalue)
                    {
                        maxvalue = inputdataPointer[i];
                        index = i;
                    }
                }
                pInputImage->GetPoint(index, point);

                vtkSmartPointer<vtkPoints> seeds = vtkSmartPointer<vtkPoints>::New();
                seeds->InsertNextPoint(point);
                vtkSmartPointer<vtkImageThresholdConnectivity> connectedthreshold = vtkSmartPointer<vtkImageThresholdConnectivity>::New();
                connectedthreshold->ReplaceInOn();
                connectedthreshold->SetInValue(1);
                connectedthreshold->ReplaceOutOn();
                connectedthreshold->SetOutValue(0);
                connectedthreshold->SetNeighborhoodRadius(1, 1, 1);
                connectedthreshold->SetInputData(pInputImage);
                connectedthreshold->SetSeedPoints(seeds);
                connectedthreshold->ThresholdBetween(dvThreshold[0], dvThreshold[1]);
                connectedthreshold->Update();


                auto itkImage = Int3DImageType::New();
                ITKVTKHelpers::ConvertVTKImageToITKImage<Int3DImageType>(connectedthreshold->GetOutput(), itkImage);

                auto imfill = itk::BinaryFillholeImageFilter<Int3DImageType>::New();
                imfill->SetInput(itkImage);
                imfill->Update();

                ITKVTKHelpers::ConvertITKImageToVTKImage(imfill->GetOutput(), pOutputImage);
            }
        }
        else
        {
            vtkSmartPointer<vtkImageThresholdConnectivity> connectedThreshold = vtkSmartPointer<vtkImageThresholdConnectivity>::New();
            connectedThreshold->ReplaceInOn();
            connectedThreshold->SetInValue(1);
            connectedThreshold->ReplaceOutOn();
            connectedThreshold->SetOutValue(0);
            connectedThreshold->SetNeighborhoodRadius(1, 1, 1);
            if (iImageType == 1)
            {
                auto enhancedVesselData = vtkSmartPointer<vtkImageData>::New();
                VesselEnhance(pInputImage, enhancedVesselData,dvThreshold);
                connectedThreshold->SetInputData(enhancedVesselData);
            }
            else
            {
                connectedThreshold->SetInputData(pInputImage);
            }
            connectedThreshold->SetSeedPoints(pSeeds);
            connectedThreshold->ThresholdBetween(dvThreshold[0], dvThreshold[1]);
            connectedThreshold->Update();
            

            auto itkImage = Int3DImageType::New();
            ITKVTKHelpers::ConvertVTKImageToITKImage<Int3DImageType>(connectedThreshold->GetOutput(), itkImage);

            auto imfill = itk::BinaryFillholeImageFilter<Int3DImageType>::New();
            imfill->SetInput(itkImage);
            imfill->Update();

            ITKVTKHelpers::ConvertITKImageToVTKImage(imfill->GetOutput(), pOutputImage);
        }
        //needRecalculateSegmentation = false;
    }
}

void CQF_CVAlgorithms::VesselEnhance(vtkImageData* pInputImage, vtkImageData* pOutputImage, double* dvThreshold)
{
    auto vesselness = vtkSmartPointer<vtkImageData>::New();
    CalculateVesselness(pInputImage,vesselness.Get(),dvThreshold);
    if (1)
    {
        pOutputImage->DeepCopy(pInputImage);
        int* dims = pOutputImage->GetDimensions();
        float* vesselnessPointer = static_cast<float*>(vesselness->GetScalarPointer());
        float* enhancedVesselDataPointer = static_cast<float*>(pOutputImage->GetScalarPointer());

        for (int i = 0; i < dims[0] * dims[1] * dims[2]; i++)
        {
            if (vesselnessPointer[i] < dvThreshold[2])
            {
                enhancedVesselDataPointer[i] = pInputImage->GetScalarRange()[0] - 1000;
            }
        }

        //needRecalculateEnhancedData = false;
    }
}

void CQF_CVAlgorithms::CalculateVesselness(vtkImageData* pInputImage, vtkImageData* pOutputImage, double* dvThreshold)
{
    if (1)
    {
        int sigmas[4] = { 1, 2, 3, 4 };
        double A = 2 * 0.5 * 0.5;
        double B = 2 * 0.5 * 0.5;
        double C = 2 * 0.2 * 0.2;

        int* origDims = pInputImage->GetDimensions();

        vtkSmartPointer<vtkImageResample> reducer = vtkSmartPointer<vtkImageResample>::New();
        reducer->SetAxisMagnificationFactor(0, 0.5);
        reducer->SetAxisMagnificationFactor(1, 0.5);
        reducer->SetAxisMagnificationFactor(2, 0.5);
        reducer->SetInputData(pInputImage);
        reducer->Update();

        vtkSmartPointer<vtkImageThreshold> thresh = vtkSmartPointer<vtkImageThreshold>::New();
        thresh->ReplaceInOn();
        thresh->SetInValue(1);
        thresh->ReplaceOutOn();
        thresh->SetOutValue(0);
        thresh->SetInputData(reducer->GetOutput());
        thresh->ThresholdBetween(dvThreshold[0], dvThreshold[1]);
        thresh->Update();

        vtkSmartPointer<vtkImageData> input = thresh->GetOutput();
        vtkSmartPointer<vtkImageGaussianSmooth> gaussian = vtkSmartPointer<vtkImageGaussianSmooth>::New();
        auto maxVessel = vtkSmartPointer<vtkImageData>::New();
        vtkSmartPointer<vtkImageData> resVesselness = vtkSmartPointer<vtkImageData>::New();
        resVesselness->DeepCopy(input);
        maxVessel->DeepCopy(input);

        int* dims = thresh->GetOutput()->GetDimensions();
        float* inputPointer = static_cast<float*>(input->GetScalarPointer());
        float* maxVesselPointer = static_cast<float*>(maxVessel->GetScalarPointer());
        float* resVesselnessPointer = static_cast<float*>(resVesselness->GetScalarPointer());

        for (int i = 0; i < 4; i++)
        {
            gaussian->SetStandardDeviations(sigmas[i], sigmas[i], sigmas[i]);
            gaussian->SetRadiusFactors(sigmas[i] * 3, sigmas[i] * 3, sigmas[i] * 3);
            gaussian->SetInputData(input);
            gaussian->Update();
            vtkSmartPointer<vtkImageData> g = gaussian->GetOutput();
            for (int z = 0; z < dims[2]; z++)
            {
                for (int y = 0; y < dims[1]; y++)
                {
                    for (int x = 0; x < dims[0]; x++)
                    {
                        int sub = sub2id(x, y, z, dims);
                        if (inputPointer[sub] > 0) {
                            if (i == 0)
                            {
                                maxVesselPointer[sub] = 0;
                            }
                            if (x > 1 && x < dims[0] - 2 && y > 1 && y < dims[1] - 2 && z > 1 && z < dims[2] - 2) {
                                float dxx = (g->GetScalarComponentAsFloat(x + 2, y, z, 0) +
                                    g->GetScalarComponentAsFloat(x - 2, y, z, 0) -
                                    2 * g->GetScalarComponentAsFloat(x, y, z, 0)) / 4.0;
                                float dyy = (g->GetScalarComponentAsFloat(x, y + 2, z, 0) +
                                    g->GetScalarComponentAsFloat(x, y - 2, z, 0) -
                                    2 * g->GetScalarComponentAsFloat(x, y, z, 0)) / 4.0;
                                float dzz = (g->GetScalarComponentAsFloat(x, y, z + 2, 0) +
                                    g->GetScalarComponentAsFloat(x, y, z - 2, 0) -
                                    2 * g->GetScalarComponentAsFloat(x, y, z, 0)) / 4.0;
                                float dxy = (g->GetScalarComponentAsFloat(x + 1, y + 1, z, 0) +
                                    g->GetScalarComponentAsFloat(x - 1, y - 1, z, 0) -
                                    g->GetScalarComponentAsFloat(x - 1, y + 1, z, 0) -
                                    g->GetScalarComponentAsFloat(x + 1, y - 1, z, 0)) / 4.0;
                                float dxz = (g->GetScalarComponentAsFloat(x + 1, y, z + 1, 0) +
                                    g->GetScalarComponentAsFloat(x - 1, y, z - 1, 0) -
                                    g->GetScalarComponentAsFloat(x - 1, y, z + 1, 0) -
                                    g->GetScalarComponentAsFloat(x + 1, y, z - 1, 0)) / 4.0;
                                float dyz = (g->GetScalarComponentAsFloat(x, y + 1, z + 1, 0) +
                                    g->GetScalarComponentAsFloat(x, y - 1, z - 1, 0) -
                                    g->GetScalarComponentAsFloat(x, y + 1, z - 1, 0) -
                                    g->GetScalarComponentAsFloat(x, y - 1, z + 1, 0)) / 4.0;
                                float c = sigmas[i] * sigmas[i];
                                double lambda[3];
                                eig3volume(dxx, dxy, dxz, dyy, dyz, dzz, c, lambda);
                                double ra = fabs(lambda[1]) / fabs(lambda[2]);
                                double rb = fabs(lambda[0]) / sqrt(fabs(lambda[1]) * fabs(lambda[2]));
                                double S = sqrt(lambda[0] * lambda[0] + lambda[1] * lambda[1] + lambda[2] * lambda[2]);
                                double expRa = 1 - exp(-ra * ra / A);
                                double expRb = exp(-rb * rb / B);
                                double expS = 1 - exp(-S * S / C);
                                float value = expRa * expRb * expS;
                                if (maxVesselPointer[sub] < value)
                                {
                                    maxVesselPointer[sub] = value;
                                }
                                if (i == 3)
                                {
                                    resVesselnessPointer[sub] = maxVesselPointer[sub];
                                }
                            }
                            else
                            {
                                resVesselnessPointer[sub] = -1;
                            }
                        }
                        else
                        {
                            resVesselnessPointer[sub] = -1;
                        }
                    }
                }
            }
        }

        double range[2];
        resVesselness->GetScalarRange(range);

        pOutputImage->DeepCopy(pInputImage);

        float* vesselnessPointer = static_cast<float*>(pOutputImage->GetScalarPointer());
        for (int z = 0; z < origDims[2]; z++)
        {
            for (int y = 0; y < origDims[1]; y++)
            {
                for (int x = 0; x < origDims[0]; x++)
                {
                    float value = resVesselnessPointer[sub2id(x / 2, y / 2, z / 2, dims)];
                    if (value >= 0)
                    {
                        vesselnessPointer[sub2id(x, y, z, origDims)] = value / range[1];
                    }
                    else
                    {
                        vesselnessPointer[sub2id(x, y, z, origDims)] = -1;
                    }
                }
            }
        }
        //needRecalculateVesselness = false;
    }
}

void CQF_CVAlgorithms::AautoSelectRange(vtkImageData* pInputImage, double* outputRange, int modality)
{
    //if (!pInputImage||!outputRange)
    //{
    //    return;
    //}
    //// Reset default th
    //double defaultLowerth = -FLT_MAX;

    //double range;
    //double valuesRange[2];
    //pInputImage->GetScalarRange(valuesRange);


    //// Find optimal threshold
    //// DSA
    //if (modality == 0)
    //{
    //    outputRange[0] = (valuesRange[0] * 2 + valuesRange[1]) / 3;
    //    outputRange[1] = valuesRange[1];
    //    range = (lowerth - valuesRange[0]) / 2.0;
    //}
    //// CTA
    //if (modality == 1)
    //{
    //    lowerth = valuesRange[0] > 200 ? valuesRange[0] : 200;
    //    upperth = valuesRange[1] < 800 ? valuesRange[1] : 800;

    //    int n = seedPoints->GetNumberOfPoints();
    //    if (n > 0)
    //    {
    //        double mean = 0;
    //        int id[3];
    //        vtkSmartPointer<vtkImageData> data = resampler->GetOutput();
    //        for (int i = 0; i < n; i++)
    //        {
    //            int* dims = data->GetDimensions();
    //            id2sub(data->FindPoint(seedPoints->GetPoint(i)), dims, id);
    //            mean += data->GetScalarComponentAsDouble(id[0], id[1], id[2], 0) / n;
    //        }
    //        lowerth = mean - 300;
    //        upperth = mean + 300;
    //    }
    //    range = 400;
    //}
    //// MRA
    //if (modality == 2)
    //{
    //    if (defaultLowerth == -99999)
    //    {
    //        vtkSmartPointer<vtkImageThresholdConnectivity> conn = vtkSmartPointer<vtkImageThresholdConnectivity>::New();
    //        conn->ReplaceInOn();
    //        conn->SetInValue(1);
    //        conn->ReplaceOutOn();
    //        conn->SetOutValue(0);
    //        conn->SetInputConnection(resampler->GetOutputPort());
    //        vtkSmartPointer<vtkImageData> imageData = resampler->GetOutput();
    //        int* dims = imageData->GetDimensions();

    //        int n = 0;
    //        float total = 0;
    //        double maxPoint[3];
    //        float* valuePointer = static_cast<float*>(imageData->GetScalarPointer());
    //        for (int i = 0; i < dims[0] * dims[1] * dims[2]; i++)
    //        {
    //            float value = valuePointer[i];
    //            if (value < valuesRange[1])
    //            {
    //                total += value;
    //                n += 1;
    //            }
    //            else
    //            {
    //                imageData->GetPoint(i, maxPoint);
    //            }
    //        }

    //        float lower = (total / n + valuesRange[1]) / 2.0;

    //        vtkSmartPointer<vtkPoints> maxSeed = vtkSmartPointer<vtkPoints>::New();
    //        maxSeed->InsertPoint(0, maxPoint);
    //        conn->SetSeedPoints(maxSeed);

    //        int loop = 0;
    //        while (true)
    //        {
    //            float totalIn = 0;
    //            int nIn = 0;
    //            float totalOut = 0;
    //            int nOut = 0;
    //            conn->ThresholdBetween(lower, valuesRange[1]);
    //            conn->Update();
    //            vtkSmartPointer<vtkImageData> threshData = conn->GetOutput();
    //            float* threshDataPointer = static_cast<float*>(threshData->GetScalarPointer());
    //            for (int i = 0; i < dims[0] * dims[1] * dims[2]; i++)
    //            {
    //                float value = valuePointer[i];
    //                float thresh = threshDataPointer[i];
    //                if (thresh == 0)
    //                {
    //                    totalOut += value;
    //                    nOut += 1;
    //                }
    //                else
    //                {
    //                    totalIn += value;
    //                    nIn += 1;
    //                }
    //            }
    //            float newLower = (totalOut / nOut + totalIn / nIn) / 2.0;
    //            lower = newLower;
    //            loop += 1;
    //            if (fabs(lower - newLower) < 5 || loop > 3)
    //            {
    //                lower = newLower;
    //                break;
    //            }
    //            lower = newLower;
    //        }
    //        defaultLowerth = lower;
    //    }
    //    lowerth = defaultLowerth;
    //    upperth = valuesRange[1];
    //    range = 800;
    //}
    //threshMinimum = lowerth - range > valuesRange[0] ? lowerth - range : valuesRange[0];
    //threshMaximum = upperth + range < valuesRange[1] ? upperth + range : valuesRange[1];
}