#ifndef VTK_Helpers_h__
#define VTK_Helpers_h__

#pragma once

#include <vtkMetaImageWriter.h>

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

    void SaveVtkImageData(vtkImageData* im, const std::string& filename )
    {
        vtkSmartPointer<vtkMetaImageWriter> writer =
            vtkSmartPointer<vtkMetaImageWriter>::New();
        writer->SetInputData(im);
        writer->SetFileName(filename.c_str());
        std::string rawfilename = filename.substr(0, filename.find_last_of("."));
        rawfilename.append(".raw");
        writer->SetRAWFileName(rawfilename.c_str());
        writer->Write();
    }

    template<typename PixelType>
    bool FindVTKImageROI(vtkImageData* im, int* roi)
    {
        if (im->GetDataDimension()!=3)
        {
            return false;
        }

        int* DIMS;
        long i, j, k, kk;
        bool foundLabel;

        DIMS = im->GetDimensions();
        foundLabel = false;
        //roi.resize(6);
        for (i = 0; i < DIMS[0]; i++)
        {
            for (j = 0; j < DIMS[1]; j++)
            {
                for (k = 0; k < DIMS[2]; k++)
                {
                    unsigned char* pixel = static_cast<unsigned char*>(im->GetScalarPointer(i,j,k));
                    if (*pixel != 0)
                    {

                        if (!foundLabel) {
                            roi[0] = i;  roi[1] = i;
                            roi[2] = j;  roi[3] = j;
                            roi[4] = k; roi[5] = k;
                        }
                        else {
                            if (i < roi[0]) roi[0] = i;
                            if (i > roi[1]) roi[1] = i;
                            if (j < roi[2]) roi[2] = j;
                            if (j > roi[3]) roi[3] = j;
                            if (k < roi[4]) roi[4] = k;
                            if (k > roi[5]) roi[5] = k;
                        }
                        foundLabel = true;
                    }
                }
            }
        }
        
                

        // Get Editor Radius information
        // TODO: get input from Editor
        int radius = 9;
        for (kk = 0; kk < 3; kk ++) {
            if (roi[2 * kk] - radius >= 0) {
                roi[2 * kk] -= radius;
            }
            if (roi[2 * kk + 1] + radius < DIMS[kk] - 1) {
                roi[2 * kk + 1] += radius;
            }
        }
        return foundLabel;
    }
}

#endif // VTK_Helpers_h__
