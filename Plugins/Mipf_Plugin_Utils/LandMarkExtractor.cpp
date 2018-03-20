#include "LandMarkExtractor.h"
#include <algorithm>
#include <numeric>


#include "mitkImage.h"
#include "mitkImageCast.h"


#include "itkRegionOfInterestImageFilter.h"
#include "itkLabelShapeOpeningImageFilter.h"
#include "itkBinaryShapeOpeningImageFilter.h"
#include "itkBinaryImageToShapeLabelMapFilter.h"
#include "itkBinaryThresholdImageFilter.h"

//compare by value function
int CmpByValue(const std::pair<int, double>& x, const std::pair<int, double>& y)
{
    return x.second < y.second;
}

//distance between two points
double PointDistance(itk::Point<double, 3> p1, itk::Point<double, 3> p2)
{
    return sqrt((p1[0] - p2[0])*(p1[0] - p2[0]) +
        (p1[1] - p2[1])*(p1[1] - p2[1]) +
        (p1[2] - p2[2])*(p1[2] - p2[2]));
}

bool FuzzyCompare(double a, double b, double error = 1.0)
{
    return abs(a - b) < error;
}

//Extract connected by attribute. Default is the sperical radius
template <class TInput, class TOutput, class TAttributeType = itk::ShapeLabelObject<int, 3>::AttributeType>
void ExtractConnectedComponents(TInput* input, TOutput* output, double size,
     bool reverse = false, TAttributeType attributeType = itk::ShapeLabelObject<int, 3>::EQUIVALENT_SPHERICAL_RADIUS)
{
    // Create a ShapeLabelMap from the image
    typedef itk::BinaryImageToShapeLabelMapFilter<TInput> BinaryImageToShapeLabelMapFilterType;
    BinaryImageToShapeLabelMapFilterType::Pointer binaryImageToShapeLabelMapFilter = BinaryImageToShapeLabelMapFilterType::New();
    binaryImageToShapeLabelMapFilter->SetFullyConnected(true);
    binaryImageToShapeLabelMapFilter->SetInputForegroundValue(1);
    binaryImageToShapeLabelMapFilter->SetInput(input);
    binaryImageToShapeLabelMapFilter->Update();

    // Remove label objects that have NUMBER_OF_PIXELS smaller than size
    typedef itk::ShapeOpeningLabelMapFilter< BinaryImageToShapeLabelMapFilterType::OutputImageType > ShapeOpeningLabelMapFilterType;
    ShapeOpeningLabelMapFilterType::Pointer shapeOpeningLabelMapFilter = ShapeOpeningLabelMapFilterType::New();
    shapeOpeningLabelMapFilter->SetInput(binaryImageToShapeLabelMapFilter->GetOutput());
    shapeOpeningLabelMapFilter->SetLambda(size);
    shapeOpeningLabelMapFilter->SetReverseOrdering(reverse);
    shapeOpeningLabelMapFilter->SetAttribute(attributeType);
    shapeOpeningLabelMapFilter->Update();

    // Create a label image
    typedef itk::LabelMapToLabelImageFilter<BinaryImageToShapeLabelMapFilterType::OutputImageType, TOutput> LabelMapToLabelImageFilterType;
    LabelMapToLabelImageFilterType::Pointer labelMapToLabelImageFilter = LabelMapToLabelImageFilterType::New();
    labelMapToLabelImageFilter->SetInput(shapeOpeningLabelMapFilter->GetOutput());
    labelMapToLabelImageFilter->Update();

    typedef itk::BinaryThresholdImageFilter<TOutput, TOutput> BinaryThresholdImageFilterType;
    BinaryThresholdImageFilterType::Pointer binaryThresholdFilter = BinaryThresholdImageFilterType::New();
    binaryThresholdFilter->SetInput(labelMapToLabelImageFilter->GetOutput());
    binaryThresholdFilter->SetLowerThreshold(1);
    binaryThresholdFilter->SetInsideValue(1);
    binaryThresholdFilter->SetOutsideValue(0);
    binaryThresholdFilter->Update();

    output->Graft(binaryThresholdFilter->GetOutput());
}

LandMarkPoint GetLandMarkByID(std::vector< LandMarkPoint > markers, int ID)
{
    for (int i = 0; i < markers.size(); i++)
    {
        if (markers.at(i).ID == ID)
        {
            return markers.at(i);
        }
    }
    return LandMarkPoint();
}


void LandMarkPoint::InsertDistanceMap(int id, double distance)
{
    std::pair<int, double> temp;
    temp.first = id;
    temp.second = distance;
    DistanceMap.push_back(temp);
}

bool LandMarkPoint::ContainDistance(double distance, std::pair<int, double>& output, double& variance, double error)
{
    double min = 1000;
    output.first = -1;
    output.second = min;
    for (DistanceMapType::iterator it = DistanceMap.begin(); it != DistanceMap.end(); it++)
    {
        if (min > abs((*it).second - distance))
        {
            min = abs((*it).second - distance);
            output = *it;
        }
    }
    if (ID != -1 && min < error)
    {
        variance = min*min;
        return true;
    }
    else
    {
        variance = -1;
        return false;
    }
}


void LandMarkPoint::SortDistanceMap()
{
    std::sort(DistanceMap.begin(), DistanceMap.end(), CmpByValue);
}

void LandMarkPoint::PrintSelf()
{
    std::cout <<std::endl << "ID:" << ID << std::endl;
    std::cout << "Coordinate:" << Coord[0] << ", " << Coord[1] << ", " << Coord[2] << std::endl;
    std::cout << "Centric Point:" << Centric << std::endl;
    std::cout << "Group Member:";
    for (GroupMemberType::iterator it = GroupMember.begin();it!=GroupMember.end();it++)
    {
        std::cout << (*it) << ",";
    }
    std::cout << std::endl;
    std::cout << "Distance Map:";
    for (DistanceMapType::iterator it = DistanceMap.begin(); it != DistanceMap.end(); it++)
    {
        std::cout << (*it).first << "--" << (*it).second << ",";
    }
    std::cout << std::endl;
}


LandMarkExtractor::LandMarkExtractor()
{
}


LandMarkExtractor::~LandMarkExtractor()
{
}

std::vector<LandMarkPoint> LandMarkExtractor::ExtractLandMarks(const mitk::Image* pInputImage, const std::vector< std::vector<double> >& vModelDistance,
    double dDiameter, int iMinThreshold, int iMaxThreshold, double dXCut, double dYCut, double dZCut)
{
    std::vector< LandMarkPoint > finalResult;
    if (!pInputImage)
    {
        return finalResult;
    }
    typedef itk::Image< int, 3 >        Int3DImageType;
    Int3DImageType::Pointer itkImage = Int3DImageType::New();
    mitk::CastToItkImage(pInputImage, itkImage);

    /**********Extract ROI************/
    typedef itk::RegionOfInterestImageFilter< Int3DImageType, Int3DImageType > roiImageFilterType;
    roiImageFilterType::Pointer roiFilter = roiImageFilterType::New();

    Int3DImageType::SizeType inSize = itkImage->GetLargestPossibleRegion().GetSize();
    Int3DImageType::IndexType start;
    start[0] = dXCut > 0 ? 0 : ((1 + dXCut)*inSize[0] - 1);
    start[1] = dYCut > 0 ? 0 : ((1 + dYCut)*inSize[1] - 1);
    start[2] = dZCut > 0 ? 0 : ((1 + dZCut)*inSize[2] - 1);

    Int3DImageType::SizeType size;
    size[0] = inSize[0] * abs(dXCut);
    size[1] = inSize[1] * abs(dYCut);
    size[2] = inSize[2] * abs(dZCut);

    Int3DImageType::RegionType desiredRegion;
    desiredRegion.SetSize(size);
    desiredRegion.SetIndex(start);

    roiFilter->SetRegionOfInterest(desiredRegion);
    roiFilter->SetInput(itkImage);
    roiFilter->Update();

    /**********Extract Interest Connected Components************/
    typedef itk::BinaryThresholdImageFilter<Int3DImageType, Int3DImageType> btImageFilterType;
    btImageFilterType::Pointer btFilter = btImageFilterType::New();
    btFilter->SetInput(roiFilter->GetOutput());
    btFilter->SetLowerThreshold(iMinThreshold);
    btFilter->SetUpperThreshold(iMaxThreshold);
    btFilter->SetInsideValue(1);
    btFilter->SetOutsideValue(0);
    btFilter->Update();

    ExtractConnectedComponents<Int3DImageType, Int3DImageType>(btFilter->GetOutput(), itkImage.GetPointer(), dDiameter, true);
    ExtractConnectedComponents<Int3DImageType, Int3DImageType>(itkImage.GetPointer(), itkImage.GetPointer(), dDiameter / 4.0);

    typedef itk::BinaryImageToShapeLabelMapFilter<Int3DImageType> BinaryImageToShapeLabelMapFilterType;
    BinaryImageToShapeLabelMapFilterType::Pointer binaryImageToShapeLabelMapFilter = BinaryImageToShapeLabelMapFilterType::New();
    binaryImageToShapeLabelMapFilter->SetInputForegroundValue(1);
    binaryImageToShapeLabelMapFilter->SetInput(itkImage);
    binaryImageToShapeLabelMapFilter->Update();

    //**********Get Center Points************/
    std::vector< LandMarkPoint > landMarkPoints;
    for (unsigned int i = 0; i < binaryImageToShapeLabelMapFilter->GetOutput()->GetNumberOfLabelObjects(); i++)
    {
        BinaryImageToShapeLabelMapFilterType::OutputImageType::LabelObjectType* labelObject = binaryImageToShapeLabelMapFilter->GetOutput()->GetNthLabelObject(i);
        itk::Point<double, 3> center = labelObject->GetCentroid();
        LandMarkPoint landMark;
        landMark.Coord = center;
        landMark.ID = i;
        landMarkPoints.push_back(landMark);
    }
    

    //**********Calculate Distance Map***********///
    for (int i = 0; i < landMarkPoints.size(); i++)
    {
        for (int j = i + 1; j < landMarkPoints.size(); j++)
        {
            double distance = PointDistance(landMarkPoints[i].Coord, landMarkPoints[j].Coord);
            landMarkPoints[i].InsertDistanceMap(j, distance);
            landMarkPoints[j].InsertDistanceMap(i, distance);
        }
    }
  /*  for (int  i=0; i < landMarkPoints.size(); i++)
    {
        landMarkPoints[i].PrintSelf();
    }*/
  // return  landMarkPoints;

    //**********Fit Point***********////
    double error = 0.5;
    for (int i = 0; i < landMarkPoints.size(); i++)
    {
        for (int j = 0; j < vModelDistance.size(); j++)
        {
            bool fitted = true;
            std::vector< std::pair<int, double> >  maps;
            std::vector<double> vs;
            for (int k=0;k<vModelDistance[j].size();k++)
            {
                std::pair<int, double> map;
                double v;
                fitted &= landMarkPoints[i].ContainDistance(vModelDistance[j][k], map, v, error);
                maps.push_back(map);
                vs.push_back(v);
            }
            if (fitted)
            {
                for (std::vector< std::pair<int, double> >::iterator it=maps.begin();it!=maps.end();it++)
                {
                    landMarkPoints[i].GroupMember.push_back(it->first);
                }
                landMarkPoints[i].CentricGrade = 1.0 / (std::accumulate(vs.begin(),vs.end(),0.0));
                landMarkPoints[i].Centric = true;
                landMarkPoints[i].GroupID = j;
            }
        }
    }

    //**********Find The Most Matched Centric Point In Each Group***********///
    for (int i = 0; i < vModelDistance.size(); i++)
    {
        LandMarkPoint mostMatchedCentric;
        for (int j = 0; j < landMarkPoints.size(); j++)
        {
            if (landMarkPoints[j].Centric&&landMarkPoints[j].GroupID == i)
            {
                if (mostMatchedCentric.CentricGrade < landMarkPoints[j].CentricGrade)
                {
                    mostMatchedCentric = landMarkPoints[j];
                }
            }
        }
        if (mostMatchedCentric.Centric&&mostMatchedCentric.GroupID == i)
        {
            finalResult.push_back(mostMatchedCentric);
            for (int k = 0; k < mostMatchedCentric.GroupMember.size(); k++)
            {
                LandMarkPoint p = GetLandMarkByID(landMarkPoints, mostMatchedCentric.GroupMember.at(k));
                p.GroupID = i;
                finalResult.push_back(p);
            }
        }
    }
    return finalResult;
}
