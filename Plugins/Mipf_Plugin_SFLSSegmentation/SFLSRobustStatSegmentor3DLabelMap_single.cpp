#ifndef SFLSRobustStatSegmentor3DLabelMap_single_txx_
#define SFLSRobustStatSegmentor3DLabelMap_single_txx_

#include "SFLSRobustStatSegmentor3DLabelMap_single.h"

#include <algorithm>
#include <ctime>
#include <limits>
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionConstIterator.h"

/* ============================================================   */
template <typename TPixel>
void CSFLSRobustStatSegmentor3DLabelMap<TPixel>::basicInit()
{
    SuperClassType::basicInit();

    m_statNeighborX = 1;
    m_statNeighborY = 1;
    m_statNeighborZ = 1;

    m_kernelWidthFactor = 10.0;

    m_inputImageIntensityMin = 0;
    m_inputImageIntensityMax = 0;

    m_bStopSegmentation = false;

    return;
}

/* ============================================================  */
template <typename TPixel>
void CSFLSRobustStatSegmentor3DLabelMap<TPixel>::setInputLabelImage(TLabelImagePointer l)
{
    m_inputLabelImage = l;

    TSize size = m_inputLabelImage->GetLargestPossibleRegion().GetSize();

    TIndex start = m_inputLabelImage->GetLargestPossibleRegion().GetIndex();
    TIndex origin = {{0, 0, 0}};
    if( start != origin )
    {
        std::cout << "Warrning: Force mask start to be (0, 0, 0)\n";

        TRegion region = m_inputLabelImage->GetLargestPossibleRegion();
        region.SetIndex(origin);

        m_inputLabelImage->SetRegions(region);
    }

    if( this->m_nx + this->m_ny + this->m_nz == 0 )
    {
        this->m_nx = size[0];
        this->m_ny = size[1];
        this->m_nz = size[2];
    }
    else if( this->m_nx != (long)size[0] || this->m_ny != (long)size[1] || this->m_nz != (long)size[2] )
    {
        std::cerr << "Error: image sizes do not match with label image size.\n";
        raise(SIGABRT);
    }

    return;
}

/* ============================================================  */
template <typename TPixel>
void CSFLSRobustStatSegmentor3DLabelMap<TPixel>::computeForce()
{
    double fmax = std::numeric_limits<double>::min();
    double kappaMax = std::numeric_limits<double>::min();

    long    n = this->m_lz.size();
    double* kappaOnZeroLS = new double[n];
    double* cvForce = new double[n];

    std::vector<typename CSFLSLayer::iterator> m_lzIterVct( n );
    {
        long iiizzz = 0;
        for( typename CSFLSLayer::iterator itz = this->m_lz.begin(); itz != this->m_lz.end(); ++itz )
        {
            m_lzIterVct[iiizzz++] = itz;
        }
    }


    for( long i = 0; i < n; ++i )
    {
        typename CSFLSLayer::iterator itz = m_lzIterVct[i];

        long ix = (*itz)[0];
        long iy = (*itz)[1];
        long iz = (*itz)[2];

        TIndex idx = {{ix, iy, iz}};

        kappaOnZeroLS[i] = this->computeKappa(ix, iy, iz);

        std::vector<double> f(m_numberOfFeature);

        computeFeatureAt(idx, f);

        double a = -kernelEvaluationUsingPDF(f);

        fmax = fmax > fabs(a) ? fmax : fabs(a);
        kappaMax = kappaMax > fabs(kappaOnZeroLS[i]) ? kappaMax : fabs(kappaOnZeroLS[i]);

        cvForce[i] = a;
    }


    this->m_force.resize(n);
    for( long i = 0; i < n; ++i )
    {
        this->m_force[i] = (1 - (this->m_curvatureWeight) ) * cvForce[i] / (fmax + 1e-10) \
                +  (this->m_curvatureWeight) * kappaOnZeroLS[i] / (kappaMax + 1e-10);
    }

    delete[] kappaOnZeroLS;
    delete[] cvForce;
}

/* ============================================================  */
template <typename TPixel>
void CSFLSRobustStatSegmentor3DLabelMap<TPixel>::inputLableImageToSeeds()
{
    typedef itk::ImageRegionConstIteratorWithIndex<TLabelImage> ImageRegionConstIteratorWithIndex_t;
    ImageRegionConstIteratorWithIndex_t it(m_inputLabelImage, m_inputLabelImage->GetLargestPossibleRegion() );
    it.GoToBegin();

    //std::ofstream sf("_seeds.txt");

    {
        std::vector<long> thisSeed(3);
        for( ; !it.IsAtEnd(); ++it )
        {
            if( it.Get() != 0 )  // 0 for bkgd, every label else is obj
            {
                TIndex idx = it.GetIndex();
                thisSeed[0] = idx[0];
                thisSeed[1] = idx[1];
                thisSeed[2] = idx[2];

                m_seeds.push_back(thisSeed);

                //sf << thisSeed[0] << ", " << thisSeed[1] << ", " << thisSeed[2] << std::endl;
            }
        }
    }

    //sf.close();

    return;
}

/* ============================================================  */
template <typename TPixel>
void CSFLSRobustStatSegmentor3DLabelMap<TPixel>::getThingsReady()
{
    inputLableImageToSeeds();

    seedToMask();

    initFeatureComputedImage();
    initFeatureImage();

    getFeatureAroundSeeds();
    estimateFeatureStdDevs();

    estimatePDFs();

    return;
}

/* ============================================================ */
template <typename TPixel>
void CSFLSRobustStatSegmentor3DLabelMap<TPixel>::initFeatureImage()
{
    if( !(this->mp_img) )
    {
        std::cerr << "Error: set input image first.\n";
        raise(SIGABRT);
    }
    for( long ifeature = 0; ifeature < m_numberOfFeature; ++ifeature )
    {
        TFloatImagePointer fimg = TFloatImage::New();
        fimg->SetRegions(this->mp_img->GetLargestPossibleRegion() );
        fimg->Allocate();
        fimg->CopyInformation(this->mp_img);

        m_featureImageList.push_back(fimg);
    }

    return;
}

/* ============================================================ */
template <typename TPixel>
void CSFLSRobustStatSegmentor3DLabelMap<TPixel>::initFeatureComputedImage()
{
    if( !(this->mp_img) )
    {
        std::cerr << "Error: set input image first.\n";
        raise(SIGABRT);
    }

    m_featureComputed = TLabelImage::New();
    m_featureComputed->SetRegions(this->mp_img->GetLargestPossibleRegion() );
    m_featureComputed->Allocate();
    m_featureComputed->CopyInformation(this->mp_img);
    m_featureComputed->FillBuffer(0);

    return;
}

/* ============================================================ */
template <typename TPixel>
void CSFLSRobustStatSegmentor3DLabelMap<TPixel>::computeFeatureAt(TIndex idx, std::vector<double>& f)
{
    f.resize(m_numberOfFeature);

    if( m_featureComputed->GetPixel(idx) )
    {
        // the feature at this pixel is computed, just retrive
        for( long i = 0; i < m_numberOfFeature; ++i )
        {
            f[i] = (m_featureImageList[i])->GetPixel(idx);
        }
    }
    else
    {
        // compute the feature
        std::vector<double> neighborIntensities;

        long ix = idx[0];
        long iy = idx[1];
        long iz = idx[2];
        for( long iiz = iz - m_statNeighborZ; iiz <= iz + m_statNeighborZ; ++iiz )
        {
            for( long iiy = iy - m_statNeighborY; iiy <= iy + m_statNeighborY; ++iiy )
            {
                for( long iix = ix - m_statNeighborX; iix <= ix + m_statNeighborX; ++iix )
                {
                    if( 0 <= iix && iix < this->m_nx    \
                            && 0 <= iiy && iiy < this->m_ny    \
                            && 0 <= iiz && iiz < this->m_nz )
                    {
                        TIndex idxa = {{iix, iiy, iiz}};
                        neighborIntensities.push_back(this->mp_img->GetPixel(idxa) );
                    }
                }
            }
        }

        getRobustStatistics(neighborIntensities, f);
        for( long ifeature = 0; ifeature < m_numberOfFeature; ++ifeature )
        {
            m_featureImageList[ifeature]->SetPixel(idx, f[ifeature]);
        }

        m_featureComputed->SetPixel(idx, 1);   // mark as computed
    }

    return;
}

/* ============================================================  */
template <typename TPixel>
void CSFLSRobustStatSegmentor3DLabelMap<TPixel>::doSegmenation()
{
    m_bStopSegmentation = false;
    double startingTime = clock();

    getThingsReady();

    this->initializeSFLS();

    for( unsigned int it = 0; it < this->m_numIter; ++it )
    {
        if (m_bStopSegmentation)
        {
            break;
        }
        std::cout << "In iteration " << it << std::endl << std::flush;

        // keep current zero contour as history is required
        if( this->m_keepZeroLayerHistory )
        {
            (this->m_zeroLayerHistory).push_back(this->m_lz);
        }

        double oldVoxelCount = this->m_insideVoxelCount;

        computeForce();

        this->normalizeForce();

        this->oneStepLevelSetEvolution();

        emit SignalInteractionEnd(mp_phi,it);

        /*----------------------------------------------------------------------
        If the level set stops growing, stop */
        this->updateInsideVoxelCount();
        if( it > 2 && oldVoxelCount >= this->m_insideVoxelCount )
        {
            break;
        }

        /* If the level set stops growing, stop
       ----------------------------------------------------------------------*/

        /*----------------------------------------------------------------------
      If the inside physical volume exceed expected volume, stop */
        double volumeIn = (this->m_insideVoxelCount) * (this->m_dx) * (this->m_dy) * (this->m_dz);
        if( volumeIn > (this->m_maxVolume) )
        {
            break;
        }
        /*If the inside physical volume exceed expected volume, stop
      ----------------------------------------------------------------------*/

        double ellapsedTime = (clock() - startingTime) / static_cast<double>(CLOCKS_PER_SEC);
        if( ellapsedTime > (this->m_maxRunningTime) )
        {
            break;
        }

    }
    emit SignalSegmentationFinished();
    return;
}

/* ============================================================ */
template <typename TPixel>
void CSFLSRobustStatSegmentor3DLabelMap<TPixel>::getRobustStatistics(std::vector<double>& samples, std::vector<double>& robustStat)
{
    /* note, sample is sorted, so the order is changed */
    robustStat.resize(m_numberOfFeature);

    std::sort(samples.begin(), samples.end() );

    double n = samples.size();

    double q1 = n / 4.0;
    double q1_floor;
    double l1 = modf(q1, &q1_floor);

    double q2 = n / 2.0;
    double q2_floor;
    double l2 = modf(q2, &q2_floor);

    double q3 = 3.0 * n / 4.0;
    double q3_floor;
    double l3 = modf(q3, &q3_floor);

    double median = (1 - l2) * samples[static_cast<long>(q2_floor)] + l2 * samples[static_cast<long>(q2_floor) + 1];

    double iqr = ( (1 - l3) * samples[static_cast<long>(q3_floor)] + l3 * samples[static_cast<long>(q3_floor) + 1] ) \
            - ( (1 - l1) * samples[static_cast<long>(q1_floor)] + l1 * samples[static_cast<long>(q1_floor) + 1] );

    robustStat[0] = median;
    robustStat[1] = iqr;

    /* next compute MAD */
    long  nn = samples.size();
    std::vector<double> samplesDeMedian(nn);
    for( long i = 0; i < nn; ++i )
    {
        samplesDeMedian[i] = fabs(samples[i] - median);
    }

    std::sort(samplesDeMedian.begin(), samplesDeMedian.end() );

    double mad =
            (1 - l2) * samplesDeMedian[static_cast<long>(q2_floor)] + l2 * samplesDeMedian[static_cast<long>(q2_floor) + 1];
    robustStat[2] = mad;

    return;
}

/* ============================================================ */
template <typename TPixel>
void CSFLSRobustStatSegmentor3DLabelMap<TPixel>::seedToMask()
{
    if( !(this->mp_img) )
    {
        std::cerr << "Error: set input image first.\n";
        raise(SIGABRT);
    }

    if( this->mp_mask )
    {

        return;
    }

    long n = m_seeds.size();
    if( n == 0 )
    {
        std::cerr << "Error: No seeds specified." << std::endl;
        raise(SIGABRT);
    }

    this->mp_mask = TMaskImage::New();
    this->mp_mask->SetRegions(this->mp_img->GetLargestPossibleRegion() );
    this->mp_mask->Allocate();
    this->mp_mask->CopyInformation(this->mp_img);
    this->mp_mask->FillBuffer(0);
    for( long i = 0; i < n; ++i )
    {
        if( 3 != m_seeds[i].size() )
        {
            std::cerr << "Error: 3 != m_seeds[i].size()\n";
            raise(SIGABRT);
        }

        long ix = m_seeds[i][0];
        long iy = m_seeds[i][1];
        long iz = m_seeds[i][2];
        for( long iiz = iz - 1; iiz <= iz + 1; ++iiz )
        {
            for( long iiy = iy - 1; iiy <= iy + 1; ++iiy )
            {
                for( long iix = ix - 1; iix <= ix + 1; ++iix )
                {
                    if( 0 <= iix && iix < this->m_nx    \
                            && 0 <= iiy && iiy < this->m_ny    \
                            && 0 <= iiz && iiz < this->m_nz )
                    {
                        TIndex idx = {{iix, iiy, iiz}};
                        this->mp_mask->SetPixel(idx, 1);
                    }
                }
            }
        }
    }

    return;
}

/* ============================================================ */
template <typename TPixel>
void CSFLSRobustStatSegmentor3DLabelMap<TPixel>::dialteSeeds()
{
    /* For each seed, add its 26 neighbors into the seed list. */

    if( !(this->mp_img) )
    {
        std::cerr << "Error: set input image first.\n";
        raise(SIGABRT);
    }

    long                            n = m_seeds.size();
    std::vector<std::vector<long> > newSeeds;

    if( n == 0 )
    {
        std::cerr << "Error: No seeds specified." << std::endl;
        raise(SIGABRT);
    }
    for( long i = 0; i < n; ++i )
    {
        if( 3 != m_seeds[i].size() )
        {
            std::cerr << "Error: 3 != m_seeds[i].size()\n";
            raise(SIGABRT);
        }

        long ix = m_seeds[i][0];
        long iy = m_seeds[i][1];
        long iz = m_seeds[i][2];
        for( long iiz = iz - 1; iiz <= iz + 1; ++iiz )
        {
            for( long iiy = iy - 1; iiy <= iy + 1; ++iiy )
            {
                for( long iix = ix - 1; iix <= ix + 1; ++iix )
                {
                    if( 0 <= iix && iix < this->m_nx    \
                            && 0 <= iiy && iiy < this->m_ny    \
                            && 0 <= iiz && iiz < this->m_nz )
                    {
                        /* Some locations may be added multiple times,
               if the original seeds are close. But I think
               this is fine */

                        std::vector<long> s(3);
                        s[0] = iix;
                        s[1] = iiy;
                        s[2] = iiz;

                        newSeeds.push_back(s);
                    }
                }
            }
        }
    }

    m_seeds.assign(newSeeds.begin(), newSeeds.end() );

    return;
}

/* ============================================================  */
template <typename TPixel>
void CSFLSRobustStatSegmentor3DLabelMap<TPixel>::getFeatureAroundSeeds()
{
    if( !m_featureImageList[m_numberOfFeature - 1] )
    {
        // last feature image is not constructed
        std::cerr << "Error: construct feature images first.\n";
        raise(SIGABRT);
    }

    long n = m_seeds.size();
    if( n == 0 )
    {
        std::cerr << "Error: No seeds specified." << std::endl;
        raise(SIGABRT);
    }

    for( long i = 0; i < n; ++i )
    {
        if( 3 != m_seeds[i].size() )
        {
            std::cerr << "Error: 3 != m_seeds[i].size()\n";
            raise(SIGABRT);
        }

        long ix = m_seeds[i][0];
        long iy = m_seeds[i][1];
        long iz = m_seeds[i][2];

        TIndex idx = {{ix, iy, iz}};

        std::vector<double> featureHere(m_numberOfFeature);
        computeFeatureAt(idx, featureHere);


        m_featureAtTheSeeds.push_back(featureHere);

    }

    return;
}

/* ============================================================ */
template <typename TPixel>
void CSFLSRobustStatSegmentor3DLabelMap<TPixel>::estimateFeatureStdDevs()
{
    m_kernelStddev.assign(m_numberOfFeature, 0.0);

    long n = m_seeds.size(); // == m_featureAtTheSeeds.size()
    for( long i = 0; i < m_numberOfFeature; ++i )
    {
        double m = 0;
        for( long ii = 0; ii < n; ++ii )
        {
            m += m_featureAtTheSeeds[ii][i];
        }
        m /= n;
        for( long ii = 0; ii < n; ++ii )
        {
            m_kernelStddev[i] += (m_featureAtTheSeeds[ii][i] - m) * (m_featureAtTheSeeds[ii][i] - m);
        }

        m_kernelStddev[i] /= (n - 1);
        m_kernelStddev[i] = sqrt(m_kernelStddev[i]);
    }

    return;
}

template <typename TPixel>
double CSFLSRobustStatSegmentor3DLabelMap<TPixel>::kernelEvaluationUsingPDF(const std::vector<double>& newFeature)
{
    double p = 1;

    for( long i = 0; i < m_numberOfFeature; ++i )
    {
        long idx = static_cast<long>(newFeature[i] - m_inputImageIntensityMin);

        double probOfThisFeature = m_PDFlearnedFromSeeds[i][idx];

        p *= probOfThisFeature;
    }

    return p;
}

/* ============================================================  */
template <typename TPixel>
double CSFLSRobustStatSegmentor3DLabelMap<TPixel>::kernelEvaluation(const std::vector<double>& newFeature)
{
    long n = m_seeds.size(); // == m_featureAtTheSeeds.size()

    double p = 1;

    // double p = 0;
    for( long i = 0; i < m_numberOfFeature; ++i )
    {
        double pp = 0.0;

        double stdDev = m_kernelStddev[i] / m_kernelWidthFactor; // /10 as in Eric's appendix

        double var2 = -1.0 / (2 * stdDev * stdDev);
        double c = 1.0 / sqrt(2 * (vnl_math::pi) ) / stdDev;
        for( long ii = 0; ii < n; ++ii )
        {
            pp += exp(var2 * (newFeature[i] - m_featureAtTheSeeds[ii][i]) * (newFeature[i] - m_featureAtTheSeeds[ii][i]) );
        }

        pp *= c;
        pp /= n;

        p *= pp;
    }

    return p;
}

/* ============================================================  */
template <typename TPixel>
void CSFLSRobustStatSegmentor3DLabelMap<TPixel>::setKernelWidthFactor(double f)
{
    if( f < 0.1 )
    {
        m_kernelWidthFactor = 0.1;
    }

    if( f > 20.0 )
    {
        m_kernelWidthFactor = 20.0;
    }

    m_kernelWidthFactor = f;


    return;
}

/* ============================================================  */
template <typename TPixel>
void CSFLSRobustStatSegmentor3DLabelMap<TPixel>::setIntensityHomogeneity(double h)
{
    double f = h * (20.0 - 0.1) + 0.1;

    setKernelWidthFactor(f);

    return;
}

/* ============================================================  */
template <typename TPixel>
void CSFLSRobustStatSegmentor3DLabelMap<TPixel>::estimatePDFs()
{
    m_PDFlearnedFromSeeds.clear();

    computeMinMax(); // so we have the range of all pdfs

    long n = m_seeds.size();
    for( long ifeature = 0; ifeature < m_numberOfFeature; ++ifeature )
    {
        std::vector<double> thisPDF(m_inputImageIntensityMax - m_inputImageIntensityMin + 1);
        // assumption: TPixel are of integer types.

        double stdDev = m_kernelStddev[ifeature] / m_kernelWidthFactor; // /10 as in Eric's appendix

        double var2 = -1.0 / (2 * stdDev * stdDev);
        double c = 1.0 / sqrt(2 * (vnl_math::pi) ) / stdDev;
        for( TPixel a = m_inputImageIntensityMin; a <= m_inputImageIntensityMax; ++a )
        {
            long ia = static_cast<long>(a - m_inputImageIntensityMin);

            double pp = 0.0;
            for( long ii = 0; ii < n; ++ii )
            {
                pp += exp(var2 * (a - m_featureAtTheSeeds[ii][ifeature]) * (a - m_featureAtTheSeeds[ii][ifeature]) );

            }

            pp *= c;
            pp /= n;

            thisPDF[ia] = pp;
        }

        m_PDFlearnedFromSeeds.push_back(thisPDF);
    }

    return;
}

/* ============================================================  */
template <typename TPixel>
void CSFLSRobustStatSegmentor3DLabelMap<TPixel>::computeMinMax()
{
    if( !(this->mp_img) )
    {
        std::cerr << "Error: set input image first.\n";
        raise(SIGABRT);
    }

    typedef itk::Image<TPixel, 3> itkImage_t;

    typedef itk::ImageRegionConstIterator<itkImage_t> itkImageRegionConstIterator_t;

    itkImageRegionConstIterator_t it( (this->mp_img), (this->mp_img)->GetLargestPossibleRegion() );
    it.GoToBegin();

    m_inputImageIntensityMin = std::numeric_limits<unsigned>::max(); // yes, it's twisted so easity to compute.
    m_inputImageIntensityMax = std::numeric_limits<unsigned>::min();
    for( ; !it.IsAtEnd(); ++it )
    {
        TPixel v = it.Get();

        m_inputImageIntensityMin = m_inputImageIntensityMin < v ? m_inputImageIntensityMin : v;
        m_inputImageIntensityMax = m_inputImageIntensityMax > v ? m_inputImageIntensityMax : v;
    }

    return;
}

#endif
