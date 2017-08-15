#ifndef ITKVTK_Helpers_h__
#define ITKVTK_Helpers_h__



#pragma once

#include "qf_config.h"
#include <vector>
#include <itkIndex.h>
#include <vtkPoints.h>
#include <itkBresenhamLine.h>
#include <VTKImageProperties.h>

#include "ITKImageTypeDef.h"

namespace ITKVTKHelpers
{
    template<typename TOutputImage>
    void ConvertVTKImageToITKImage(vtkImageData* inputImage, TOutputImage* outputImage);

    template<typename TIntputImage>
    void ConvertITKImageToVTKImage(TIntputImage* inputImage, vtkImageData* outputImage);


    QF_API std::vector<itk::Index<3> > PointsToPixelList(vtkPoints* const points);

    QF_API void SetPixels(vtkImageData* const VTKImage, const std::vector<itk::Index<3> >& pixels, const unsigned char color[3]);

    QF_API void GetNonzeroPoints(vtkImageData* const imageData, std::vector< itk::Point<double, 3> >& points, int value, int sampleFactor = 1, int* roi = NULL);

    QF_API void PointsToPixels(vtkImageData* const imageData, const std::vector< itk::Point<double, 3> >& points, std::set< itk::Index<3>, IndexSortCriterion > & pixels);

    QF_API void GetNonzeroPixels(vtkImageData* const imageData, std::vector<itk::Index<3> >& pixels, int sampleFactor = 1, int* roi = NULL);

}

#include "ITKVTK_Helpers.hpp"


#endif // ITKVTK_Helpers_h__
