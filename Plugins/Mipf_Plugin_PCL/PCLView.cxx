#include "PCLView.h" 
#include "iqf_main.h" 

#include "mitkPointSet.h"


#include <pcl/console/parse.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/registration/icp.h>
#include <pcl/registration/icp_nl.h>
#include <pcl/registration/transformation_estimation_lm.h>
#include <pcl/registration/warp_point_rigid_3d.h>
#include <pcl/io/vtk_lib_io.h>
#include <pcl/registration/incremental_registration.h>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

typedef pcl::PointXYZ PointType;
typedef pcl::PointCloud<PointType> Cloud;
typedef Cloud::ConstPtr CloudConstPtr;
typedef Cloud::Ptr CloudPtr;
  
PCLView::PCLView() :MitkPluginView() 
{
}
 
PCLView::~PCLView() 
{
}
 
void PCLView::CreateView()
{
    m_ui.setupUi(this);

    m_ui.FixedPointSetSelector->SetDataStorage(GetDataStorage());
    m_ui.MovingPointSetSelector->SetDataStorage(GetDataStorage());

    m_ui.FixedPointSetSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Surface>::New());
    m_ui.MovingPointSetSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Surface>::New());


    connect(m_ui.ApplyBtn, &QPushButton::clicked, this, &PCLView::Apply);
} 
 
WndHandle PCLView::GetPluginHandle() 
{
    return this; 
}

void PCLView::Apply()
{
    //////////////////////////////////convert vtk pointset to pcl point cloud//////////////////////////////////////////////////////////////////////////
    mitk::Surface* fixedPointSet = dynamic_cast<mitk::Surface*>(m_ui.FixedPointSetSelector->GetSelectedNode()->GetData());
    mitk::Surface* movingPointSet = dynamic_cast<mitk::Surface*>(m_ui.MovingPointSetSelector->GetSelectedNode()->GetData());
    if (!fixedPointSet||!movingPointSet)
    {
        return;
    }
    CloudPtr fixedCloud(new Cloud);
    pcl::io::vtkPolyDataToPointCloud(fixedPointSet->GetVtkPolyData(), *fixedCloud);

    CloudPtr movingCloud(new Cloud);
    pcl::io::vtkPolyDataToPointCloud(movingPointSet->GetVtkPolyData(), *movingCloud);


    double dist = m_ui.McdLE->text().toDouble();
    double rans = m_ui.RansLE->text().toDouble();
    int iter = m_ui.IteratorTimesLE->text().toInt();
    bool nonLinear = m_ui.LinearCB->isChecked();

    //init icp
    pcl::IterativeClosestPoint<PointType, PointType>::Ptr icp;
    //boost::shared_ptr<pcl::registration::WarpPointRigid3D<PointType, PointType> > warp_fcn
    //(new pcl::registration::WarpPointRigid3D<PointType, PointType>);
    //// Create a TransformationEstimationLM object, and set the warp to it
    //boost::shared_ptr<pcl::registration::TransformationEstimationLM<PointType, PointType> > te(new pcl::registration::TransformationEstimationLM<PointType, PointType>);
    //te->setWarpFunction(warp_fcn);
    // Pass the TransformationEstimation objec to the ICP algorithm
    if (nonLinear)
    {
        std::cout << "Using IterativeClosestPointNonLinear" << std::endl;
        icp.reset(new pcl::IterativeClosestPointNonLinear<PointType, PointType>());
    }
    else
    {
        std::cout << "Using IterativeClosestPoint" << std::endl;
        icp.reset(new pcl::IterativeClosestPoint<PointType, PointType>());
    }
   // icp->setTransformationEstimation(te);
    icp->setMaximumIterations(iter);
    icp->setMaxCorrespondenceDistance(dist);
    icp->setRANSACOutlierRejectionThreshold(rans);
    icp->setInputTarget(fixedCloud);
    icp->setInputSource(movingCloud);

    //register
    Eigen::Matrix4f t(Eigen::Matrix4f::Identity());
    Eigen::Matrix4f guess(Eigen::Matrix4f::Identity());
    CloudPtr tmp(new Cloud);
    icp->align(*tmp, guess);
    t = t * icp->getFinalTransformation();
    //pcl::transformPointCloud(*movingCloud, *tmp, t);
    std::cout << icp->getFinalTransformation() << std::endl;

    //display transfrom matrix
    QString matrixStr = QString("%1 %2 %3 %4\n%5 %6 %7 %8\n%9 %10 %11 %12\n%13 %14 %15 %16")
        .arg(guess(0, 0)).arg(guess(0, 1)).arg(guess(0, 2)).arg(guess(0, 3))
        .arg(guess(1, 0)).arg(guess(1, 1)).arg(guess(1, 2)).arg(guess(1, 3))
        .arg(guess(2, 0)).arg(guess(2, 1)).arg(guess(2, 2)).arg(guess(2, 3))
        .arg(guess(3, 0)).arg(guess(3, 1)).arg(guess(3, 2)).arg(guess(3, 3));
    m_ui.TransformMatrixTE->setPlainText(matrixStr);


    //save result
    mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
    mitk::Surface::Pointer resultData = mitk::Surface::New();
    
    auto resutlVtkData = vtkSmartPointer<vtkPolyData>::New();
    pcl::io::pointCloudTovtkPolyData(*tmp, resutlVtkData);
    resultData->SetVtkPolyData(resutlVtkData);
    resultNode->SetData(resultData);
    resultNode->SetName("Result");
    resultNode->SetColor(1.0, 0.0, 0.0);
    GetDataStorage()->Add(resultNode, m_ui.MovingPointSetSelector->GetSelectedNode());
    RequestRenderWindowUpdate();

}