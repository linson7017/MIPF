#ifndef ITK_Helpers_h__
#define ITK_Helpers_h__

#pragma once

#include "qf_config.h"

#include "ITKImageTypeDef.h"
#include "itkLinearInterpolateImageFunction.h"

#include "itkShapeLabelObject.h"

namespace ITKHelpers
{

    /** Get Image Scalar Range. */
    template<typename TInputImage>
    void GetImageScalarRange(const TInputImage* image, double& rangeMin, double& rangeMax);

    /** Deep copy a scalar image. */
    template<typename TInputPixel, typename TOutputPixel>
    void DeepCopy(const itk::Image<TInputPixel, 3>* const input, itk::Image<TOutputPixel, 3>* const output);

    /** Deep copy a scalar image in special region. */
    template<typename TInputImage, typename TOutputImage>
    void DeepCopyInRegion(const TInputImage* input, const itk::ImageRegion<3>& region, TOutputImage* output);

    /** Resample an image. */
    template<typename TInputImage, typename TOutputImage>
    void Resample(const TInputImage* input, TOutputImage* output, const float resample, float defaultValue = 0);

    template<typename TInputImage>
    void SaveImage(TInputImage* image, std::string filePath);

    template<typename TInputImage, typename TOutputImage, template<class, typename> class TInterpolator=itk::LinearInterpolateImageFunction>
    void  ResampleLabelImage(const TInputImage* inputImage, TOutputImage* outputImage, double sampleFactor);

    template<typename TInputImage, typename TOutputImage,typename InterpolatorType = itk::LinearInterpolateImageFunction<typename TOutputImage, double > >
    void  DownSampleImage(const TInputImage* inputImage, TOutputImage* outputImage, float sampleFactor);

    template<typename TInputImage>
    void GetITKImageNonzeroPixels(TInputImage* const imageData, std::set< itk::Index<3>, IndexSortCriterion >& pixels, int value);

    template<typename TInputImage>
    void SetITKImagePixel(TInputImage*  imageData, std::set< itk::Index<3>, IndexSortCriterion >& pixels, int value);

    template <typename TInputImage, typename TOutputImage>
    void DilateImage(TInputImage* image, TOutputImage* outImage, float radius, int value = 1);

    template <typename TInputImage, typename TOutputImage>
    void OpeningBinaryImage(TInputImage* image, TOutputImage* outImage, float radius);

    template<typename TInputImage>
    bool  FindImageROI(const TInputImage* itkImage, int imageLabel, int* roi);

    template<typename TInputImage, typename TOutputImage>
    void FastGenerateNarrowBandImage(TInputImage* const imageData, TOutputImage* output, int size = 1, bool inside = false, float maximum = 1);


    template<typename TInputImage,typename TOutputImage>
    void GenerateNarrowBandImage(TInputImage* const imageData, TOutputImage* output, int size = 1, bool inside = false, float maximum = 1);

    template <class TInput, class TOutput>
    void ExtractLargestConnected(TInput* input, TOutput* output);

    template <class TInput, class TOutput, class TAttributeType = itk::ShapeLabelObject<int, 3>::AttributeType>
    void ExtractConnectedLargerThan(TInput* input, TOutput* output, double size = 1000.0,
        TAttributeType attributeType = itk::ShapeLabelObject<int, 3>::NUMBER_OF_PIXELS, bool reverse = false);

    template <class TInput, class TOutput>
    void ExtractConnectedContainsIndex(TInput* input, TOutput* output, IndexContainer indexes);

    template <class TInput, class TOutput>
    void ExtractCentroidImageWithGivenSize(TInput* input, TOutput* output, int* outputSize);

    template <class TImageType>
    void BinaryFillLargeHolesByRegionGrowing(TImageType* input, TImageType* output,itk::Index<TImageType::ImageDimension> seed);

    QF_API std::vector<itk::Index<3> > DilatePixelList(const std::vector<itk::Index<3> >& pixelList,
        const itk::ImageRegion<3>& region, const unsigned int radius);

    QF_API bool IndexInRegion(const itk::Index<3>& pixel, const itk::ImageRegion<3>& region);

}


#include "ITK_Helpers.hpp"

#endif // ITK_Helpers_h__
