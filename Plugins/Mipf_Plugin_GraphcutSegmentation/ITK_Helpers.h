#ifndef ITK_Helpers_h__
#define ITK_Helpers_h__

#pragma once

#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkImageDuplicator.h"
#include "itkResampleImageFilter.h"

namespace ITKHelpers
{

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

std::vector<itk::Index<3> > DilatePixelList(const std::vector<itk::Index<3> >& pixelList,
    const itk::ImageRegion<3>& region, const unsigned int radius);

}


#endif // ITK_Helpers_h__
