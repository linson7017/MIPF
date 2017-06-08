/********************************************************************
	FileName:    GraphCut.h
	Author:        Ling Song
	Date:           Month 5 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef GraphCut_h__
#define GraphCut_h__
#pragma once
#include "PixelDifference.h"
#include "ITKImageTypeDef.h"
//itk
#include <itkImage.h>
#include "itkSampleToHistogramFilter.h"
#include "itkHistogram.h"
#include "itkListSample.h"
#include "itkTimeProbe.h"
#include <vector>

//maxflow
#include "maxflow/graph.h"
#include "Mask.h"



typedef Graph<double,double,double> GraphType;

template <typename TImageType = Float3DImageType, typename TPixelDifferenceFunc = RGBPixelDifference<typename TImageType::PixelType>>
class GraphCut
{
public:
    typedef itk::Statistics::Histogram< float,itk::Statistics::DenseFrequencyContainer2 > HistogramType;
    typedef std::vector<itk::Index<3> > IndexContainer;
    typedef itk::Image<int, 3> NodeImageType;

    TPixelDifferenceFunc m_pixelDifferenceFunc;

    GraphCut();
    ~GraphCut();

    void PerformSegmentation();

    void SetImage(const TImageType* image);
    TImageType* GetImage();

    void SetSources(const IndexContainer& sources);
    void SetSinks(const IndexContainer& sinks);
    IndexContainer GetSources();
    IndexContainer GetSinks();


    UChar3DImageType* GetSegmentMask();

    void SetLambda(const float);
    void SetNumberOfHistogramBins(const int);

    void SetScalarRange(double min, double max) 
    { 
        m_scalarMin = min;m_scalarMax = max; 
    }

private:
    GraphType* m_graph;

    UChar3DImageType::Pointer m_segmentMask;

    IndexContainer m_sources;
    IndexContainer m_sinks;
    float m_lambda;
    int m_numberOfHistogramBins;

    NodeImageType::Pointer m_nodeImage;
    


    typedef typename TImageType::PixelType PixelType;
    typedef itk::Statistics::ListSample<PixelType> SampleType;
    typedef itk::Statistics::SampleToHistogramFilter<SampleType, HistogramType> SampleToHistogramFilterType;


    float m_rgbWeight;
    
    void CreateSamples();

    double ComputeNoise();

    void CreateGraph();

    void CutGraph();

    typename SampleType::Pointer m_foregroundSample;
    typename SampleType::Pointer m_backgroundSample;

    const HistogramType* m_foregroundHistogram;
    const HistogramType* m_backgroundHistogram;

    typename SampleToHistogramFilterType::Pointer m_foregroundHistogramFilter;
    typename SampleToHistogramFilterType::Pointer m_backgroundHistogramFilter;

    typename TImageType::Pointer m_image;

    double m_scalarMax;
    double m_scalarMin;

    itk::TimeProbe m_clock;
};

#include "GraphCut.hpp"

#endif // GraphCut_h__
