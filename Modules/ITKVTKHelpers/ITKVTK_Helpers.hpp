#ifndef ITKVTK_Helpers_hpp__
#define ITKVTK_Helpers_hpp__

#include "ITKVTK_Helpers.h"
#include "itkVTKImageToImageFilter.h"
#include "itkImageToVTKImageFilter.h"

namespace ITKVTKHelpers
{
    template<typename TOutputImage>
    void ConvertVTKImageToITKImage(vtkImageData* inputImage, TOutputImage* outputImage)
    {
        typedef itk::VTKImageToImageFilter<TOutputImage> vtkImageToImageFilterType;
        vtkImageToImageFilterType::Pointer filter = vtkImageToImageFilterType::New();
        filter->SetInput(inputImage);
        filter->Update();
        outputImage->Graft(filter->GetOutput());
    }

    template<typename TIntputImage>
    void ConvertITKImageToVTKImage(TIntputImage* inputImage, vtkImageData* outputImage)
    {
        typedef itk::ImageToVTKImageFilter<TIntputImage> itkImageToVtkImageFilterType;
        itkImageToVtkImageFilterType::Pointer filter = itkImageToVtkImageFilterType::New();
        filter->SetInput(inputImage);
        filter->Update();
        outputImage->DeepCopy(filter->GetOutput());
    }

}


#endif // ITKVTK_Helpers_h__
