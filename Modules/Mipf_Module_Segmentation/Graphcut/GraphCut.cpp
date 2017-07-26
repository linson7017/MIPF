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

#include <algorithm>


void GraphCut::CompareIndexCantainer(
    const IndexContainer& c1, const IndexContainer& c2,
    IndexContainer& increase, IndexContainer& reduce)
{
    for (IndexContainer::iterator it = c2.begin();it != c2.end();it++)
    {
         if (!c1.count(*it))
         {
             increase.insert(*it);
         }
    }
    for (IndexContainer::iterator it = c1.begin();it != c1.end();it++)
    {
        if (!c2.count(*it))
        {
            reduce.insert(*it);
        }
    }

}


GraphCut::GraphCut() :m_graph(NULL)
{
    itk::MetaImageIOFactory::RegisterOneFactory();
    m_lambda = 0.5;
    m_numberOfHistogramBins = 32;
    m_bFirstTime = true;
}


GraphCut::~GraphCut()
{
    delete m_graph;
}

void GraphCut::Release()
{
    delete this;
}


void GraphCut::SetRegion(itk::ImageRegion<3> region)
{
    m_region = region;
}


void GraphCut::Init()
{
    if (m_graph)
    {
        m_graph->reset();
    } 
    m_bFirstTime = true;
    m_preSinks.clear();
    m_preSources.clear();
    m_sinks.clear();
    m_sources.clear();
}


void GraphCut::SetImage(const Float3DImageType* image)
{
    if (!m_bFirstTime)
    {
        return;

    }
    m_image = Float3DImageType::New();
    ITKHelpers::DeepCopy(image, m_image.GetPointer());
    m_region = m_image->GetLargestPossibleRegion();
    ITKHelpers::GetImageScalarRange(m_image.GetPointer(), m_scalarMin, m_scalarMax);


    m_segmentMask = UChar3DImageType::New();
    m_segmentMask->SetRegions(m_region);
    m_segmentMask->SetOrigin(image->GetOrigin());
    m_segmentMask->SetDirection(image->GetDirection());
    m_segmentMask->SetSpacing(image->GetSpacing());
    m_segmentMask->Allocate();

    m_nodeImage = NodeImageType::New();
    m_nodeImage->SetRegions(m_region);
    m_nodeImage->SetOrigin(image->GetOrigin());
    m_nodeImage->SetDirection(image->GetDirection());
    m_nodeImage->SetSpacing(image->GetSpacing());
    m_nodeImage->Allocate();

    m_foregroundHistogram = NULL;
    m_backgroundHistogram = NULL;

    m_foregroundSample = SampleType::New();
    m_backgroundSample = SampleType::New();

    m_foregroundHistogramFilter = SampleToHistogramFilterType::New();
    m_backgroundHistogramFilter = SampleToHistogramFilterType::New();
}


void GraphCut::CutGraph()
{
    //std::cout << "RGBWeight: " << m_rgbWeight << std::endl;
    
    std::cout << "Node in graph:" << m_graph->get_node_num()<<std::endl;
    std::cout << "Arc in graph:" << m_graph->get_arc_num() << std::endl;

    // Compute max-flow

    if (1)
    {
        m_graph->maxflow();
    }
    else
    {
        Block<GraphType::node_id>* changed_list = new Block<GraphType::node_id>(10000);;
        m_graph->maxflow(true, changed_list);
        int x = 0;
    }
    
    m_clock.Stop();
    std::cout << "Max Flow Compute Cost:" << (float)m_clock.GetMean() << std::endl;
   
    UChar3DImageType::PixelType sinkPixel = 0;
    UChar3DImageType::PixelType sourcePixel = 1;

    itk::ImageRegionConstIterator<NodeImageType> nodeImageIterator(m_nodeImage,
        m_region);
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
    m_bFirstTime = false;

    //delete m_graph;
}


void GraphCut::PerformSegmentation()
{
    // This function performs some initializations and then creates and cuts the graph
    std::cout << "/****************Graph Cut Begin!*******************/" << std::endl;
    std::cout << "First Time:" << m_bFirstTime << std::endl;
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
    if (m_bFirstTime)
    {
        itk::ImageRegionIterator<NodeImageType> nodeImageIterator(m_nodeImage, m_nodeImage->GetLargestPossibleRegion());
        nodeImageIterator.GoToBegin();

        while (!nodeImageIterator.IsAtEnd())
        {
            nodeImageIterator.Set(NULL);
            ++nodeImageIterator;
        }
        ITKHelpers::SaveImage<NodeImageType>(m_nodeImage, "D:/temp/nodeImage.mha");
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


void GraphCut::CreateSamples()
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

   int temp = 0;

    for (IndexContainer::iterator it = m_sources.begin();it != m_sources.end();it++)
    {
        if (ITKHelpers::IndexInRegion(*it, m_region))
        {
            m_foregroundSample->PushBack(m_image->GetPixel(*it));
            temp++;
        }
       
    }
    std::cout << "Foreground Samples: " << temp << std::endl;
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

    temp = 0;
    for (IndexContainer::iterator it = m_sinks.begin();it != m_sinks.end();it++)
    {
        if (ITKHelpers::IndexInRegion(*it, m_region))
        {
            m_backgroundSample->PushBack(m_image->GetPixel(*it));
            temp++;
        }  
    }


    std::cout << "Background Samples: " << temp << std::endl;


    m_backgroundHistogramFilter->SetHistogramSize(histogramSize);
    m_backgroundHistogramFilter->SetHistogramBinMinimum(binMinimum);
    m_backgroundHistogramFilter->SetHistogramBinMaximum(binMaximum);
    m_backgroundHistogramFilter->SetAutoMinimumMaximum(false);
    m_backgroundHistogramFilter->SetInput(m_backgroundSample);
    m_backgroundHistogramFilter->Modified();
    m_backgroundHistogramFilter->Update();

    m_backgroundHistogram = m_backgroundHistogramFilter->GetOutput();

}


void GraphCut::CreateGraph()
{
    std::cout << "Region Start: " << m_region.GetIndex(0)<<","
        << m_region.GetIndex(1) << "," 
        << m_region.GetIndex(2) << std::endl;
    std::cout << "Region Size: " << m_region.GetSize(0) << ","
        << m_region.GetSize(1) << ","
        << m_region.GetSize(2) << std::endl;


    // Form the graph
    //TImageType::RegionType imageRegion = m_image->GetBufferedRegion();
    const unsigned int numberOfPixels = m_region.GetNumberOfPixels();
    std::cout << "Number Of Pixels:" << numberOfPixels << std::endl;
    long tempOutput = 0;

    if (!m_graph)
    {
        m_graph = new GraphType(numberOfPixels, m_region.GetImageDimension() * numberOfPixels);
    }
    
     if (m_bFirstTime)
     {
         m_graph->reset();
         m_preSinks.clear();
         m_preSources.clear();
         // Add all of the nodes to the graph and store their IDs in a "node image"
         itk::ImageRegionIterator<NodeImageType> nodeImageIterator(m_nodeImage, m_region);
         nodeImageIterator.GoToBegin();
         while (!nodeImageIterator.IsAtEnd())
         {
             nodeImageIterator.Set(m_graph->add_node());
             ++nodeImageIterator;
             tempOutput++;
         }
         m_clock.Stop();
         std::cout << "Add Node Cost:" << (float)m_clock.GetMean() << std::endl;
         std::cout << "Node Number:" << tempOutput << std::endl;
         tempOutput = 0;
         m_clock.Start();

         // Estimate the "camera noise"
         m_sigma = this->ComputeNoise();
         std::cout << "Sigma:" << m_sigma << std::endl;
         m_clock.Stop();
         std::cout << "Compute Noise Cost:" << (float)m_clock.GetMean() << std::endl;
         m_clock.Start();

         //add edges
         AddEdges();

         CreateSamples();
         m_clock.Stop();
         std::cout << "Create Samples Cost:" << (float)m_clock.GetMean() << std::endl;
         m_clock.Start();
    }  


    ////////// Add t-edges and set t-edge weights (links from image nodes to virtual background and virtual foreground node) //////////

    // Compute the histograms of the selected foreground and background pixels
    if (m_bFirstTime)
    {
        itk::ImageRegionIterator<Float3DImageType> imageIterator(m_image, m_region);
        itk::ImageRegionIterator<NodeImageType> nodeIterator(m_nodeImage, m_region);
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

            HistogramType::MeasurementVectorType measurementVector(Pixel1D);
            for (unsigned int i = 0; i < Pixel1D; i++)
            {
                measurementVector[i] = pixel;
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
            if (sinkHistogramValue > sourceHistogramValue)
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
            tempOutput++;
        }
    }

    IndexContainer incress;
    IndexContainer reduce;
    incress.clear();
    reduce.clear();
    std::cout << "Sources Number:" << m_sources.size() << std::endl;
    std::cout << "Sinks Number:" << m_sinks.size() << std::endl;
    std::cout << "Pre Sources Number:" << m_preSources.size() << std::endl;
    std::cout << "Pre Sinks Number:" << m_preSinks.size() << std::endl;

    //object
    CompareIndexCantainer(m_preSources, m_sources, incress, reduce);
    for (IndexContainer::iterator it = incress.begin();it != incress.end();it++)
    {
        if (ITKHelpers::IndexInRegion(*it, m_nodeImage->GetBufferedRegion()))
        {
            m_graph->add_tweights(m_nodeImage->GetPixel(*it), m_lambda * std::numeric_limits<float>::max(), 0);
            if (!m_bFirstTime)
            {
                m_graph->mark_node(m_nodeImage->GetPixel(*it));
            }
        }
        
    }
    for (IndexContainer::iterator it = reduce.begin();it != reduce.end();it++)
    {
        if (ITKHelpers::IndexInRegion(*it, m_nodeImage->GetBufferedRegion()))
        {
            m_graph->add_tweights(m_nodeImage->GetPixel(*it), -m_lambda * std::numeric_limits<float>::max(), 0);
            if (!m_bFirstTime)
            {
                m_graph->mark_node(m_nodeImage->GetPixel(*it));
            }
        }
    }

    incress.clear();
    reduce.clear();
    //background
    CompareIndexCantainer(m_preSinks, m_sinks, incress, reduce);
    // Set very high sink weights for the pixels which were selected as background by the user
    for (IndexContainer::iterator it = incress.begin();it != incress.end();it++)
    {
        if (ITKHelpers::IndexInRegion(*it, m_nodeImage->GetBufferedRegion()))
        {
            m_graph->add_tweights(m_nodeImage->GetPixel(*it), 0, m_lambda * std::numeric_limits<float>::max());
            if (!m_bFirstTime)
            {
                m_graph->mark_node(m_nodeImage->GetPixel(*it));
            }
        }      
    }
    for (IndexContainer::iterator it = reduce.begin();it != reduce.end();it++)
    {
        if (ITKHelpers::IndexInRegion(*it, m_nodeImage->GetBufferedRegion()))
        {
            m_graph->add_tweights(m_nodeImage->GetPixel(*it), 0, -m_lambda * std::numeric_limits<float>::max());
            if (!m_bFirstTime)
            {
                m_graph->mark_node(m_nodeImage->GetPixel(*it));
            }
        }
    }

    m_preSources = m_sources;
    m_preSinks = m_sinks;

    m_clock.Stop();
    std::cout << "Add Twieght Cost:" << (float)m_clock.GetMean() << std::endl;
    std::cout << "Twieght Number:" << tempOutput << std::endl;
    tempOutput = 0;
    m_clock.Start();
}


void GraphCut::AddEdges()
{
    ////////// Create n-edges and set n-edge weights (links between image nodes) //////////

    // We are only using a 4-connected structure, so the kernel (iteration neighborhood) must only be 3x3 (specified by a radius of 1)
    itk::Size<3> radius;
    radius.Fill(1);

    typedef itk::ShapedNeighborhoodIterator<Float3DImageType> IteratorType;

    // Traverse the image adding an edge between the current pixel and the pixel below it and the current pixel and the pixel to the right of it.
    // This prevents duplicate edges (i.e. we cannot add an edge to all 4-connected neighbors of every pixel or almost every edge would be duplicated.
    std::vector<typename IteratorType::OffsetType> neighbors;
    typename IteratorType::OffsetType bottom = { { 0,1,0 } };
    neighbors.push_back(bottom);
    typename IteratorType::OffsetType right = { { 1,0,0 } };
    neighbors.push_back(right);
    typename IteratorType::OffsetType back = { { 0,0,1 } };
    neighbors.push_back(back);

    typename IteratorType::OffsetType center = { { 0,0,0 } };

    IteratorType iterator(radius, m_image, m_region);
    iterator.ClearActiveList();
    iterator.ActivateOffset(bottom);
    iterator.ActivateOffset(right);
    iterator.ActivateOffset(back);
    iterator.ActivateOffset(center);

    int pixelNum = 0;
    int tempOut = 0;
    for (iterator.GoToBegin(); !iterator.IsAtEnd(); ++iterator)
    {
        PixelType centerPixel = iterator.GetPixel(center);
        pixelNum++;
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
            float weight = exp(-pow(pixelDifference, 2) / (2.0*m_sigma*m_sigma));
            assert(weight >= 0);

            // Add the edge to the graph
            int node1 = m_nodeImage->GetPixel(iterator.GetIndex(center));
            int node2 = m_nodeImage->GetPixel(iterator.GetIndex(neighbors[i]));
            m_graph->add_edge(node1, node2, weight, weight);
            tempOut++;
        }
    }
    m_clock.Stop();
    std::cout << "Add Edge Cost:" << (float)m_clock.GetMean() << std::endl;
    std::cout << "Edge Pixel Number:" << pixelNum << std::endl;
    std::cout << "Edge Number:" << tempOut << std::endl;
    m_clock.Start();
}


double GraphCut::ComputeNoise()
{
    // Compute an estimate of the "camera noise". This is used in the N-weight function.

    // Since we use a 4-connected neighborhood, the kernel must be 3x3 (a rectangular radius of 1 creates a kernel side length of 3)
    itk::Size<3> radius;
    radius[0] = 1;
    radius[1] = 1;
    radius[2] = 1;

    typedef itk::ShapedNeighborhoodIterator<Float3DImageType> IteratorType;

    std::vector<typename IteratorType::OffsetType> neighbors;
    typename IteratorType::OffsetType bottom = { { 0,1,0 } };
    neighbors.push_back(bottom);
    typename IteratorType::OffsetType right = { { 1,0,0 } };
    neighbors.push_back(right);
    typename IteratorType::OffsetType back = { { 0,0,1 } };
    neighbors.push_back(back);

    typename IteratorType::OffsetType center = { { 0,0,0 } };

    IteratorType iterator(radius, m_image, m_region);
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



void GraphCut::SetLambda(const float lambda)
{
    m_lambda = lambda;
}


void GraphCut::SetNumberOfHistogramBins(int bins)
{
    m_numberOfHistogramBins = bins;
}


UChar3DImageType* GraphCut::GetSegmentMask()
{
    return m_segmentMask;
}


IndexContainer GraphCut::GetSinks()
{
    return m_sinks;
}



IndexContainer GraphCut::GetSources()
{
    return m_sources;
}


void GraphCut::SetSources(const IndexContainer& sources)
{
    m_sources = sources;
}


void GraphCut::SetSinks(const IndexContainer& sinks)
{
    m_sinks = sinks;
}


Float3DImageType* GraphCut::GetImage()
{
    return m_image;
}
