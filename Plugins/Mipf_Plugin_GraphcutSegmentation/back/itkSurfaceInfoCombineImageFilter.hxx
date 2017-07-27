#include "itkSurfaceInfoCombineImageFilter.h"


template<typename TImageSource, typename TImageDirect>
void DeepCopy(typename TImageSource::ConstPointer source, typename TImageDirect::Pointer direct)
{
    direct->SetRegions(source->GetLargestPossibleRegion());
    direct->Allocate();

    itk::ImageRegionConstIterator<TImageSource> inputIterator(source, source->GetLargestPossibleRegion());
    itk::ImageRegionIterator<TImageDirect> outputIterator(direct, direct->GetLargestPossibleRegion());

    while (!inputIterator.IsAtEnd())
    {
        outputIterator.Set(inputIterator.Get());
        ++inputIterator;
        ++outputIterator;
    }
}

namespace itk
{
    template <class TInputImage, class TInfoImage, class TOutputImage>
    void SurfaceInfoCombineImageFilter<TInputImage, TInfoImage, TOutputImage>::SetInputImage(const TInputImage* image)
    {
        this->SetNthInput(0, const_cast<TInputImage*>(image));
    }

    template <class TInputImage, class TInfoImage, class TOutputImage>
    void SurfaceInfoCombineImageFilter<TInputImage, TInfoImage, TOutputImage>::SetInfoImage(const InfoImageType* image)
    {
        this->SetNthInput(1, const_cast<InfoImageType*>(image));
    }

    template <class TInputImage, class TInfoImage, class TOutputImage>
    bool SurfaceInfoCombineImageFilter<TInputImage, TInfoImage, TOutputImage>::GetNeighbors(std::vector<InputImagePixelType> &array, InputImageConstPointer image, InputImageIndexType index)
    {
        InputImageSizeType size = image->GetLargestPossibleRegion().GetSize();
        int crSize = m_NeighbourSize * 2 + 1;
        if (index[0]<m_NeighbourSize ||
            index[1]<m_NeighbourSize ||
            index[2]<m_NeighbourSize ||
            index[0]>(size[0] - m_NeighbourSize - 1) ||
            index[1]>(size[1] - m_NeighbourSize - 1) ||
            index[2]>(size[2] - m_NeighbourSize - 1))
        {
            return false;
        }

        InputImageIndexType tempIndex;
        int center = 0;
        for (int x = -m_NeighbourSize; x < m_NeighbourSize + 1; x++)
        {
            for (int y = -m_NeighbourSize; y < m_NeighbourSize + 1; y++)
            {
                for (int z = -m_NeighbourSize; z < m_NeighbourSize + 1; z++)
                {
                    if (x == 0 && y == 0 && z == 0)
                    {
                        center = 1;
                        continue;
                    }
                    tempIndex[0] = index[0] + x;
                    tempIndex[1] = index[1] + y;
                    tempIndex[2] = index[2] + z;
                    int i = (x + m_NeighbourSize) * (crSize*crSize) + (y + m_NeighbourSize) * crSize + z + m_NeighbourSize - center;
                    array[i] = image->GetPixel(tempIndex);
                }
            }
        }
        return true;
    }


    template <class TInputImage, class TInfoImage, class TOutputImage>
    void SurfaceInfoCombineImageFilter<TInputImage, TInfoImage, TOutputImage>::GenerateData()
    {


        InputImageConstPointer inputImage = dynamic_cast<const TInputImage*>(ProcessObject::GetInput(0));
        InfoImageConstPointer  infoImage = dynamic_cast<const TInfoImage*>(ProcessObject::GetInput(1));

        OutputImagePointer outputImage = this->GetOutput();
        outputImage->SetRegions(infoImage->GetLargestPossibleRegion());
        outputImage->Allocate();

        //deep copy image
        DeepCopy<TInfoImage, TOutputImage>(infoImage, outputImage);

        //report progress
        ProgressReporter progress(this, 0,
            inputImage->GetLargestPossibleRegion().GetNumberOfPixels());


        //extract the surface
        int crSize = m_NeighbourSize * 2 + 1;
        const int bufferSize = crSize*crSize*crSize - 1;
        std::vector<InputImagePixelType> A;
        A.resize(bufferSize);
        TInputImage::SizeType size = inputImage->GetLargestPossibleRegion().GetSize();
        int contourPixelCount = 0;
        for (int x = 0; x < size[0]; x++)
        {
            for (int y = 0; y < size[1]; y++)
            {
                for (int z = 0; z < size[2]; z++)
                {
                    for (int i = 0; i < bufferSize; i++)
                    {
                        A[0] = 0;
                    }
                    TInputImage::IndexType index;
                    index[0] = x;
                    index[1] = y;
                    index[2] = z;
                    if (!GetNeighbors(A, inputImage, index))
                    {
                        outputImage->SetPixel(index, m_MinScalar);
                        continue;
                    }
                    int sum = 0;
                    for (int i = 0; i < bufferSize; i++)
                    {
                        sum += A[i];
                    }
                    if (sum == bufferSize* m_Background || sum == bufferSize* m_Foreground)
                    {
                        if (sum == bufferSize * m_Background)
                        {
                            outputImage->SetPixel(index, m_MinScalar);
                        }
                        else
                        {
                            outputImage->SetPixel(index, m_MaxScalar);
                        }
                    }
                    else
                    {
                        double  value = outputImage->GetPixel(index);
                        m_ContourValue += value;
                        m_ContourValueMax = m_ContourValueMax < value ? value : m_ContourValueMax;
                        m_ContourValueMin = m_ContourValueMin > value ? value : m_ContourValueMin;
                        contourPixelCount++;
                        continue;
                    }
                    progress.CompletedPixel();
                }
            }
        }
        m_ContourValue /= contourPixelCount;
    }
}
