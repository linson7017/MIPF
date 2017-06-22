#ifndef ITKVTK_Helpers_h__
#define ITKVTK_Helpers_h__



#pragma once

#include <vector>
#include <itkIndex.h>
#include <vtkPoints.h>
#include <itkBresenhamLine.h>

namespace ITKVTKHelpers
{
    std::vector<itk::Index<3> > PointsToPixelList(vtkPoints* const points)
    {
        // The points of the polydata are floating point values, we must convert them to pixel indices.

        //std::cout << "Enter PolyDataToPixelList()" << std::endl;
        std::cout << "There are " << points->GetNumberOfPoints() << " points." << std::endl;

        // Convert vtkPoints to indices
        //std::cout << "Converting vtkPoints to indices..." << std::endl;
        std::vector<itk::Index<3> > linePoints;
        for (vtkIdType pointId = 0; pointId < points->GetNumberOfPoints(); ++pointId)
        {
            itk::Index<3> index;
            double p[3];
            points->GetPoint(pointId, p);
            // std::cout << "point " << pointId << " : " << p[0] << " " << p[1] << " " << p[2] << std::endl;

            // Use itk::Math::Round instead of round() for cross-platform compatibility (specifically,
            // VS2010 does not have round() in cmath)
            index[0] = static_cast<itk::Index<3>::IndexValueType>(itk::Math::Round<double, double>(p[0]));
            index[1] = static_cast<itk::Index<3>::IndexValueType>(itk::Math::Round<double, double>(p[1]));
            index[2] = static_cast<itk::Index<3>::IndexValueType>(itk::Math::Round<double, double>(p[2]));
            if (linePoints.size() == 0)
            {
                linePoints.push_back(index);
                continue;
            }

            // Don't duplicate indices of points acquired in a row that round to the same pixel.
            if (index != linePoints[linePoints.size() - 1])
            {
                linePoints.push_back(index);
            }
        }

        if (linePoints.size() < 2)
        {
            std::cerr << "Cannot draw a lines between " << linePoints.size() << " points." << std::endl;
            return linePoints;
        }

        // Compute the indices between every pair of points
        //std::cout << "Computing the indices between every pair of points..." << std::endl;
        std::vector<itk::Index<3> > allIndices;
        for (unsigned int linePointId = 1; linePointId < linePoints.size(); linePointId++)
        {
            //std::cout << "Getting the indices..." << std::endl;
            itk::Index<3> index0 = linePoints[linePointId - 1];
            itk::Index<3> index1 = linePoints[linePointId];

            if (index0 == index1)
            {
                std::cout << "Can't draw a line between the same pixels (" << index0 << " and " << index1 << "!" << std::endl;
                continue;
            }

            //std::cout << "Constructing the line..." << std::endl;
            itk::BresenhamLine<3> line;
            std::vector<itk::Index<3> > indices = line.BuildLine(index0, index1);
            //std::cout << "Saving indices..." << std::endl;
            for (unsigned int i = 0; i < indices.size(); i++)
            {
                allIndices.push_back(indices[i]);
            }

        } // end for loop over line segments

          //std::cout << "Exit PolyDataToPixelList()" << std::endl;
        return allIndices;
    }

    void SetPixels(vtkImageData* const VTKImage, const std::vector<itk::Index<3> >& pixels, const unsigned char color[3])
    {
        int* dims = VTKImage->GetDimensions();

        for (unsigned int i = 0; i < pixels.size(); ++i)
        {
            if (pixels[i][0] >= dims[0] || pixels[i][1] >= dims[1]|| pixels[i][2]>dims[2]) // The pixel is out of bounds
            {
                continue;
            }
            unsigned char* pixel = static_cast<unsigned char*>(VTKImage->GetScalarPointer(pixels[i][0], pixels[i][1], 0));
            pixel[0] = color[0];
            pixel[1] = color[1];
            pixel[2] = color[2];
            // Make sure the pixel is not transparent
            if (VTKImage->GetNumberOfScalarComponents() == 4)
            {
                pixel[3] = 255;
            }
        }

    }

    
    class finder
    {
    public:
        explicit finder(const itk::Index<3>& _t) :t(_t) {}
        const bool operator()(const itk::Index<3>& __t)const 
        { 
            return (t.GetElement(0) == __t.GetElement(0))&&
                (t.GetElement(1) == __t.GetElement(1)) &&
                (t.GetElement(2) == __t.GetElement(2));
        }
    private:
        itk::Index<3> t;
    };
    void GetNonzeroPixels(vtkImageData* const imageData, std::vector<itk::Index<3> >& pixels,int sampleFactor = 1)
    {
        int dims[3];
        imageData->GetDimensions(dims);
        for (int z = 0; z < dims[2]- sampleFactor; z+= sampleFactor)
        {
            for (int y = 0; y < dims[1] - sampleFactor; y+= sampleFactor)
            {
                for (int x = 0; x < dims[0] - sampleFactor; x+= sampleFactor)
                {
                    int* pixel = static_cast<int*>(imageData->GetScalarPointer(x, y, z));
                    if (pixel[0]!=0)
                    {
                        itk::Index<3> index;
                        index[0] = x ;
                        index[1] = y ;
                        index[2] = z ;
                        pixels.push_back(index);
                    }
                }
            }
        }
        int size = pixels.size();
    }

}


#endif // ITKVTK_Helpers_h__
