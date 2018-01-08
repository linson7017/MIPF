#ifndef AirwaySegmentation_h__
#define AirwaySegmentation_h__

#include <itkImage.h>
#include <Segmentation/IQF_AirwaySegmentation.h>

class AirwaySegmentation : public IQF_AirwaySegmentation
{
	
public:
	#define DIM 3

	typedef signed short    InputPixelType;
	typedef unsigned short  OutputPixelType;

	typedef itk::Image<InputPixelType, DIM>  InputImageType;
	typedef itk::Image<OutputPixelType, DIM> OutputImageType;
	virtual int DoSegmentation();
	virtual void SetSourceImage(TInputImageType* const sourceimage);
	virtual void SetLabelImage(TOutputImageType*  labelimage);
	virtual void SetSeedPoint(const double* seed);
    virtual void Release();

	AirwaySegmentation();

private:
	InputImageType* m_pSourceImage;
	OutputImageType* m_pLabelImage;
	double SeedPoint[3];

	OutputImageType::Pointer TracheaSegmentation(InputImageType::Pointer VOI,
		InputImageType::IndexType indexFiducialSlice,
		std::vector<std::vector<float> > fiducial,
		int labelColor);

	template <class ImageType>
	typename ImageType::Pointer Paste(typename ImageType::Pointer sourceImage, typename ImageType::IndexType index, typename ImageType::Pointer destImage);

	OutputImageType::Pointer RightLeftSegmentation(InputImageType::Pointer VOI,
		InputImageType::IndexType index,
		std::string reconKernel,
		int trachea_voxels,
		int labelColor);

};
#endif // AirwaySegmentation_h__