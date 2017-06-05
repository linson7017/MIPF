#include "ITK_Helpers.h"

namespace ITKHelpers
{

    std::vector<itk::Index<3> > DilatePixelList(const std::vector<itk::Index<3> >& pixelList,
        const itk::ImageRegion<3>& region, const unsigned int radius)
    {
        //std::cout << "DilatePixelList: input has " << pixelList.size() << " pixels." << std::endl;
        // Construct an image of the pixels in the list
        typedef itk::Image<unsigned char, 3> ImageType;
        ImageType::Pointer image = ImageType::New();
        image->SetRegions(region);
        image->Allocate();
        image->FillBuffer(0);

        typedef std::vector<itk::Index<3> > PixelVectorType;

        for (PixelVectorType::const_iterator iter = pixelList.begin(); iter != pixelList.end(); ++iter)
        {
            // Note, this must be 255, not just any non-zero number, for BinaryDilateImageFilter to work properly.
            image->SetPixel(*iter, 255);
        }

        //WriteImage(image.GetPointer(), "beforeDilation.png");

        // Dilate the image
        typedef itk::BinaryBallStructuringElement<ImageType::PixelType, 3> StructuringElementType;
        StructuringElementType structuringElement;
        structuringElement.SetRadius(radius);
        structuringElement.CreateStructuringElement();

        typedef itk::BinaryDilateImageFilter<ImageType, ImageType, StructuringElementType> BinaryDilateImageFilterType;

        BinaryDilateImageFilterType::Pointer dilateFilter = BinaryDilateImageFilterType::New();
        dilateFilter->SetInput(image);
        dilateFilter->SetKernel(structuringElement);
        dilateFilter->Update();

        //WriteImage(dilateFilter->GetOutput(), "afterDilation.png");

        PixelVectorType dilatedPixelList;

        itk::ImageRegionConstIteratorWithIndex<ImageType> imageIterator(dilateFilter->GetOutput(),
            dilateFilter->GetOutput()->GetLargestPossibleRegion());
        while (!imageIterator.IsAtEnd())
        {
            if (imageIterator.Get())
            {
                dilatedPixelList.push_back(imageIterator.GetIndex());
            }
            ++imageIterator;
        }

        //std::cout << "DilatePixelList: output has " << dilatedPixelList.size() << " pixels." << std::endl;
        return dilatedPixelList;
    }

}