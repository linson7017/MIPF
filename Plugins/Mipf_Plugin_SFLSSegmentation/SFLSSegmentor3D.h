#ifndef LABELMAPPREPROCESSOR_H
#define LABELMAPPREPROCESSOR_H

#include "itkImage.h"
#include "itkImageRegionIterator.h"

#include <algorithm>

template <typename pixel_t>
typename itk::Image<pixel_t, 3>::Pointer
preprocessLabelMap(typename itk::Image<pixel_t, 3>::Pointer originalLabelMap, pixel_t desiredLabel)
{
    typedef itk::Image<pixel_t, 3> image_t;

    typedef itk::ImageRegionIterator<image_t> imageRegionIterator_t;

    // 1.
    imageRegionIterator_t iter(originalLabelMap, originalLabelMap->GetLargestPossibleRegion() );
    iter.GoToBegin();

    typename image_t::SizeType sz = originalLabelMap->GetLargestPossibleRegion().GetSize();

    std::vector<pixel_t> uniqueLabels(sz[0] * sz[1] * sz[2]);
    long                 i = 0;
    for( ; !iter.IsAtEnd(); ++iter )
    {
        uniqueLabels[i++] = iter.Get();
    }

    std::sort(uniqueLabels.begin(), uniqueLabels.end() );
    typename std::vector<pixel_t>::iterator itl = std::unique(uniqueLabels.begin(), uniqueLabels.end() );
    uniqueLabels.resize( itl - uniqueLabels.begin() );

    if( uniqueLabels[0] != 0 )
    {
        std::cerr << "Error: least label is not 0? no background?\n";
    }

    short numOfLabels = uniqueLabels.size() - 1; // 0 not count

    // 2.
    if( 1 == numOfLabels )
    {
        return originalLabelMap;
    }

    // 3.
    if( !std::binary_search(uniqueLabels.begin(), uniqueLabels.end(), desiredLabel) )
    {
        return originalLabelMap;
    }

    // 4.
    typename image_t::Pointer newLabelMap = image_t::New();
    newLabelMap->CopyInformation(originalLabelMap);
    newLabelMap->SetRegions( originalLabelMap->GetLargestPossibleRegion() );
    newLabelMap->Allocate();
    newLabelMap->FillBuffer(0);

    imageRegionIterator_t iterNew(newLabelMap, newLabelMap->GetLargestPossibleRegion() );
    iterNew.GoToBegin();
    iter.GoToBegin();
    for( ; !iter.IsAtEnd(); ++iter, ++iterNew )
    {
        if( iter.Get() == desiredLabel )
        {
            iterNew.Set(1);
        }
    }

    return newLabelMap;
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef  SFLS_h_
#define SFLS_h_

#include <list>
#include "vnl/vnl_vector_fixed.h"

class CSFLS
{
public:
    typedef CSFLS Self;

    typedef vnl_vector_fixed<int, 3> NodeType;
    typedef std::list<NodeType>      CSFLSLayer;

    CSFLS(){}

    CSFLSLayer m_lz;
    CSFLSLayer m_ln1;
    CSFLSLayer m_ln2;
    CSFLSLayer m_lp1;
    CSFLSLayer m_lp2;
};

#endif //SFLS_h_

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SFLSSegmentor3D_h_
#define SFLSSegmentor3D_h_

#include <list>
#include <vector>

#include "itkImage.h"

template <typename TPixel>
class CSFLSSegmentor3D : public CSFLS
{
public:
    typedef CSFLSSegmentor3D<TPixel> Self;

    typedef CSFLS SuperClassType;

    typedef SuperClassType::NodeType   NodeType;
    typedef SuperClassType::CSFLSLayer CSFLSLayer;

    typedef itk::Image<TPixel, 3>        TImage;
    typedef itk::Image<float, 3>         TFloatImage;
    typedef itk::Image<double, 3>        TDoubleImage;
    typedef itk::Image<char, 3>          TCharImage;
    typedef itk::Image<unsigned char, 3> TUCharImage;
    typedef itk::Image<unsigned short, 3>         TShortImage;

    typedef TImage      ImageType;
    typedef TFloatImage LSImageType;
    typedef TCharImage  LabelImageType;
    typedef TUCharImage MaskImageType;
    typedef TShortImage ShortImageType;

    typedef typename TImage::IndexType  TIndex;
    typedef typename TImage::SizeType   TSize;
    typedef typename TImage::RegionType TRegion;

    CSFLSSegmentor3D();
    virtual ~CSFLSSegmentor3D()
    {

    }

    void basicInit();

    void setNumIter(unsigned long n);

    void setImage(typename ImageType::Pointer img);
    void setMask(typename MaskImageType::Pointer mask);

    virtual void computeForce() = 0;

    void normalizeForce();

    bool getPhiOfTheNbhdWhoIsClosestToZeroLevelInLayerCloserToZeroLevel(long ix, long iy, long iz, double& thePhi);

    void oneStepLevelSetEvolution();

    void initializeSFLS()
    {
        initializeSFLSFromMask();
    }
    void initializeSFLSFromMask(); // m_insideVoxelCount is first computed here

    void initializeLabel();

    void initializePhi();

    // geometry
    double computeKappa(long ix, long iy, long iz);

    void setMaxVolume(double v); // v is in mL

    void setMaxRunningTime(double t); // t in min

    // about evolution history
    void keepZeroLayerHistory(bool b)
    {
        m_keepZeroLayerHistory = b;
    }
    void getZeroLayerAtIteration(unsigned long i);

    void writeZeroLayerAtIterationToFile(unsigned long i, const char* name);

    void writeZeroLayerToFile(const char* namePrefix);

    void setCurvatureWeight(double a);

    LSImageType::Pointer getLevelSetFunction();


    typename ImageType::Pointer mp_img;
    typename LabelImageType::Pointer mp_label;
    typename MaskImageType::Pointer mp_mask; // 0, non-0 mask for object
    typename LSImageType::Pointer mp_phi;

    std::vector<double> m_force;

    double m_timeStep;

    unsigned long m_numIter;
protected:
    double m_curvatureWeight;

    bool m_done;

    long m_nx;
    long m_ny;
    long m_nz;

    double m_dx; // in mm
    double m_dy; // in mm
    double m_dz; // in mm

    long   m_insideVoxelCount;
    double m_insideVolume;

    double m_maxVolume;      // max physical volume, in mm^3
    double m_maxRunningTime; // in sec

    CSFLSLayer m_lIn2out;
    CSFLSLayer m_lOut2in;

    void updateInsideVoxelCount();

    inline bool doubleEqual(double a, double b, double eps = 1e-10)
    {
        return a - b < eps && b - a < eps;
    }

    bool                    m_keepZeroLayerHistory;
    std::vector<CSFLSLayer> m_zeroLayerHistory;

};

#include "SFLSSegmentor3D.cpp"

#endif //SFLSSegmentor3D_h_

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
