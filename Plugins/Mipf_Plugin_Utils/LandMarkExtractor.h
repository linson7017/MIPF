#ifndef LandMarkExtractor_h__
#define LandMarkExtractor_h__

#pragma once

#include "itkPoint.h"

namespace mitk
{
    class Image;
}

struct LandMarkPoint
{
    int ID = -1;      //���ID  >0
    int GroupID = -1;       //������Group��ID       >0
    itk::Point<double, 3>  Coord;   //�������
    bool Centric = false;  //�Ƿ�Ϊ������ĵ�

    typedef std::vector<std::pair<int, double> > DistanceMapType; 
    DistanceMapType DistanceMap;      //����������ľ���

    typedef std::vector<int> GroupMemberType;
    GroupMemberType GroupMember;       //���������

    double CentricGrade = 0.0;       //��Ϊ�����ĵ�����ֵ���ɷ���ĵ�������

    void InsertDistanceMap(int id, double distance);    
    void SortDistanceMap();
    bool ContainDistance(double distance, std::pair<int, double>& output, double& variance, double error = 1.0);
    void PrintSelf();
};


class LandMarkExtractor
{
public:
    LandMarkExtractor();
    ~LandMarkExtractor();
    static std::vector<LandMarkPoint> ExtractLandMarks(const mitk::Image* pInputImage, const std::vector< std::vector<double> >& vModelDistance,
        double dDiameter, int iMinThreshold = 1800, int iMaxThreshold=5000, double dXCut=1.0, double dYCut=0.7, double dZCut=1.0);
};

#endif // LandMarkExtractor_h__
