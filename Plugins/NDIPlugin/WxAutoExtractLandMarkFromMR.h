#pragma once

#include <QVector3D>

#include <mitkDataNode.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>

#include "itkImage.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkTranslationTransform.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkExtractImageFilter.h"
#include "itkNormalizedCorrelationImageToImageMetric.h"

typedef mitk::Point3D WxPoint3D;

class WxAutoExtractLandMarkFromMR
{
public:

	WxAutoExtractLandMarkFromMR();
	~WxAutoExtractLandMarkFromMR();
	
	void FindModelPoints(mitk::DataNode::Pointer mainNode,
									  double dSPVDistance, 
									  short sMaxThreshold, short sMinThreshold,  
									  double dxCut, double dyCut, double dzCut);
		
	std::vector<WxPoint3D> GetResult();

	bool RegionOfInterestImageFilter(mitk::Image::Pointer original, mitk::Image::Pointer roi, QVector3D roiSize,QVector3D roiStart);
	double MeanSquaresImageToImage(mitk::Image::Pointer iFixData, mitk::Image::Pointer iMovingData);

private:
	double DistanceBetweenPointAndModel(std::vector<WxPoint3D> vecModel, WxPoint3D pPoint);
	double MaxDistanceOfModel(std::vector<WxPoint3D> vecModel);
	WxPoint3D CalculateCenterPointOfModel(std::vector<WxPoint3D> vecModel);

private:
	short sVixelValueThresholdMax;		//��־��MR�������ֵ
	short sVixelValueThresholdMin;        //��־��MR��С����ֵ
	double dSamePointVixelDistance;    //��־�����ش�С��λ

	double dDistanceTemp;
	WxPoint3D pReturnPointTemp;
	std::vector<WxPoint3D> vecSpherePointMM;   //��־����������λ��
	bool bFoundHome;
};