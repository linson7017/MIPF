#include "SliceBySliceBlobTracking.h"
#include <vtkExtractVOI.h>
#include <vtkImageShiftScale.h>

//opencv
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/types_c.h>





using namespace cv;

SliceBySliceBlobTracking::SliceBySliceBlobTracking()
{
    _refDistance2 = 10;
}

SliceBySliceBlobTracking::~SliceBySliceBlobTracking()
{
    _inputImage->Delete();
}


void SliceBySliceBlobTracking::SetInputImage(ImageType image)
{
    if (image)
    {
        vtkImageData* vtkImage = (vtkImageData*)image.Get();
        //rescale the image scale range to [0-255]
        vtkSmartPointer<vtkImageShiftScale> shiftScaleFilter =
            vtkSmartPointer<vtkImageShiftScale>::New();
        shiftScaleFilter->SetInputData(vtkImage);
        shiftScaleFilter->SetOutputScalarTypeToUnsignedChar();
        shiftScaleFilter->SetShift(-1.0f * vtkImage->GetScalarRange()[0]);
        float oldRange = vtkImage->GetScalarRange()[1] - vtkImage->GetScalarRange()[0];
        float newRange = 255;
        shiftScaleFilter->SetScale(newRange / oldRange);
        shiftScaleFilter->Update();

        _inputImage = vtkImageData::New();
        _inputImage->DeepCopy(shiftScaleFilter->GetOutput());
    }
}

void SliceBySliceBlobTracking::SetSeedPoints(XMarkerList& list)
{
    _initPointsXMarkerList = list;
}

void SliceBySliceBlobTracking::fitOneSlice(int sliceIndex, int startSlice, int* extent, std::vector<Vector3> refPoints, VTKImageProperties& mp)
{
    vtkSmartPointer<vtkExtractVOI> extractor = vtkSmartPointer<vtkExtractVOI>::New();
    extractor->SetInputData(_inputImage);
    extractor->SetVOI(extent[0], extent[1], extent[2], extent[3], sliceIndex, sliceIndex);
    extractor->Update();
    vtkImageData* sliceImage = extractor->GetOutput();

    //copy image value to cv Mat
    cv::Mat mat(abs(extent[1] - extent[0]), abs(extent[3] - extent[2]), CV_32FC1);
    mat.setTo(Scalar(0));
    Scalar vv = mat.at<float>(1, 1);
    for (size_t j = extent[0]; j < extent[1]; j++)
    {
        for (size_t k = extent[2]; k < extent[3]; k++)
        {
            float value = sliceImage->GetScalarComponentAsFloat(j, k, sliceIndex, 0);
            //note: the cv mat is organized using row and colume, so we should transpose the image to match it
            mat.at<float>(k, j) = value;
        }
    }
    //change image pixel type to x8 char
    Mat image;
    mat.convertTo(image, CV_8UC1);
    //use cv simple blob feature to detect the interesting points on crw
    Ptr<FeatureDetector> blobsDetector = FeatureDetector::create("SimpleBlob");
    vector<KeyPoint> keypoints;
    blobsDetector->detect(image, keypoints);

    //filter the points far from the seed points and add extracted points to graphs
    for (int i = 0; i < refPoints.size(); i++)
    {
        for (int j = 0; j < keypoints.size(); j++)
        {
            //convert the extracted point from 2D to 3D
            Vector3 keyPoint = Vector3(keypoints.at(j).pt.x + extent[0], keypoints.at(j).pt.y + extent[2], sliceIndex);
            if (sliceIndex == startSlice)
            {
                if (filterPoint(keyPoint, refPoints.at(i), 4 * _refDistance2))
                {
                    refPoints.at(i) = keyPoint;
                    _trackedGraphs.at(i).push_back(mp.mapVoxelToWorld(keyPoint));
                    break;
                }
            }
            else
            {
                if (filterPoint(keyPoint, refPoints.at(i), _refDistance2))
                {
                    refPoints.at(i) = keyPoint;
                    _trackedGraphs.at(i).push_back(mp.mapVoxelToWorld(keyPoint));
                    break;
                }
            }
        }

    }

    Mat drawImage = mat.clone();
    for (size_t i = 0; i < keypoints.size(); ++i)
        circle(drawImage, keypoints[i].pt, 2, Scalar(255, 0, 255), -1);

    std::string sliceName = "D:/SliceImage/imagelek";
    char index[16];
    itoa(sliceIndex, index, 10);
    sliceName.append(index);
    cv::imwrite(sliceName.append(".jpg"), drawImage);
}


void SliceBySliceBlobTracking::Track()
{
    if (!_inputImage)
    {
        return;
    }
    if (_initPointsXMarkerList.size() < 1)
    {
        return;
    }
    int step = 1;
    int extent[6];
    _inputImage->GetExtent(extent);

    std::vector<Vector3> initRefPoints;
    initRefPoints.resize(_initPointsXMarkerList.size());
    _trackedGraphs.resize(_initPointsXMarkerList.size());

    VTKImageProperties mp;
    mp.setImageProperties(_inputImage);

    for (int i = 0; i < _initPointsXMarkerList.size(); i++)
    {
        Vector3 pointIndex = mp.mapWorldToVoxel(Vector3(_initPointsXMarkerList.getItemAt(i).x(), _initPointsXMarkerList.getItemAt(i).y(), _initPointsXMarkerList.getItemAt(i).z()));
        initRefPoints[i] = pointIndex;
    }
    //get the start slice index
    int startSlice = initRefPoints.at(0).z();

    //track both direction
    for (int sliceIndex = startSlice; sliceIndex <= extent[5]; sliceIndex += step)
    {
        fitOneSlice(sliceIndex, startSlice, extent, initRefPoints, mp);
    }
    //set ref points to init and track the reversed direction
    for (int sliceIndex = startSlice - step; sliceIndex > extent[4]; sliceIndex -= step)
    {
        fitOneSlice(sliceIndex, startSlice, extent, initRefPoints, mp);
    }
}

bool SliceBySliceBlobTracking::filterPoint(Vector3& p1, Vector3& p2, double dis)
{
    double distance = (p1 - p2).length();
    return (p1 - p2).lengthSquared() < dis;
}