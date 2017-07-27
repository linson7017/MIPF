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


typedef Graph<float,float, float> GraphType;

template <typename TImageType = Float3DImageType, typename TPixelDifferenceFunc = GrayPixelDifference<typename TImageType::PixelType>>
class GraphCut
{
public:
    typedef itk::Statistics::Histogram< float,itk::Statistics::DenseFrequencyContainer2 > HistogramType;
    typedef std::set<itk::Index<3>, IndexSortCriterion > IndexContainer;
    typedef itk::Image<int, 3> NodeImageType;

    TPixelDifferenceFunc m_pixelDifferenceFunc;

    GraphCut();
    ~GraphCut();

    void PerformSegmentation();

    void Init();

    void SetImage(const TImageType* image);
    TImageType* GetImage();

    void SetSources(const IndexContainer& sources);
    void SetSinks(const IndexContainer& sinks);
    IndexContainer GetSources();
    IndexContainer GetSinks();

    void SetRegion(itk::ImageRegion<3> region);

    void SetCaculateMask(UChar3DImageType* mask);


    UChar3DImageType* GetSegmentMask();

    void SetLambda(const float);
    void SetNumberOfHistogramBins(const int);

    void SetFirstTIme(bool firstTime) { m_bFirstTime = firstTime; }
    bool GetFirstTime() { return m_bFirstTime; }

    void SetScalarRange(double min, double max) 
    { 
        m_scalarMin = min;m_scalarMax = max; 
    }

private:


    GraphType* m_graph;

    UChar3DImageType::Pointer m_segmentMask;
    UChar3DImageType::Pointer m_caculateMask;
    itk::ImageRegion<3> m_region;

    IndexContainer m_sources;
    IndexContainer m_sinks;

    IndexContainer m_preSources;
    IndexContainer m_preSinks;

    float m_lambda;
    int m_numberOfHistogramBins;

    NodeImageType::Pointer m_nodeImage;
    

    typedef typename TImageType::PixelType PixelType;
    typedef typename  itk::FixedArray< PixelType, Pixel1D > VectorPixelType;
    typedef itk::Statistics::ListSample<VectorPixelType> SampleType;
    typedef itk::Statistics::SampleToHistogramFilter<SampleType, HistogramType> SampleToHistogramFilterType;


    float m_rgbWeight;
    
    void CreateSamples();

    double ComputeNoise();

    void CreateGraph();

    void AddEdges();

    void CutGraph();

    void CompareIndexCantainer(const IndexContainer& c1, const IndexContainer& c2,IndexContainer& increase,IndexContainer& reduce);

    typename SampleType::Pointer m_foregroundSample;
    typename SampleType::Pointer m_backgroundSample;

    const HistogramType* m_foregroundHistogram;
    const HistogramType* m_backgroundHistogram;

    typename SampleToHistogramFilterType::Pointer m_foregroundHistogramFilter;
    typename SampleToHistogramFilterType::Pointer m_backgroundHistogramFilter;

    typename TImageType::Pointer m_image;

    double m_scalarMax;
    double m_scalarMin;
    double m_lowerThreshold;
    double m_higherThreshold;
    double m_sigma;

    itk::TimeProbe m_clock;

    bool m_bFirstTime;
};

#include "GraphCut.hpp"

#endif // GraphCut_h__
