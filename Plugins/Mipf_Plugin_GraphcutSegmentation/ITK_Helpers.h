#ifndef ITK_Helpers_h__
#define ITK_Helpers_h__

#pragma once

#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkImageDuplicator.h"
#include "itkResampleImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"

namespace ITKHelpers
{


    template<typename TInputImage>
    void GetImageScalarRange(const TInputImage* image, double& rangeMin,double& rangeMax)
    {
        typedef itk::MinimumMaximumImageCalculator <TInputImage>
            ImageCalculatorFilterType;
        ImageCalculatorFilterType::Pointer imageCalculatorFilter
            = ImageCalculatorFilterType::New();
        imageCalculatorFilter->SetImage(image);
        imageCalculatorFilter->Compute();

        rangeMin = (double)imageCalculatorFilter->GetMinimum();
        rangeMax = (double)imageCalculatorFilter->GetMaximum();
    }

/** Deep copy a scalar image. */
template<typename TInputPixel, typename TOutputPixel>
void DeepCopy(const itk::Image<TInputPixel, 3>* const input, itk::Image<TOutputPixel, 3>* const output)
{
    if (output->GetLargestPossibleRegion() != input->GetLargestPossibleRegion())
    {
        output->SetRegions(input->GetLargestPossibleRegion());
        output->Allocate();
    }
    DeepCopyInRegion(input, input->GetLargestPossibleRegion(), output);
}


template<typename TInputImage, typename TOutputImage>
void DeepCopyInRegion(const TInputImage* input, const itk::ImageRegion<3>& region, TOutputImage* output)
{
    // This function assumes that the size of input and output are the same.

    itk::ImageRegionConstIterator<TInputImage> inputIterator(input, region);
    itk::ImageRegionIterator<TOutputImage> outputIterator(output, region);

    while (!inputIterator.IsAtEnd())
    {
        outputIterator.Set(inputIterator.Get());
        ++inputIterator;
        ++outputIterator;
    }
}

template<typename TInputImage, typename TOutputImage>
void Resample(const TInputImage* input , TOutputImage* output, const int resample)
{
    TInputImage::SizeType inputSize = input->GetLargestPossibleRegion().GetSize();
    TOutputImage::SizeType outputSize;
    for (int i=0;i<inputSize.GetSizeDimension();i++)
    {
        outputSize[i] = inputSize[i] / (float)resample;
    }

    TInputImage::SpacingType inputSpacing = input->GetSpacing();
    TOutputImage::SpacingType outputSpacing;
    for (int i = 0;i < input->GetImageDimension();i++)
    {
        outputSpacing[i] = inputSpacing[i] * (float)resample;
    }

    typedef itk::IdentityTransform<double, 3> TransformType;
    typedef itk::ResampleImageFilter<TInputImage, TOutputImage> ResampleImageFilterType;
    ResampleImageFilterType::Pointer resampleFilter = ResampleImageFilterType::New();
    resampleFilter->SetInput(input);
    resampleFilter->SetSize(outputSize);
    resampleFilter->SetOutputSpacing(outputSpacing);
    resampleFilter->SetOutputOrigin(input->GetOrigin());
    resampleFilter->SetOutputDirection(input->GetDirection());
    resampleFilter->SetDefaultPixelValue(100);

    resampleFilter->SetTransform(TransformType::New());
    resampleFilter->UpdateLargestPossibleRegion();
    resampleFilter->Update();

    output->Graft(resampleFilter->GetOutput());
}

template<typename TInputImage>
void SaveImage(TInputImage* image,std::string filePath)
{
    typedef  itk::ImageFileWriter<TInputImage>   WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName(filePath);
    writer->SetInput(image);
    writer->Update();
}

template<typename TInputImage, typename TOutputImage>
void  DownSampleImage(const TInputImage* inputImage, TOutputImage* outputImage, int sampleFactor)
{
    typedef itk::ResampleImageFilter< TInputImage, TOutputImage >                                ResampleImageFilterType;
    typedef itk::LinearInterpolateImageFunction< TOutputImage, double >                        LinearInterpolatorType;

    ResampleImageFilterType::Pointer downsampler = ResampleImageFilterType::New();
    downsampler->SetInput(inputImage);

    LinearInterpolatorType::Pointer interpolator = LinearInterpolatorType::New();
    downsampler->SetInterpolator(interpolator);

    downsampler->SetDefaultPixelValue(0);

    ResampleImageFilterType::SpacingType spacing = inputImage->GetSpacing();
    spacing *= (double)sampleFactor;
    downsampler->SetOutputSpacing(spacing);

    downsampler->SetOutputOrigin(inputImage->GetOrigin());
    downsampler->SetOutputDirection(inputImage->GetDirection());

    ResampleImageFilterType::SizeType size = inputImage->GetLargestPossibleRegion().GetSize();
    for (int i = 0; i < 3; ++i)
    {
        size[i] /= sampleFactor;
    }
    downsampler->SetSize(size);
    downsampler->UpdateLargestPossibleRegion();

    outputImage->Graft(downsampler->GetOutput());
}

template<typename TInputImage>
void GetITKImageNonzeroPixels(TInputImage* const imageData, std::vector<itk::Index<3> >& pixels)
{
    TInputImage::SizeType size = inputImage->GetLargestPossibleRegion().GetSize();
    int contourPixelCount = 0;
    for (int x = 0; x < size[0]; x++)
    {
        for (int y = 0; y < size[1]; y++)
        {
            for (int z = 0; z < size[2]; z++)
            {
                TInputImage::IndexType cindex;
                cindex[0] = x;
                cindex[1] = y;
                cindex[2] = z;
                if ((int)inputImage->GetPixel(cindex) != 0)
                {
                    itk::Index<3> index;
                    index[0] = x;
                    index[1] = y;
                    index[2] = z;
                    pixels.push_back(index);
                }
            }
        }
    }
}

template<typename TInputImage>
bool  FindImageROI(const TInputImage* itkImage,int imageLabel, int* roi)
{
    bool roiFound = false;

    TInputImage::IndexType minIndex;
    minIndex.Fill(numeric_limits<TInputImage::IndexValueType>::max());
    TInputImage::IndexType maxIndex;
    maxIndex.Fill(numeric_limits<TInputImage::IndexValueType>::min());

    itk::ImageRegionIteratorWithIndex<TInputImage> iter(itkImage, itkImage->GetLargestPossibleRegion());
    for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
    {
        if (iter.Get() == imageLabel)
        {
            roiFound = true;
            iter.Set(1);

            TInputImage::IndexType currentIndex = iter.GetIndex();

            for (unsigned int dim = 0; dim < 3; ++dim)
            {
                minIndex[dim] = min(currentIndex[dim], minIndex[dim]);
                maxIndex[dim] = max(currentIndex[dim], maxIndex[dim]);
            }
        }
        else
        {
            iter.Set(0);
        }
    }
    return   roiFound;
}

std::vector<itk::Index<3> > DilatePixelList(const std::vector<itk::Index<3> >& pixelList,
    const itk::ImageRegion<3>& region, const unsigned int radius);

bool IndexInRegion(const itk::Index<3>& pixel, const itk::ImageRegion<3>& region);

}


#endif // ITK_Helpers_h__
