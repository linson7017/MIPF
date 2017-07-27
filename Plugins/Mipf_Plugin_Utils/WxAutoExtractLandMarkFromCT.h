#ifndef WxAutoExtractLandMarkFromCT_h__
#define WxAutoExtractLandMarkFromCT_h__

#pragma once

#include <mitkDataNode.h>

typedef mitk::Point3D WxPoint3D;

class  WxAutoExtractLandMarkFromCT
{
public:

	struct LineSegment
	{
		double dDistance;
		WxPoint3D pStart;
		WxPoint3D pEnd;
	};

	WxAutoExtractLandMarkFromCT();
	~WxAutoExtractLandMarkFromCT();

	//vecModelDistance向量中每组4个钢珠
	//周围3个点到中心点的球心距离，std::vector<double>为一组3个
	//dSPVDistance为钢珠在CT下的体素大小
	//sMaxThreshold\sMinThreshold为钢珠的最大\最小CT值
	//dxCut, dyCut, dzCut为识别的图像区域范围
	int FindModelPoints(mitk::DataNode::Pointer mainNode, std::vector<std::vector<double>>  vecModelDistance, 
									double dSPVDistance, short sMaxThreshold, short sMinThreshold,
									double dxCut, double dyCut, double dzCut);

	std::vector<std::vector<WxPoint3D>> GetResult();
	
private:
	double DistanceBetweenPointAndModel(std::vector<WxPoint3D> vecModel, WxPoint3D pPoint);
	double MaxDistanceOfModel(std::vector<WxPoint3D> vecModel);
	WxPoint3D CalculateCenterPoint(std::vector<WxPoint3D> vecModel);

private:
	short sVixelValueThresholdMax;		//标志点最大CT值
	short sVixelValueThresholdMin;		//标志点最小CT值
	double dSamePointVixelDistance;    //标志点体素大小

	double dDistanceTemp;
    WxPoint3D pReturnPointTemp;
	std::vector<WxPoint3D> vecSpherePointMM;   //世界坐标
	bool bFoundHome;

	//std::vector<WxPoint3D>为一组大小为4，存储4个钢珠的世界坐标
	//第1点为中心点，后面3个点按照输入的球心距离vecModelDistance顺序排列
	//vecReturnPoint的大小即为体表所贴模型的组数
	std::vector<std::vector<WxPoint3D>>  vecReturnPoint;
};
#endif // WxAutoExtractLandMarkFromCT_h__