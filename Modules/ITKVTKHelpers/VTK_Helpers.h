#ifndef VTK_Helpers_h__
#define VTK_Helpers_h__

#pragma once
#include "qf_config.h"
                                     
#include <vtkMetaImageWriter.h>
#include <vtkImageData.h>

namespace VTKHelpers
{
#ifndef  VTK_CREATE
# define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()
#endif


    QF_API void SetImageSizeToMatch(vtkImageData* const input, vtkImageData* const output);

    QF_API void MakeImageTransparent(vtkImageData* const image);

    QF_API void SaveVtkImageData(vtkImageData* im, const std::string& filename);

    QF_API bool FindVTKImageROI(vtkImageData* im, int* roi);
}

#endif // VTK_Helpers_h__
