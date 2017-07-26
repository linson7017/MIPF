#include "ITK_Helpers.h"

#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryCrossStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkGrayscaleDilateImageFilter.h"
#include "itkImageDuplicator.h"
#include "itkResampleImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkSignedDanielssonDistanceMapImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryContourImageFilter.h"

#include "itkLabelImageGenericInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"

#include "itkImageFileWriter.h"

namespace ITKHelpers
{

    template<typename TInputImage>
    void GetImageScalarRange(const TInputImage* image, double& rangeMin, double& rangeMax)
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
    void Resample(const TInputImage* input, TOutputImage* output, const float resample, float defaultValue)
    {
        TInputImage::SizeType inputSize = input->GetLargestPossibleRegion().GetSize();
        TOutputImage::SizeType outputSize;
        for (int i = 0; i < inputSize.GetSizeDimension(); i++)
        {
            outputSize[i] = inputSize[i] / (float)resample;
        }

        TInputImage::SpacingType inputSpacing = input->GetSpacing();
        TOutputImage::SpacingType outputSpacing;
        for (int i = 0; i < input->GetImageDimension(); i++)
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
        resampleFilter->SetDefaultPixelValue(defaultValue);

        resampleFilter->SetTransform(TransformType::New());
        resampleFilter->UpdateLargestPossibleRegion();
        resampleFilter->Update();

        output->Graft(resampleFilter->GetOutput());
    }

    template<typename TInputImage>
    void SaveImage(TInputImage* image, std::string filePath)
    {
        typedef  itk::ImageFileWriter<TInputImage>   WriterType;
        WriterType::Pointer writer = WriterType::New();
        writer->SetFileName(filePath);
        writer->SetInput(image);
        writer->Update();
    }

    template<typename TInputImage, typename TOutputImage, template<class, typename> class TInterpolator>
    void  ResampleLabelImage(const TInputImage* inputImage, TOutputImage* outputImage, double sampleFactor)
    {
        typedef itk::ResampleImageFilter< TInputImage, TOutputImage >                                ResampleImageFilterType;
        //typedef itk::LinearInterpolateImageFunction< TOutputImage, double >                        LinearInterpolatorType;

        ResampleImageFilterType::Pointer downsampler = ResampleImageFilterType::New();
        downsampler->SetInput(inputImage);

        //check if need smooth
        typedef itk::LabelImageGenericInterpolateImageFunction<TInputImage, TInterpolator, double> GLInterplatorType;
        GLInterplatorType::Pointer   interpolator = GLInterplatorType::New();
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

    template<typename TInputImage, typename TOutputImage, typename InterpolatorType >
    void  DownSampleImage(const TInputImage* inputImage, TOutputImage* outputImage, float sampleFactor)
    {
        typedef itk::ResampleImageFilter< TInputImage, TOutputImage >                                ResampleImageFilterType;
        //typedef itk::LinearInterpolateImageFunction< TOutputImage, double >                        LinearInterpolatorType;

        ResampleImageFilterType::Pointer downsampler = ResampleImageFilterType::New();
        downsampler->SetInput(inputImage);

        InterpolatorType::Pointer interpolator = InterpolatorType::New();
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
    void GetITKImageNonzeroPixels(TInputImage* const imageData, std::set< itk::Index<3>, IndexSortCriterion >& pixels, int value)
    {
        pixels.clear();
        itk::ImageRegionIterator<TInputImage> iterator(imageData,
            imageData->GetLargestPossibleRegion());
        iterator.GoToBegin();

        while (!iterator.IsAtEnd())
        {
            if (static_cast<int>(iterator.Value()) == value)
            {
                pixels.insert(iterator.GetIndex());
            }
            ++iterator;
        }
    }

    template<typename TInputImage>
    void SetITKImagePixel(TInputImage*  imageData, std::set< itk::Index<3>, IndexSortCriterion >& pixels, int value)
    {
        for (std::set< itk::Index<3>, IndexSortCriterion >::iterator it = pixels.begin(); it != pixels.end(); it++)
        {
            if (ITKHelpers::IndexInRegion(*it, imageData->GetBufferedRegion()))
            {
                imageData->SetPixel(*it, value);
            }

        }
    }

    template <typename TInputImage, typename TOutputImage>
    void DilateImage(TInputImage* image, TOutputImage* outImage, float radius, int value)
    {
        typedef itk::BinaryCrossStructuringElement<TInputImage::PixelType, 3> StructuringElementType;
        StructuringElementType structuringElement;
        structuringElement.SetRadius(radius);
        structuringElement.CreateStructuringElement();

        typedef itk::BinaryDilateImageFilter<TInputImage, TOutputImage, StructuringElementType> BinaryDilateImageFilterType;

        BinaryDilateImageFilterType::Pointer dilateFilter = BinaryDilateImageFilterType::New();
        dilateFilter->SetInput(image);
        dilateFilter->SetForegroundValue(value);
        dilateFilter->SetBackgroundValue(0);
        dilateFilter->SetKernel(structuringElement);

        dilateFilter->Update();
        outImage->Graft(dilateFilter->GetOutput());
    }

    template<typename TInputImage>
    bool  FindImageROI(const TInputImage* itkImage, int imageLabel, int* roi)
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

    template<typename TInputImage, typename TOutputImage>
    void FastGenerateNarrowBandImage(TInputImage* const imageData, TOutputImage* output, int size, bool inside, float maximum)
    {
        typedef itk::BinaryContourImageFilter <ImageType, ImageType >
            binaryContourImageFilterType;

        // Outer boundary
        binaryContourImageFilterType::Pointer binaryContourFilter
            = binaryContourImageFilterType::New();
        binaryContourFilter->SetInput(image);
        binaryContourFilter->SetForegroundValue(0);
        binaryContourFilter->SetBackgroundValue(255);
        binaryContourFilter->Update();

        // Invert the result
        typedef itk::InvertIntensityImageFilter <ImageType>
            InvertIntensityImageFilterType;

        InvertIntensityImageFilterType::Pointer invertIntensityFilter
            = InvertIntensityImageFilterType::New();
        invertIntensityFilter->SetInput(binaryContourFilter->GetOutput());
        invertIntensityFilter->Update();

        ImageType::Pointer outerBoundary = ImageType::New();
        outerBoundary->Graft(invertIntensityFilter->GetOutput());

        // Inner boundary
        binaryContourFilter->SetForegroundValue(255);
        binaryContourFilter->SetBackgroundValue(0);
        binaryContourFilter->Update();
    }


    template<typename TInputImage, typename TOutputImage>
    void GenerateNarrowBandImage(TInputImage* const imageData, TOutputImage* output, int size, bool inside, float maximum )
    {
        typedef itk::SignedDanielssonDistanceMapImageFilter<
            TInputImage, UInt3DImageType >  DistanceMFilterType;

        DistanceMFilterType::Pointer distanceMapper = DistanceMFilterType::New();
        distanceMapper->SetInput(imageData);
        distanceMapper->SetUseImageSpacing(true);
        //distanceMapper->SetInputIsBinary(true);
        distanceMapper->Update();

        typedef itk::BinaryThresholdImageFilter<UInt3DImageType, TOutputImage>   ThresholdImageFilterType;
        ThresholdImageFilterType::Pointer threshold = ThresholdImageFilterType::New();
        threshold->SetInput(distanceMapper->GetOutput());
        threshold->SetUpperThreshold(size*(inside ? -1 : 1));
        threshold->SetLowerThreshold(1 * (inside ? -1 : 1));
        threshold->SetOutsideValue(0);
        threshold->SetInsideValue(1);
        threshold->Update();
        output->Graft(threshold->GetOutput());
    }
}