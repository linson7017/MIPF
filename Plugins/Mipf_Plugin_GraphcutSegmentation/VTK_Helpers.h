#ifndef VTK_Helpers_h__
#define VTK_Helpers_h__

#pragma once
#include <vtkImageData.h>

namespace VTKHelpers
{

    void SetImageSizeToMatch(vtkImageData* const input, vtkImageData* const output)
    {
        int* dims = input->GetDimensions();
        output->SetDimensions(dims);

        double* origin = input->GetOrigin();
        output->SetOrigin(origin);

        double* spacing = input->GetSpacing();
        output->SetSpacing(spacing);

        int* extent = input->GetExtent();
        output->SetExtent(extent);
    }

    enum PixelValuesEnum { TRANSPARENT_PIXEL = 0, OPAQUE_PIXEL = 255 };

    void MakeImageTransparent(vtkImageData* const image)
    {
        int dims[3];
        image->GetDimensions(dims);

        if (image->GetNumberOfScalarComponents() < 4)
        {
            //image->SetNumberOfScalarComponents(4);
            //image->AllocateScalars();
            image->AllocateScalars(image->GetScalarType(), 4);
        }

        for (int i = 0; i < dims[0]; ++i)
        {
            for (int j = 0; j < dims[1]; ++j)
            {
                for (int k=0;k<dims[2];++k)
                {
                    unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(i, j, k));
                    pixel[3] = OPAQUE_PIXEL;
                }                
            }
        }
        image->Modified();
    }
}

#endif // VTK_Helpers_h__
