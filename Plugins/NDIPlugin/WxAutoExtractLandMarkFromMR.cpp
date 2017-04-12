#include "WxAutoExtractLandMarkFromMR.h"

WxAutoExtractLandMarkFromMR::WxAutoExtractLandMarkFromMR()
{
	sVixelValueThresholdMax = 28000;  //MR最大像素值
	sVixelValueThresholdMin = 15000;   //MR最小像素值
	dSamePointVixelDistance = 8;         //体素大小单位

	dDistanceTemp = 0.0;
	bFoundHome = false;
} 

WxAutoExtractLandMarkFromMR::~WxAutoExtractLandMarkFromMR()
{
}

double WxAutoExtractLandMarkFromMR::DistanceBetweenPointAndModel(std::vector<WxPoint3D> vecModel, WxPoint3D pPoint)
{
	double dDistanceTemp;
	double dDistanceMin = 99999;
	std::vector<WxPoint3D>::iterator itrOfPoint = vecModel.begin();
	for (; itrOfPoint != vecModel.end(); itrOfPoint++)
	{
		dDistanceTemp = sqrt(((*itrOfPoint)[0]-pPoint[0]) * ((*itrOfPoint)[0]-pPoint[0])
									+ ((*itrOfPoint)[1]-pPoint[1]) * ((*itrOfPoint)[1]-pPoint[1])
									+((*itrOfPoint)[2]-pPoint[2]) * ((*itrOfPoint)[2]-pPoint[2]) );

		if (dDistanceTemp < dDistanceMin)
		{
			dDistanceMin = dDistanceTemp;
		}
	}
	return dDistanceMin;
}

double WxAutoExtractLandMarkFromMR::MaxDistanceOfModel(std::vector<WxPoint3D> vecModel)
{
	double dDistanceTemp;
	double dDistanceMax = 0.0;
	std::vector<WxPoint3D> vecModelTemp;

	std::vector<WxPoint3D>::iterator itrOfPoint = vecModel.begin();
	for (; itrOfPoint != vecModel.end(); itrOfPoint++)
	{
		vecModelTemp.push_back(*itrOfPoint);
	}

	itrOfPoint = vecModel.begin();
	for (; itrOfPoint != vecModel.end(); itrOfPoint++)
	{
		std::vector<WxPoint3D>::iterator itrOfPointTemp = vecModelTemp.begin();
		for (; itrOfPointTemp != vecModelTemp.end(); itrOfPointTemp++)
		{
			dDistanceTemp = sqrt(((*itrOfPoint)[0]-(*itrOfPointTemp)[0]) * ((*itrOfPoint)[0]-(*itrOfPointTemp)[0])
										+ ((*itrOfPoint)[1]-(*itrOfPointTemp)[1]) * ((*itrOfPoint)[1]-(*itrOfPointTemp)[1])
										+((*itrOfPoint)[2]-(*itrOfPointTemp)[2]) * ((*itrOfPoint)[2]-(*itrOfPointTemp)[2]) );

			if (dDistanceMax < dDistanceTemp)
			{
				dDistanceMax = dDistanceTemp;
			}
		}		
	}
	return dDistanceMax;
}

WxPoint3D WxAutoExtractLandMarkFromMR::CalculateCenterPointOfModel(std::vector<WxPoint3D> vecModel)
{
	WxPoint3D pCenter;
	double dX = 0.0, dY = 0.0, dZ = 0.0;

	if (vecModel.size() > 0)
	{
		std::vector<WxPoint3D>::iterator itrOfPoint = vecModel.begin();
		for (; itrOfPoint != vecModel.end(); itrOfPoint++)
		{
			dX += (*itrOfPoint)[0];
			dY += (*itrOfPoint)[1];
			dZ += (*itrOfPoint)[2];
		}

		pCenter[0] = dX / vecModel.size();
		pCenter[1] = dY / vecModel.size();
		pCenter[2] = dZ / vecModel.size();
	}
	return pCenter;
}

void WxAutoExtractLandMarkFromMR::FindModelPoints(mitk::DataNode::Pointer mainNode, 
																						double dSPVDistance,
																						short sMaxThreshold, short sMinThreshold, 
																						double dxCut, double dyCut, double dzCut)
{
	sVixelValueThresholdMax = sMaxThreshold;  //MR最大像素值
	sVixelValueThresholdMin = sMinThreshold;    //MR最小像素值
	dSamePointVixelDistance = dSPVDistance;  //体素大小单位

	mitk::Image::Pointer  mainImage = dynamic_cast<mitk::Image*>(mainNode->GetData());
	unsigned int* dimens = mainImage->GetDimensions();
	short* mainData = (short*)mainImage->GetData();
	int mX = dimens[0];
	int mY = dimens[1];
	int mZ = dimens[2];

	std::vector<std::vector<WxPoint3D>> vecAllDetectPoints;

	for (int z = int(mZ*dzCut); z<int(mZ*(1-dzCut));z++)
	{
		for (int y=0;y<int(mY*dyCut);y++)
		{
			for (int x=int(mX*dxCut);x<int(mX*(1-dxCut));x++)
			{
				int index = z*mY*mX + y*mX + x;
				short data =mainData[index];
				if((data < sVixelValueThresholdMax) && (data > sVixelValueThresholdMin))
				{
					pReturnPointTemp[0] = x;
					pReturnPointTemp[1] = y;
					pReturnPointTemp[2] = z;

					mainImage->GetGeometry()->IndexToWorld(pReturnPointTemp, pReturnPointTemp);

					bFoundHome = false;
					dDistanceTemp = 0.0;
					std::vector<std::vector<WxPoint3D>>::iterator itrOfAllPoint = vecAllDetectPoints.begin();
					for (; itrOfAllPoint != vecAllDetectPoints.end(); itrOfAllPoint++)
					{
						dDistanceTemp = DistanceBetweenPointAndModel(*itrOfAllPoint, pReturnPointTemp);

						if (dDistanceTemp < dSamePointVixelDistance)
						{
							(*itrOfAllPoint).push_back(pReturnPointTemp);
							bFoundHome = true;
							break;
						}
					}

					if (!bFoundHome)  //如果没有找到相同模型的点,则另立新模型
					{
						std::vector<WxPoint3D> vecModelPoint;
						vecModelPoint.push_back(pReturnPointTemp);
						vecAllDetectPoints.push_back(vecModelPoint);
					}
				}
			}
		}
	}
	
	vecSpherePointMM.clear();

	std::vector<std::vector<WxPoint3D>>::iterator itrOfAllPoint = vecAllDetectPoints.begin();
	for (; itrOfAllPoint != vecAllDetectPoints.end(); itrOfAllPoint++)
	{
		double dMax = MaxDistanceOfModel(*itrOfAllPoint);
		if (dMax < dSamePointVixelDistance)
		{
			if(itrOfAllPoint->size()>=dSamePointVixelDistance)
			{
				WxPoint3D pCenter3D = CalculateCenterPointOfModel(*itrOfAllPoint);
				vecSpherePointMM.push_back(pCenter3D);
			}
		}
	}
}

std::vector<WxPoint3D> WxAutoExtractLandMarkFromMR::GetResult()
{
	return vecSpherePointMM;
}

double WxAutoExtractLandMarkFromMR::MeanSquaresImageToImage(mitk::Image::Pointer iFixData, mitk::Image::Pointer iMovingData)
{
	const unsigned int Dimension = 3;
	typedef  short  PixelType;

	typedef itk::Image< PixelType, Dimension >   ImageType;

	typedef itk::NormalizedCorrelationImageToImageMetric<ImageType, ImageType >  MetricType;
	MetricType::Pointer metric = MetricType::New();

	typedef itk::TranslationTransform< double, Dimension >  TransformType;
	TransformType::Pointer transform = TransformType::New();
	typedef itk::NearestNeighborInterpolateImageFunction<ImageType, double >  InterpolatorType;
	InterpolatorType::Pointer interpolator = InterpolatorType::New();

	transform->SetIdentity();
	ImageType::Pointer fixedImage = ImageType::New();
	ImageType::Pointer movingImage = ImageType::New();

	mitk::CastToItkImage( iFixData, fixedImage );
	mitk::CastToItkImage( iMovingData, movingImage );

	metric->SetTransform( transform );
	metric->SetInterpolator( interpolator );
	metric->SetFixedImage(  fixedImage  );
	metric->SetMovingImage( movingImage );
	metric->SetFixedImageRegion( fixedImage->GetLargestPossibleRegion() );
	try
	{
		metric->Initialize();
	}
	catch( itk::ExceptionObject & excep )
	{
		std::cerr << "Exception catched !" << std::endl;
		std::cerr << excep << std::endl;
	}

	return metric->GetValue(transform->GetParameters());
}


bool WxAutoExtractLandMarkFromMR::RegionOfInterestImageFilter(mitk::Image::Pointer original, 
																										  mitk::Image::Pointer roi,
																										  QVector3D roiSize,
																										  QVector3D roiStart)
{
	const unsigned int Dimension = 3;
	typedef  short  PixelType;
	typedef itk::Image< PixelType, Dimension >   ImageType;
	ImageType::Pointer itkoriginal = ImageType::New();
	mitk::CastToItkImage( original, itkoriginal );

	ImageType::IndexType start;
	start[0] = roiStart.x(); 
	start[1] = roiStart.y(); 
	start[2] = roiStart.z();
	ImageType::SizeType size;
	size[0] = roiSize.x(); 
	size[1] = roiSize.y(); 
	size[2] = roiSize.z();
	ImageType::RegionType desiredRegion;
	desiredRegion.SetSize(size);
	desiredRegion.SetIndex(start);

	//typedef itk::RegionOfInterestImageFilter< ImageType, ImageType > RoiFilterType;
	//RoiFilterType::Pointer filter = RoiFilterType::New();
	//filter->SetRegionOfInterest(desiredRegion);
	//filter->SetInput(itkoriginal);
	//filter->Update();
	//mitk::CastToMitkImage(filter->GetOutput(),roi);

	typedef itk::ExtractImageFilter< ImageType, ImageType > ExtractFilterType;
	ExtractFilterType::Pointer filter = ExtractFilterType::New();
	filter->SetExtractionRegion(desiredRegion);
	filter->SetInput(itkoriginal);
	filter->Update();
	mitk::CastToMitkImage(filter->GetOutput(),roi);

	return true;
}