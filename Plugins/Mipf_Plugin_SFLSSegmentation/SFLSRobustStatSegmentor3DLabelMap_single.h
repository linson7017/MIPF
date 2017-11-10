#ifndef SFLSRobustStatSegmentor3DLabelMap_single_h_
#define SFLSRobustStatSegmentor3DLabelMap_single_h_

#include "SFLSSegmentor3D.h"

#include <list>
#include <vector>
#include <QObject>


class DSFSSegmentorNotifer :public QObject
{
    Q_OBJECT
public:
    DSFSSegmentorNotifer(){}
    ~DSFSSegmentorNotifer() {}
    virtual void DoSegmentationExt() = 0;
    virtual void StopSegmentationExt() = 0;
public :
    void SlotDoSegmentation() { DoSegmentationExt(); }
    void SlotStopSegmentation() { StopSegmentationExt(); }

signals:
    void SignalInteractionEnd(const itk::Image<float,3>::Pointer& image,unsigned int currentInteraction);
    void SignalSegmentationFinished();
};

template <typename TPixel>
class CSFLSRobustStatSegmentor3DLabelMap :public DSFSSegmentorNotifer, public CSFLSSegmentor3D<TPixel>
{
public:
    typedef CSFLSSegmentor3D<TPixel> SuperClassType;

    typedef CSFLSRobustStatSegmentor3DLabelMap<TPixel> Self;

    typedef typename SuperClassType::NodeType   NodeType;
    typedef typename SuperClassType::CSFLSLayer CSFLSLayer;

    CSFLSRobustStatSegmentor3DLabelMap() : CSFLSSegmentor3D<TPixel>()
    {
        basicInit();
    }

    virtual void DoSegmentationExt() override
    { 
        std::cout << "Start Segmentation !" << std::endl;
        doSegmenation(); 
    }
    virtual void StopSegmentationExt() override
    {
        std::cout << "Stop Segmentation !" << std::endl;
        m_bStopSegmentation = true;
    }

    void basicInit();

    typedef typename SuperClassType::TShortImage TLabelImage;
    typedef typename TLabelImage::Pointer       TLabelImagePointer;

    typedef typename SuperClassType::TFloatImage TFloatImage;
    typedef typename TFloatImage::Pointer        TFloatImagePointer;

    typedef typename SuperClassType::MaskImageType TMaskImage;

    typedef typename SuperClassType::TIndex  TIndex;
    typedef typename SuperClassType::TSize   TSize;
    typedef typename SuperClassType::TRegion TRegion;

    void setInputLabelImage(TLabelImagePointer l);

    void doSegmenation();

    void computeForce();

    void setKernelWidthFactor(double f);

    void setIntensityHomogeneity(double h);
protected:

    TLabelImagePointer              m_inputLabelImage;
    std::vector<std::vector<long> > m_seeds; // in IJK

    std::vector<std::vector<double> > m_featureAtTheSeeds;

    long m_statNeighborX;
    long m_statNeighborY;
    long m_statNeighborZ;

    const static long m_numberOfFeature = 3;

    TLabelImagePointer              m_featureComputed; // if feature at this point is computed, then is 1
    std::vector<TFloatImagePointer> m_featureImageList;

    double m_kernelWidthFactor; // kernel_width = empirical_std/m_kernelWidthFactor, Eric has it at 10.0

    void initFeatureComputedImage();

    void initFeatureImage();

    void computeFeatureAt(TIndex idx, std::vector<double>& f);

    void getRobustStatistics(std::vector<double>& samples, std::vector<double>& robustStat);

    void inputLableImageToSeeds();

    void seedToMask();

    void dialteSeeds();

    void getFeatureAroundSeeds();

    void estimateFeatureStdDevs();

    TPixel m_inputImageIntensityMin;
    TPixel m_inputImageIntensityMax;
    void computeMinMax();

    // each feature corresponds to a inner std::vector<double>
    std::vector<std::vector<double> > m_PDFlearnedFromSeeds;
    void estimatePDFs();

    void getThingsReady();

    std::vector<double> m_kernelStddev;
    double kernelEvaluation(const std::vector<double>& newFeature);

    double kernelEvaluationUsingPDF(const std::vector<double>& newFeature);

    bool m_bStopSegmentation;
};

#include "SFLSRobustStatSegmentor3DLabelMap_single.cpp"

#endif //SFLSRobustStatSegmentor3DLabelMap_single_h_
