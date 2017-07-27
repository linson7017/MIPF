#include "WxAutoExtractLandMarkFromCT.h"
#include "mitkImage.h"

WxAutoExtractLandMarkFromCT::WxAutoExtractLandMarkFromCT()
{
	sVixelValueThresholdMax = 2800;  //CT值
	sVixelValueThresholdMin = 1500;
	dSamePointVixelDistance = 10;   //体素单位

	dDistanceTemp = 0.0;
	bFoundHome = false;
} 

WxAutoExtractLandMarkFromCT::~WxAutoExtractLandMarkFromCT()
{

}

double WxAutoExtractLandMarkFromCT::DistanceBetweenPointAndModel(std::vector<WxPoint3D> vecModel, WxPoint3D pPoint)
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

double WxAutoExtractLandMarkFromCT::MaxDistanceOfModel(std::vector<WxPoint3D> vecModel)
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

WxPoint3D WxAutoExtractLandMarkFromCT::CalculateCenterPoint(std::vector<WxPoint3D> vecModel)
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

int WxAutoExtractLandMarkFromCT::FindModelPoints(mitk::DataNode::Pointer mainNode, std::vector<std::vector<double>> vecModelDistance, 
																		double dSPVDistance,
																		short sMaxThreshold, short sMinThreshold, 
																		double dxCut, double dyCut, double dzCut)
{
	if (vecModelDistance.size() == 0)
	{
		return 1;
	}

	sVixelValueThresholdMax = sMaxThreshold;  //标志点最大CT值
	sVixelValueThresholdMin = sMinThreshold;    //标志点最小CT值
	dSamePointVixelDistance = dSPVDistance;  //标志点体素大小
	
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

					if (!bFoundHome)  //如果没有找到相同模型的点，则另立新模型
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
			WxPoint3D pCenter3D = CalculateCenterPoint(*itrOfAllPoint);
			vecSpherePointMM.push_back(pCenter3D);
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////各个点坐标在vecSpherePointMM中，距离在vecModelDistance中///////////
	vecReturnPoint.clear();

	std::vector<WxPoint3D> vecSpherePointMMTemp;
	//创建临时点坐标集

	std::vector<WxPoint3D>::iterator itrOfPoint = vecSpherePointMM.begin();
	for (; itrOfPoint != vecSpherePointMM.end(); itrOfPoint++)
	{
		vecSpherePointMMTemp.push_back(*itrOfPoint);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//寻找与每个点距离最小的三个点，构成一个备选模型，共vecSpherePointMM.Size个。
	LineSegment lsLineOne, lsLineTwo, lsLineThree;
	std::vector<std::vector<LineSegment>> vecAlternativeModels;
	std::vector<LineSegment> vecCurrentModel;
	itrOfPoint = vecSpherePointMM.begin();
	for (; itrOfPoint != vecSpherePointMM.end(); itrOfPoint++)
	{
		lsLineOne.dDistance = 99999;
		lsLineTwo.dDistance = 99999;
		lsLineThree.dDistance = 99999;
		double dDistanceOfPoint;
		LineSegment lsDistanceCurrent, lsDistanceTemp;
		vecCurrentModel.clear();
		std::vector<WxPoint3D>::iterator itrOfPointTemp = vecSpherePointMMTemp.begin();
		for (; itrOfPointTemp != vecSpherePointMMTemp.end(); itrOfPointTemp++)
		{
			dDistanceOfPoint = sqrt(((*itrOfPoint)[0] - (*itrOfPointTemp)[0]) * ((*itrOfPoint)[0] - (*itrOfPointTemp)[0])
				+ ((*itrOfPoint)[1] - (*itrOfPointTemp)[1]) * ((*itrOfPoint)[1] - (*itrOfPointTemp)[1])
				+ ((*itrOfPoint)[2] - (*itrOfPointTemp)[2]) * ((*itrOfPoint)[2] - (*itrOfPointTemp)[2]));

			if (dDistanceOfPoint != 0)  //等于0就是它本身
			{
				lsDistanceCurrent.dDistance = dDistanceOfPoint;
				lsDistanceCurrent.pStart = (*itrOfPoint);
				lsDistanceCurrent.pEnd = (*itrOfPointTemp);

				if (lsDistanceCurrent.dDistance < lsLineOne.dDistance)
				{
					lsDistanceTemp.dDistance = lsLineOne.dDistance;
					lsDistanceTemp.pStart = lsLineOne.pStart;
					lsDistanceTemp.pEnd = lsLineOne.pEnd;

					lsLineOne.dDistance = lsDistanceCurrent.dDistance;
					lsLineOne.pStart = lsDistanceCurrent.pStart;
					lsLineOne.pEnd = lsDistanceCurrent.pEnd;

					lsDistanceCurrent.dDistance = lsDistanceTemp.dDistance;
					lsDistanceCurrent.pStart = lsDistanceTemp.pStart;
					lsDistanceCurrent.pEnd = lsDistanceTemp.pEnd;
				}
				
				if (lsDistanceCurrent.dDistance < lsLineTwo.dDistance)
				{
					lsDistanceTemp.dDistance = lsLineTwo.dDistance;
					lsDistanceTemp.pStart = lsLineTwo.pStart;
					lsDistanceTemp.pEnd = lsLineTwo.pEnd;

					lsLineTwo.dDistance = lsDistanceCurrent.dDistance;
					lsLineTwo.pStart = lsDistanceCurrent.pStart;
					lsLineTwo.pEnd = lsDistanceCurrent.pEnd;

					lsDistanceCurrent.dDistance = lsDistanceTemp.dDistance;
					lsDistanceCurrent.pStart = lsDistanceTemp.pStart;
					lsDistanceCurrent.pEnd = lsDistanceTemp.pEnd;
				}
				
				if (lsDistanceCurrent.dDistance < lsLineThree.dDistance)
				{
					lsDistanceTemp.dDistance = lsLineThree.dDistance;
					lsDistanceTemp.pStart = lsLineThree.pStart;
					lsDistanceTemp.pEnd = lsLineThree.pEnd;

					lsLineThree.dDistance = lsDistanceCurrent.dDistance;
					lsLineThree.pStart = lsDistanceCurrent.pStart;
					lsLineThree.pEnd = lsDistanceCurrent.pEnd;

					lsDistanceCurrent.dDistance = lsDistanceTemp.dDistance;
					lsDistanceCurrent.pStart = lsDistanceTemp.pStart;
					lsDistanceCurrent.pEnd = lsDistanceTemp.pEnd;
				}
			}
		}
		//*****lsLine-One/Two/Three 按照距离从小到大排列, vecModelDistance中每个模型的三个距离也请从小到大排列*****
		vecCurrentModel.push_back(lsLineOne);
		vecCurrentModel.push_back(lsLineTwo);
		vecCurrentModel.push_back(lsLineThree);
		vecAlternativeModels.push_back(vecCurrentModel);
		}

	//匹配备选模型，共vecModelDistance.Size个。
	std::vector<WxPoint3D> vecAcceptPoint;
	double dDistanceOfModel;
	double dMinDistanceOfModel;
	std::vector<std::vector<double>>::iterator itrOfModelDistance = vecModelDistance.begin();
	for (; itrOfModelDistance != vecModelDistance.end(); itrOfModelDistance++)
	{
		dMinDistanceOfModel = 99999;
		std::vector<std::vector<LineSegment>>::iterator itrOfAlternativeModels = vecAlternativeModels.begin();
		for (; itrOfAlternativeModels != vecAlternativeModels.end(); itrOfAlternativeModels++)
		{
			dDistanceOfModel = sqrt(((*itrOfModelDistance)[0] - (*itrOfAlternativeModels)[0].dDistance) * ((*itrOfModelDistance)[0] - (*itrOfAlternativeModels)[0].dDistance)
				+ ((*itrOfModelDistance)[1] - (*itrOfAlternativeModels)[1].dDistance) * ((*itrOfModelDistance)[1] - (*itrOfAlternativeModels)[1].dDistance)
				+ ((*itrOfModelDistance)[2] - (*itrOfAlternativeModels)[2].dDistance) * ((*itrOfModelDistance)[2] - (*itrOfAlternativeModels)[2].dDistance));

			if (dDistanceOfModel < dMinDistanceOfModel)
			{
				dMinDistanceOfModel = dDistanceOfModel;
				vecAcceptPoint.clear();
				vecAcceptPoint.push_back((*itrOfAlternativeModels)[0].pStart);
				vecAcceptPoint.push_back((*itrOfAlternativeModels)[0].pEnd);
				vecAcceptPoint.push_back((*itrOfAlternativeModels)[1].pEnd);
				vecAcceptPoint.push_back((*itrOfAlternativeModels)[2].pEnd);
			}
		}

		vecReturnPoint.push_back(vecAcceptPoint);
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	return 0;
}

std::vector<std::vector<WxPoint3D>> WxAutoExtractLandMarkFromCT::GetResult()
{
	return vecReturnPoint;
}