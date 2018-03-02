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
        _inputImage = vtkImageData::New();
        _inputImage->DeepCopy(vtkImage);
    }
}

void SliceBySliceBlobTracking::SetSeedPoints(XMarkerList& list)
{
    _initPointsXMarkerList = list;
}

void SliceBySliceBlobTracking::fitOneSliceByBrightestPixel(int sliceIndex, int startSlice, int* extent, std::vector<Vector3>& refPoints, VTKImageProperties& mp, std::vector<Vector3>& directs)
{
    vtkSmartPointer<vtkExtractVOI> extractor = vtkSmartPointer<vtkExtractVOI>::New();
    extractor->SetInputData(_inputImage);
    extractor->SetVOI(extent[0], extent[1], extent[2], extent[3], sliceIndex, sliceIndex);
    extractor->Update();
    vtkImageData* sliceImage = extractor->GetOutput();

    //copy image value to cv Mat
    cv::Mat mat(abs(extent[1] - extent[0]), abs(extent[3] - extent[2]), CV_32FC1);
    mat.setTo(cv::Scalar(0));
    for (int j = extent[0]; j < extent[1]; j++)
    {
        for (int k = extent[2]; k < extent[3]; k++)
        {
            float value = sliceImage->GetScalarComponentAsFloat(j, k, sliceIndex, 0);
            //note: the cv mat is organized using row and colume, so we should transpose the image to match it
            mat.at<float>(k, j) = value;
        }
    }
    //change image pixel type to x8 char
    //cv::Mat image;
   // mat.convertTo(mat, CV_8UC1); 
    const double Threshold = 200;
    for (int i = 0; i < refPoints.size(); i++)
    {
        Vector3 patchCenter = refPoints.at(i);

        int PatchSizeXHalf = 15;
        int PatchSizeYHalf = 30;
        if ((patchCenter.x()-PatchSizeXHalf<0) || (patchCenter.x() + PatchSizeXHalf>mat.cols-1))
        {
            PatchSizeXHalf = patchCenter.x() > mat.cols - 1 - patchCenter.x() ? mat.cols - 1 - patchCenter.x() : patchCenter.x();
        }
        if ((patchCenter.y() - PatchSizeYHalf<0) || (patchCenter.y() + PatchSizeYHalf>mat.rows - 1))
        {
            PatchSizeYHalf = patchCenter.y() > mat.rows - 1 - patchCenter.y() ? mat.rows - 1 - patchCenter.y() : patchCenter.y();
        }
        cv::Point2i patchCorner(patchCenter.x() - PatchSizeXHalf, patchCenter.y() - PatchSizeYHalf);
        cv::Rect rect(patchCorner, cv::Size2i(2 * PatchSizeXHalf, 2 * PatchSizeYHalf));
        cv::Mat patch = mat(rect);
        //cv::GaussianBlur(patch, smoothPatch, cv::Size(3, 3), 1, 1);
       // cv::medianBlur(patch, smoothPatch, 5);

        cv::Mat binaryImage;
        //cv::normalize(patch, patch,1.0, 0.0, cv::NORM_MINMAX);
        double maxValue;
        cv::Point2i maxIndex;
        cv::minMaxLoc(patch, 0, &maxValue, 0, &maxIndex);
        //cv::threshold(patch, binaryImage, Threshold, 255, CV_THRESH_BINARY);
        //binaryImage.convertTo(binaryImage, CV_8UC1);
        Vector3 keyPoint(maxIndex.x + rect.x + extent[0], maxIndex.y + rect.y + extent[2], sliceIndex);
        if (filterPoint(keyPoint, refPoints.at(i), _refDistance2))
        {
            refPoints.at(i) = keyPoint;
            _trackedGraphs.at(i).push_back(mp.mapVoxelToWorld(keyPoint));
        }
        continue;
        //////////////////////
       //std::vector<std::vector<cv::Point>> contours;
       //cv::findContours(binaryImage, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
       //if (contours.size()==0)
       //{
       //    continue;
       //}
       //double maxArea = 0;
       //std::vector<cv::Point> maxContour;
       //for (size_t i = 0; i < contours.size(); i++)
       //{
       //    double area = cv::contourArea(contours[i]);
       //    if (area > maxArea)
       //    {
       //        maxArea = area;
       //        maxContour = contours[i];
       //    }
       //}
       //
       //if (maxContour.size()<4)
       //{
       //    continue;
       //}
       //cv::Rect maxRect = cv::boundingRect(maxContour);
       //cv::Point maxPos;
       //maxPos.x = maxRect.x + maxRect.width / 2;
       //maxPos.y = maxRect.y + maxRect.height / 2;
       ////Vector3 keyPoint(maxPos.x + rect.x +extent[0], maxPos.y+ rect.y + extent[2], sliceIndex);
       //if (sliceIndex==startSlice)
       //{
       //    refPoints.at(i) = keyPoint;
       //    _trackedGraphs.at(i).push_back(mp.mapVoxelToWorld(keyPoint));        
       //}
       //else
       //{
       //    if (directs.at(i).isNull())
       //    {
       //        directs.at(i) = keyPoint - refPoints.at(i);
       //        directs.at(i).normalize();
       //        refPoints.at(i) = keyPoint;
       //        _trackedGraphs.at(i).push_back(mp.mapVoxelToWorld(keyPoint));
       //    }
       //    else
       //    {
       //        Vector3 cd = keyPoint - p;
       //        cd.normalize();
       //        if (abs(Vector3::dotProduct(cd, directs.at(i))) > 0.6)
       //        {
       //            refPoints.at(i) = keyPoint;
       //            _trackedGraphs.at(i).push_back(mp.mapVoxelToWorld(keyPoint));
       //            directs.at(i) = cd;
       //        }
       //    }
        //}
       
    }

    //Mat drawImage = mat.clone();
    //for (size_t i = 0; i < refPoints.size(); ++i)
    //{
    //    cv::Point2d p(refPoints[i].x(), refPoints[i].y());
    //    circle(drawImage, p, 2, Scalar(255, 0, 255), -1);
    //}
    //
    //std::string sliceName = "D:/SliceImage/imagelek";
    //char index[16];
    //itoa(sliceIndex, index, 10);
    //sliceName.append(index);
    //cv::imwrite(sliceName.append(".jpg"), drawImage);
}

void SliceBySliceBlobTracking::fitOneSlice(int sliceIndex, int startSlice, int* extent, std::vector<Vector3> refPoints, VTKImageProperties& mp)
{
    vtkSmartPointer<vtkExtractVOI> extractor = vtkSmartPointer<vtkExtractVOI>::New();
    extractor->SetInputData(_inputImage);
    extractor->SetVOI(extent[0], extent[1], extent[2], extent[3], sliceIndex, sliceIndex);
    extractor->Update();
    //rescale the image scale range to [0-255]
    vtkSmartPointer<vtkImageShiftScale> shiftScaleFilter =
        vtkSmartPointer<vtkImageShiftScale>::New();
    shiftScaleFilter->SetInputData(extractor->GetOutput());
    shiftScaleFilter->SetOutputScalarTypeToUnsignedChar();
    shiftScaleFilter->SetShift(-1.0f * extractor->GetOutput()->GetScalarRange()[0]);
    float oldRange = extractor->GetOutput()->GetScalarRange()[1] - extractor->GetOutput()->GetScalarRange()[0];
    float newRange = 255;
    shiftScaleFilter->SetScale(newRange / oldRange);
    shiftScaleFilter->Update();
    vtkImageData* sliceImage = shiftScaleFilter->GetOutput();

    //copy image value to cv Mat
    cv::Mat mat(abs(extent[1] - extent[0]), abs(extent[3] - extent[2]), CV_32FC1);
    mat.setTo(cv::Scalar(0));
    cv::Scalar vv = mat.at<float>(1, 1);
    for (int j = extent[0]; j < extent[1]; j++)
    {
        for (int k = extent[2]; k < extent[3]; k++)
        {
            float value = sliceImage->GetScalarComponentAsFloat(j, k, sliceIndex, 0);
            //note: the cv mat is organized using row and colume, so we should transpose the image to match it
            mat.at<float>(k, j) = value;
        }
    }
    //change image pixel type to x8 char
    //cv::Mat image;
    mat.convertTo(mat, CV_8UC1);
    //use cv simple blob feature to detect the interesting points on crw
    //Ptr<FeatureDetector> blobsDetector = FeatureDetector::create("SimpleBlob");
    //vector<KeyPoint> keypoints;
    //blobsDetector->detect(image, keypoints);

   // vector<Vector3> points;
    for (int i = 0; i < refPoints.size(); i++)
    {
        Vector3 patchCenter = refPoints.at(i);
        int PatchSizeXHalf = 25;
        int PatchSizeYHalf = 25;
        if ((patchCenter.x() - PatchSizeXHalf < 0) || (patchCenter.x() + PatchSizeXHalf > mat.cols - 1))
        {
            PatchSizeXHalf = patchCenter.x() > mat.cols - 1 - patchCenter.x() ? mat.cols - 1 - patchCenter.x() : patchCenter.x();
        }
        if ((patchCenter.y() - PatchSizeYHalf < 0) || (patchCenter.y() + PatchSizeYHalf > mat.rows - 1))
        {
            PatchSizeYHalf = patchCenter.y() > mat.rows - 1 - patchCenter.y() ? mat.rows - 1 - patchCenter.y() : patchCenter.y();
        }
        cv::Point2i patchCorner(patchCenter.x() - PatchSizeXHalf, patchCenter.y() - PatchSizeYHalf);
        cv::Rect rect(patchCorner, cv::Size2i(2 * PatchSizeXHalf, 2 * PatchSizeYHalf));
        cv::Mat patch = mat(rect);

        std::vector<cv::KeyPoint> keypoints;
        cv::SimpleBlobDetector::Params params;
        params.minThreshold = 0;
        params.maxThreshold = 160;
        params.minArea = 16;
        params.maxArea = 256;
        cv::SimpleBlobDetector detector(params);
        detector.detect(patch, keypoints);
        /*cv::Ptr<cv::FeatureDetector> blobsDetector = cv::FeatureDetector::create("SimpleBlob");
        blobsDetector->detect(patch, keypoints);*/

        for (int j=0;j<keypoints.size();j++)
        {
            Vector3 keyPoint(keypoints[j].pt.x + rect.x + extent[0], keypoints[j].pt.y + rect.y + extent[2], sliceIndex);
           // points.push_back(keyPoint);
            if (filterPoint(keyPoint, refPoints.at(i), _refDistance2))
            {
                refPoints.at(i) = keyPoint;
                _trackedGraphs.at(i).push_back(mp.mapVoxelToWorld(keyPoint));
            }
        }
    }

    /*Mat drawImage = mat.clone();
    for (size_t i = 0; i < points.size(); ++i)
    {
        cv::Point2d p(points[i].x(), points[i].y());
        circle(drawImage, p, 2, Scalar(255, 0, 255), -1);
    }

    std::string sliceName = "D:/SliceImage/imagelek";
    char index[16];
    itoa(sliceIndex, index, 10);
    sliceName.append(index);
    cv::imwrite(sliceName.append(".jpg"), drawImage);*/

    //filter the points far from the seed points and add extracted points to graphs
    //for (int i = 0; i < refPoints.size(); i++)
    //{
    //    for (int j = 0; j < keypoints.size(); j++)
    //    {
    //        //convert the extracted point from 2D to 3D
    //        Vector3 keyPoint = Vector3(keypoints.at(j).pt.x + extent[0], keypoints.at(j).pt.y + extent[2], sliceIndex);
    //        if (sliceIndex == startSlice)
    //        {
    //            if (filterPoint(keyPoint, refPoints.at(i), 4 * _refDistance2))
    //            {
    //                refPoints.at(i) = keyPoint;
    //                _trackedGraphs.at(i).push_back(mp.mapVoxelToWorld(keyPoint));
    //                break;
    //            }
    //        }
    //        else
    //        {
    //            if (filterPoint(keyPoint, refPoints.at(i), _refDistance2))
    //            {
    //                refPoints.at(i) = keyPoint;
    //                _trackedGraphs.at(i).push_back(mp.mapVoxelToWorld(keyPoint));
    //                break;
    //            }
    //        }
    //    }

    //}
}


void SliceBySliceBlobTracking::Track(int trackMode)
{

    if (trackMode == BLOB_TRACK)
    {
        printf("Slice by slice tracking by blob detection !\n");
    }
    else
    {
        printf("Slice by slice tracking by brightest pixel detection !\n");
    }

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
    std::vector<Vector3> tempRefPoints = initRefPoints;
    std::vector<Vector3> directs;
    directs.resize(initRefPoints.size(), Vector3());
    for (int sliceIndex = startSlice; sliceIndex <= extent[5]; sliceIndex += step)
    {
        if (trackMode ==BLOB_TRACK)
        {
            fitOneSlice(sliceIndex, startSlice, extent, tempRefPoints, mp);
        }
        else
        {
            fitOneSliceByBrightestPixel(sliceIndex, startSlice, extent, tempRefPoints, mp, directs);
        }
    }
    //set ref points to init and track the reversed direction
    tempRefPoints.clear();
    tempRefPoints = initRefPoints;
    directs.resize(initRefPoints.size(), Vector3());
    for (int sliceIndex = startSlice - step; sliceIndex > extent[4]; sliceIndex -= step)
    {
        if (trackMode == BLOB_TRACK)
        {
            fitOneSlice(sliceIndex, startSlice, extent, tempRefPoints, mp);
        }
        else
        {
            fitOneSliceByBrightestPixel(sliceIndex, startSlice, extent, tempRefPoints, mp, directs);
        }
    }
}

bool SliceBySliceBlobTracking::filterPoint(Vector3& p1, Vector3& p2, double dis)
{
    return (p1 - p2).length() < dis;
}