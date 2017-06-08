#ifndef GraphCut_hpp__
#define GraphCut_hpp__

#include "GraphCut.h"
#include "ITK_Helpers.h"
// ITK
#include "itkImageRegionIterator.h"
#include "itkShapedNeighborhoodIterator.h"
#include "itkMaskImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkMetaImageIOFactory.h"
#include "itkTimeProbe.h"

#include <cmath>

template <typename TImageType, typename TPixelDifferenceFunc>
GraphCut<TImageType, TPixelDifferenceFunc>::GraphCut()
{
    itk::MetaImageIOFactory::RegisterOneFactory();
}

template <typename TImageType, typename TPixelDifferenceFunc>
GraphCut<TImageType, TPixelDifferenceFunc>::~GraphCut()
{
}

template <typename TImageType, typename TPixelDifferenceFunc>
void GraphCut<TImageType, TPixelDifferenceFunc>::SetImage(const TImageType* image)
{
    m_image = TImageType::New();
    ITKHelpers::DeepCopy(image, m_image.GetPointer());

    m_segmentMask = Mask::New();
    m_segmentMask->SetRegions(m_image->GetLargestPossibleRegion());
    m_segmentMask->Allocate();

    m_nodeImage = NodeImageType::New();
    m_nodeImage->SetRegions(m_image->GetLargestPossibleRegion());
    m_nodeImage->Allocate();

    m_lambda = 0.5;
    m_numberOfHistogramBins = 32; 

    m_foregroundHistogram = NULL;
    m_backgroundHistogram = NULL;

    m_foregroundSample = SampleType::New();
    m_backgroundSample = SampleType::New();

    m_foregroundHistogramFilter = SampleToHistogramFilterType::New();
    m_backgroundHistogramFilter = SampleToHistogramFilterType::New();

    m_rgbWeight = 0.5; // This value is never used - it is set from the slider
}

template <typename TImageType, typename TPixelDifferenceFunc>
void GraphCut<TImageType, TPixelDifferenceFunc>::CutGraph()
{
    //std::cout << "RGBWeight: " << m_rgbWeight << std::endl;
    

    // Compute max-flow
    m_graph->maxflow();
    m_clock.Stop();
    std::cout << "Max Flow Compute Cost:" << (float)m_clock.GetMean() << std::endl;

   
    Mask::PixelType sinkPixel = 0;
    Mask::PixelType sourcePixel = 255;

    itk::ImageRegionConstIterator<NodeImageType> nodeImageIterator(m_nodeImage,
        m_nodeImage->GetLargestPossibleRegion());
    nodeImageIterator.GoToBegin();

    while (!nodeImageIterator.IsAtEnd())
    {
        if (m_graph->what_segment(nodeImageIterator.Get()) == GraphType::SOURCE)
        {
            m_segmentMask->SetPixel(nodeImageIterator.GetIndex(), sourcePixel);
        }
        else if (m_graph->what_segment(nodeImageIterator.Get()) == GraphType::SINK)
        {
            m_segmentMask->SetPixel(nodeImageIterator.GetIndex(), sinkPixel);
        }
        ++nodeImageIterator;
    }
   

    delete m_graph;
}

template <typename TImageType, typename TPixelDifferenceFunc>
void GraphCut<TImageType, TPixelDifferenceFunc>::PerformSegmentation()
{
    // This function performs some initializations and then creates and cuts the graph
    std::cout << "/****************Graph Cut Begin!*******************/" << std::endl;
    std::cout << "Lambda: " << m_lambda << std::endl;
    std::cout << "Number Of Histogram Bins: " << m_numberOfHistogramBins << std::endl;
    std::cout << "Source Points Size: " << m_sources.size() << std::endl;
    std::cout << "Sink Points Size: " << m_sinks.size() << std::endl;
    m_clock.Reset();
    m_clock.Start();

    // Ensure at least one pixel has been specified for both the foreground and background
    if ((m_sources.size() <= 0) || (m_sinks.size() <= 0))
    {
        std::cerr << "At least one source (foreground) pixel and one sink (background) pixel must be specified!" << std::endl;
        std::cerr << "Currently there are " << m_sources.size() << " and " << m_sinks.size() << " sinks." << std::endl;
        return;
    }

    // Blank the NodeImage
    itk::ImageRegionIterator<NodeImageType> nodeImageIterator(m_nodeImage, m_nodeImage->GetLargestPossibleRegion());
    nodeImageIterator.GoToBegin();

    while (!nodeImageIterator.IsAtEnd())
    {
        nodeImageIterator.Set(NULL);
        ++nodeImageIterator;
    }

    // Blank the output image
    //itk::ImageRegionIterator<GrayscaleImageType> segmentMaskImageIterator(m_segmentMask,
    //                                             m_segmentMask->GetLargestPossibleRegion());
    itk::ImageRegionIterator<UChar3DImageType> segmentMaskImageIterator(m_segmentMask,
        m_segmentMask->GetLargestPossibleRegion());
    segmentMaskImageIterator.GoToBegin();

    UChar3DImageType::PixelType empty = 0;
    //empty[0] = 0;

    while (!segmentMaskImageIterator.IsAtEnd())
    {
        segmentMaskImageIterator.Set(empty);
        ++segmentMaskImageIterator;
    }
    m_clock.Stop();
    std::cout << "Helper Image Build Cost:" << (float)m_clock.GetMean() << std::endl;
    m_clock.Start();

    this->CreateGraph();
    this->CutGraph();
    
}

template <typename TImageType, typename TPixelDifferenceFunc>
void GraphCut<TImageType, TPixelDifferenceFunc>::CreateSamples()
{
    // This function creates ITK samples from the scribbled pixels and then computes the foreground and background histograms

    // We want the histogram bins to take values from 0 to 255 in all dimensions
    HistogramType::MeasurementVectorType binMinimum(m_image->GetNumberOfComponentsPerPixel());
    HistogramType::MeasurementVectorType binMaximum(m_image->GetNumberOfComponentsPerPixel());
    for (unsigned int i = 0; i < m_image->GetNumberOfComponentsPerPixel(); i++)
    {
        binMinimum[i] = m_scalarMin;
        binMaximum[i] = m_scalarMax;
    }

    // Setup the histogram size
    std::cout << "Image components per pixel: " << m_image->GetNumberOfComponentsPerPixel() << std::endl;
    typename SampleToHistogramFilterType::HistogramSizeType histogramSize(m_image->GetNumberOfComponentsPerPixel());
    histogramSize.Fill(m_numberOfHistogramBins);

    // Create foreground samples and histogram
   m_foregroundSample->Clear();
   m_foregroundSample->SetMeasurementVectorSize(m_image->GetNumberOfComponentsPerPixel());
    //std::cout << "Measurement vector size: " <<m_foregroundSample->GetMeasurementVectorSize() << std::endl;
    //std::cout << "Pixel size: " << m_image->GetPixel(m_sources[0]).GetNumberOfElements() << std::endl;

    for (unsigned int i = 0; i < m_sources.size(); i++)
    {
       m_foregroundSample->PushBack(m_image->GetPixel(m_sources[i]));
    }

    m_foregroundHistogramFilter->SetHistogramSize(histogramSize);
    m_foregroundHistogramFilter->SetHistogramBinMinimum(binMinimum);
    m_foregroundHistogramFilter->SetHistogramBinMaximum(binMaximum);
    m_foregroundHistogramFilter->SetAutoMinimumMaximum(false);
    m_foregroundHistogramFilter->SetInput(m_foregroundSample);
    m_foregroundHistogramFilter->Modified();
    m_foregroundHistogramFilter->Update();

    m_foregroundHistogram = m_foregroundHistogramFilter->GetOutput();

    // Create background samples and histogram
    m_backgroundSample->Clear();
    m_backgroundSample->SetMeasurementVectorSize(m_image->GetNumberOfComponentsPerPixel());
    for (unsigned int i = 0; i < m_sinks.size(); i++)
    {
        m_backgroundSample->PushBack(m_image->GetPixel(m_sinks[i]));
    }

    m_backgroundHistogramFilter->SetHistogramSize(histogramSize);
    m_backgroundHistogramFilter->SetHistogramBinMinimum(binMinimum);
    m_backgroundHistogramFilter->SetHistogramBinMaximum(binMaximum);
    m_backgroundHistogramFilter->SetAutoMinimumMaximum(false);
    m_backgroundHistogramFilter->SetInput(m_backgroundSample);
    m_backgroundHistogramFilter->Modified();
    m_backgroundHistogramFilter->Update();

    m_backgroundHistogram = m_backgroundHistogramFilter->GetOutput();

}

template <typename TImageType, typename TPixelDifferenceFunc>
void GraphCut<TImageType, TPixelDifferenceFunc>::CreateGraph()
{
    // Form the graph
    TImageType::RegionType imageRegion = m_image->GetBufferedRegion();
    const unsigned int numberOfPixels = imageRegion.GetNumberOfPixels();
    

    m_graph = new GraphType(numberOfPixels, 2*numberOfPixels);

    // Add all of the nodes to the graph and store their IDs in a "node image"
    itk::ImageRegionIterator<NodeImageType> nodeImageIterator(m_nodeImage, m_nodeImage->GetLargestPossibleRegion());
    nodeImageIterator.GoToBegin();

    while (!nodeImageIterator.IsAtEnd())
    {
        nodeImageIterator.Set(m_graph->add_node());
        ++nodeImageIterator;
    }
    m_clock.Stop();
    std::cout << "Add Node Cost:" << (float)m_clock.GetMean() << std::endl;
    m_clock.Start();

    // Estimate the "camera noise"
    double sigma = this->ComputeNoise();
    m_clock.Stop();
    std::cout << "Compute Noise Cost:" << (float)m_clock.GetMean() << std::endl;
    m_clock.Start();

    ////////// Create n-edges and set n-edge weights (links between image nodes) //////////

    // We are only using a 4-connected structure, so the kernel (iteration neighborhood) must only be 3x3 (specified by a radius of 1)
    itk::Size<3> radius;
    radius.Fill(1);

    typedef itk::ShapedNeighborhoodIterator<TImageType> IteratorType;

    // Traverse the image adding an edge between the current pixel and the pixel below it and the current pixel and the pixel to the right of it.
    // This prevents duplicate edges (i.e. we cannot add an edge to all 4-connected neighbors of every pixel or almost every edge would be duplicated.
    std::vector<typename IteratorType::OffsetType> neighbors;
    typename IteratorType::OffsetType bottom = { { 0,1,0 } };
    neighbors.push_back(bottom);
    typename IteratorType::OffsetType right = { { 1,0,0 } };
    neighbors.push_back(right);
    typename IteratorType::OffsetType back = { { 0,0,1 } };
    neighbors.push_back(back);

    typename IteratorType::OffsetType center = { { 0,0 } };

    IteratorType iterator(radius, m_image, m_image->GetLargestPossibleRegion());
    iterator.ClearActiveList();
    iterator.ActivateOffset(bottom);
    iterator.ActivateOffset(right);
    iterator.ActivateOffset(back);
    iterator.ActivateOffset(center);

    for (iterator.GoToBegin(); !iterator.IsAtEnd(); ++iterator)
    {
        PixelType centerPixel = iterator.GetPixel(center);

        for (unsigned int i = 0; i < neighbors.size(); i++)
        {
            bool valid;
            iterator.GetPixel(neighbors[i], valid);

            // If the current neighbor is outside the image, skip it
            if (!valid)
            {
                continue;
            }
            PixelType neighborPixel = iterator.GetPixel(neighbors[i]);

            // Compute the Euclidean distance between the pixel intensities
            float pixelDifference = m_pixelDifferenceFunc.Difference(centerPixel, neighborPixel);

            // Compute the edge weight
            float weight = exp(-pow(pixelDifference, 2) / (2.0*sigma*sigma));
            assert(weight >= 0);

            // Add the edge to the graph
            int node1 = m_nodeImage->GetPixel(iterator.GetIndex(center));
            int node2 = m_nodeImage->GetPixel(iterator.GetIndex(neighbors[i]));
            m_graph->add_edge(node1, node2, weight, weight);
        }
    }
    m_clock.Stop();
    std::cout << "Add Edge Cost:" << (float)m_clock.GetMean() << std::endl;
    m_clock.Start();

    ////////// Add t-edges and set t-edge weights (links from image nodes to virtual background and virtual foreground node) //////////

    // Compute the histograms of the selected foreground and background pixels
    CreateSamples();
    m_clock.Stop();
    std::cout << "Create Samples Cost:" << (float)m_clock.GetMean() << std::endl;
    m_clock.Start();

    itk::ImageRegionIterator<TImageType> imageIterator(m_image, m_image->GetLargestPossibleRegion());
    itk::ImageRegionIterator<NodeImageType> nodeIterator(m_nodeImage, m_nodeImage->GetLargestPossibleRegion());
    imageIterator.GoToBegin();
    nodeIterator.GoToBegin();

    // Since the t-weight function takes the log of the histogram value,
    // we must handle bins with frequency = 0 specially (because log(0) = -inf)
    // For empty histogram bins we use tinyValue instead of 0.
    float tinyValue = 1e-10;

    while (!imageIterator.IsAtEnd())
    {
        PixelType pixel = imageIterator.Get();
        //std::cout << "Pixels have size: " << pixel.Size() << std::endl;
       
        HistogramType::MeasurementVectorType measurementVector(pixel.Size());
        for (unsigned int i = 0; i < pixel.Size(); i++)
        {
            measurementVector[i] = pixel[i];
        }

        HistogramType::IndexType backgroundIndex;
        m_backgroundHistogram->GetIndex(measurementVector, backgroundIndex);
        float sinkHistogramValue = m_backgroundHistogram->GetFrequency(backgroundIndex);

        HistogramType::IndexType foregroundIndex;
        m_foregroundHistogram->GetIndex(measurementVector, foregroundIndex);
        float sourceHistogramValue = m_foregroundHistogram->GetFrequency(foregroundIndex);

        // Conver the histogram value/frequency to make it as if it came from a normalized histogram
        sinkHistogramValue /= m_backgroundHistogram->GetTotalFrequency();
        sourceHistogramValue /= m_foregroundHistogram->GetTotalFrequency();

        if (sinkHistogramValue <= 0)
        {
            sinkHistogramValue = tinyValue;
        }
        if (sourceHistogramValue <= 0)
        {
            sourceHistogramValue = tinyValue;
        }

        // Add the edge to the graph and set its weight
        if (sinkHistogramValue>sourceHistogramValue)
        {
            int x = 0;
        }
        else
        {
            int x = 0;
        }
        m_graph->add_tweights(nodeIterator.Get(),
            -m_lambda*log(sinkHistogramValue),
            -m_lambda*log(sourceHistogramValue)); // log() is the natural log
        ++imageIterator;
        ++nodeIterator;
    }

    // Set very high source weights for the pixels which were selected as foreground by the user
    for (unsigned int i = 0; i < m_sources.size(); i++)
    {
        m_graph->add_tweights(m_nodeImage->GetPixel(m_sources[i]), m_lambda * std::numeric_limits<float>::max(), 0);
    }

    // Set very high sink weights for the pixels which were selected as background by the user
    for (unsigned int i = 0; i < m_sinks.size(); i++)
    {
        m_graph->add_tweights(m_nodeImage->GetPixel(m_sinks[i]), 0, m_lambda * std::numeric_limits<float>::max());
    }

    m_clock.Stop();
    std::cout << "Add Twieght Cost:" << (float)m_clock.GetMean() << std::endl;
    m_clock.Start();
}

template <typename TImageType, typename TPixelDifferenceFunc>
double GraphCut<TImageType, TPixelDifferenceFunc>::ComputeNoise()
{
    // Compute an estimate of the "camera noise". This is used in the N-weight function.

    // Since we use a 4-connected neighborhood, the kernel must be 3x3 (a rectangular radius of 1 creates a kernel side length of 3)
    itk::Size<3> radius;
    radius[0] = 1;
    radius[1] = 1;
    radius[2] = 1;

    typedef itk::ShapedNeighborhoodIterator<TImageType> IteratorType;

    std::vector<typename IteratorType::OffsetType> neighbors;
    typename IteratorType::OffsetType bottom = { { 0,1,0 } };
    neighbors.push_back(bottom);
    typename IteratorType::OffsetType right = { { 1,0,0 } };
    neighbors.push_back(right);
    typename IteratorType::OffsetType back = { { 0,0,1 } };
    neighbors.push_back(back);

    typename IteratorType::OffsetType center = { { 0,0,0 } };

    IteratorType iterator(radius, m_image, m_image->GetLargestPossibleRegion());
    iterator.ClearActiveList();
    iterator.ActivateOffset(bottom);
    iterator.ActivateOffset(right);
    iterator.ActivateOffset(back);
    iterator.ActivateOffset(center);

    double sigma = 0.0;
    int numberOfEdges = 0;

    // Traverse the image collecting the differences between neighboring pixel intensities
    for (iterator.GoToBegin(); !iterator.IsAtEnd(); ++iterator)
    {
        PixelType centerPixel = iterator.GetPixel(center);

        for (unsigned int i = 0; i < neighbors.size(); i++)
        {
            bool valid;
            iterator.GetPixel(neighbors[i], valid);
            if (!valid)
            {
                continue;
            }
            PixelType neighborPixel = iterator.GetPixel(neighbors[i]);

            float colorDifference = m_pixelDifferenceFunc.Difference(centerPixel, neighborPixel);
            sigma += colorDifference;
            numberOfEdges++;
        }

    }

    // Normalize
    sigma /= static_cast<double>(numberOfEdges);

    return sigma;
}

template <typename TImageType, typename TPixelDifferenceFunc>
typename GraphCut<TImageType, TPixelDifferenceFunc>::IndexContainer GraphCut<TImageType, TPixelDifferenceFunc>::GetSources()
{
    return m_sources;
}

template <typename TImageType, typename TPixelDifferenceFunc>
void GraphCut<TImageType, TPixelDifferenceFunc>::SetLambda(const float lambda)
{
    m_lambda = lambda;
}

template <typename TImageType, typename TPixelDifferenceFunc>
void GraphCut<TImageType, TPixelDifferenceFunc>::SetNumberOfHistogramBins(int bins)
{
    m_numberOfHistogramBins = bins;
}

template <typename TImageType, typename TPixelDifferenceFunc>
UChar3DImageType* GraphCut<TImageType, TPixelDifferenceFunc>::GetSegmentMask()
{
    return m_segmentMask;
}

template <typename TImageType, typename TPixelDifferenceFunc>
typename GraphCut<TImageType, TPixelDifferenceFunc>::IndexContainer GraphCut<TImageType, TPixelDifferenceFunc>::GetSinks()
{
    return m_sinks;
}

template <typename TImageType, typename TPixelDifferenceFunc>
void GraphCut<TImageType, TPixelDifferenceFunc>::SetSources(const IndexContainer& sources)
{
    m_sources = sources;
}

template <typename TImageType, typename TPixelDifferenceFunc>
void GraphCut<TImageType, TPixelDifferenceFunc>::SetSinks(const IndexContainer& sinks)
{
    m_sinks = sinks;
}

template <typename TImageType, typename TPixelDifferenceFunc>
TImageType* GraphCut<TImageType, TPixelDifferenceFunc>::GetImage()
{
    return m_image;
}
#endif // GraphCut_hpp__