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

	//vecModelDistance������ÿ��4������
	//��Χ3���㵽���ĵ�����ľ��룬std::vector<double>Ϊһ��3��
	//dSPVDistanceΪ������CT�µ����ش�С
	//sMaxThreshold\sMinThresholdΪ��������\��СCTֵ
	//dxCut, dyCut, dzCutΪʶ���ͼ������Χ
	int FindModelPoints(mitk::DataNode::Pointer mainNode, std::vector<std::vector<double>>  vecModelDistance, 
									double dSPVDistance, short sMaxThreshold, short sMinThreshold,
									double dxCut, double dyCut, double dzCut);

	std::vector<std::vector<WxPoint3D>> GetResult();
	
private:
	double DistanceBetweenPointAndModel(std::vector<WxPoint3D> vecModel, WxPoint3D pPoint);
	double MaxDistanceOfModel(std::vector<WxPoint3D> vecModel);
	WxPoint3D CalculateCenterPoint(std::vector<WxPoint3D> vecModel);

private:
	short sVixelValueThresholdMax;		//��־�����CTֵ
	short sVixelValueThresholdMin;		//��־����СCTֵ
	double dSamePointVixelDistance;    //��־�����ش�С

	double dDistanceTemp;
    WxPoint3D pReturnPointTemp;
	std::vector<WxPoint3D> vecSpherePointMM;   //��������
	bool bFoundHome;

	//std::vector<WxPoint3D>Ϊһ���СΪ4���洢4���������������
	//��1��Ϊ���ĵ㣬����3���㰴����������ľ���vecModelDistance˳������
	//vecReturnPoint�Ĵ�С��Ϊ�������ģ�͵�����
	std::vector<std::vector<WxPoint3D>>  vecReturnPoint;
};
#endif // WxAutoExtractLandMarkFromCT_h__