#ifndef SurfaceInfoCombineImageFilter_h__
#define SurfaceInfoCombineImageFilter_h__

#pragma once
#include "itkMacro.h"

namespace itk
{
	/** \class SurfaceInfoCombineImageFilter
	* \brief :This filter is used to combine the foreground binary image(TInputImage) with the surface gray scales of the origin image(TInfoImage)
	* The amount of the gray scale pixel in output image can be controlled by the parameter m_NeighbourSize which determined the neighbourhood size when scanning the edge
	* Contour value will be calculated by averaging the gray scales near the edge 
	*/
	template <class TInputImage, class TInfoImage, class TOutputImage>
	class SurfaceInfoCombineImageFilter : public itk::ImageToImageFilter<TInputImage, TOutputImage>
	{
	    public:
		    typedef SurfaceInfoCombineImageFilter             Self;
		    typedef ImageToImageFilter< TInputImage, TOutputImage > Superclass;
		    typedef SmartPointer< Self >        Pointer;
		    typedef SmartPointer<const Self>												ConstPointer;

		    //type define
		    typedef TInputImage																InputImageType;
		    typedef typename InputImageType::Pointer										InputImagePointer;
		    typedef typename InputImageType::ConstPointer									InputImageConstPointer;
		    typedef typename InputImageType::RegionType										InputImageRegionType;
		    typedef typename InputImageType::PixelType                                      InputImagePixelType;
		    typedef typename InputImageType::IndexType                                      InputImageIndexType;
		    typedef typename InputImageType::SizeType                                       InputImageSizeType;

		    /** **/
		    typedef TInfoImage														        InfoImageType;
		    typedef typename InfoImageType::Pointer											InfoImagePointer;
		    typedef typename InfoImageType::ConstPointer									InfoImageConstPointer;
		    typedef typename InfoImageType::RegionType										InfoImageRegionType;
		    typedef typename InfoImageType::PixelType										InfoImagePixelType;

		    /** **/
		    typedef TOutputImage														    OutputImageType;
		    typedef typename TOutputImage::Pointer											OutputImagePointer;
		    typedef typename TOutputImage::ConstPointer										OutputImageConstPointer;
		    typedef typename TOutputImage::RegionType										OutputImageRegionType;
		    typedef typename TOutputImage::PixelType										OutputImagePixelType;

		    /** internal image type during compution**/
		    typedef itk::Image<float, itkGetStaticConstMacro(InputImageDimension)>				InternalImageType;
		    typedef typename InternalImageType::IndexType									InternalImageIndexType;
		    typedef typename InternalImageType::SizeType									InternalImageSizeType;

		    void SetInputImage(const TInputImage* image);
		    void SetInfoImage(const InfoImageType* image);
            itkSetMacro(Foreground, int);
            itkSetMacro(Background, int);
            itkSetMacro(MaxScalar, int);
            itkSetMacro(MinScalar, int);
		    //Set the neighbourhood size, if value 1 is set, the neighbourhood will be 26, calculate by function (2*1+1)^3
		    itkSetMacro(NeighbourSize, int);
		    itkGetConstMacro(ContourValue, double);
            itkGetConstMacro(ContourValueMax, double);
            itkGetConstMacro(ContourValueMin, double);

		    /** Method for creation through the object factory. */
		    itkNewMacro(Self);
		    /** Run-time type information (and related methods). */
		    itkTypeMacro(SurfaceInfoCombineImageFilter, ImageToImageFilter);	
	    protected:
		    SurfaceInfoCombineImageFilter() :
                m_Foreground(255), 
                m_Background(0), 
                m_ContourValue(0), 
                m_NeighbourSize(1), 
                m_MaxScalar(10000.0),
                m_MinScalar(0.0),
                m_ContourValueMax(0.0),
                m_ContourValueMin(0.0){}
		    ~SurfaceInfoCombineImageFilter() {}
		    /** Does the real work. */
		    virtual void GenerateData();
		    bool GetNeighbors(std::vector<InputImagePixelType> &array, InputImageConstPointer image, InputImageIndexType index);
	    private:
            int m_Foreground;
            int m_Background;
            double m_ContourValue;
            int m_NeighbourSize;
            double m_MaxScalar;
            double m_MinScalar;
            double m_ContourValueMax;
            double m_ContourValueMin;

	};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkSurfaceInfoCombineImageFilter.hxx"
#endif


#endif // SurfaceInfoCombineImageFilter_h__
