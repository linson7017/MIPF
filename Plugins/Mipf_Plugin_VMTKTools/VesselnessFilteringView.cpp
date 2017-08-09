#include "VesselnessFilteringView.h"


//vtk
#include <vtkImageConstantPad.h>
#include <vtkImageCast.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>

#include  <vmtk/vtkvmtkVesselnessMeasureImageFilter.h>


VesselnessFilteringView::VesselnessFilteringView()
{
}


VesselnessFilteringView::~VesselnessFilteringView()
{
}

void VesselnessFilteringView::CreateView()
{
    m_ui.setupUi(this);

    m_ui.ImageSelector->SetDataStorage(GetDataStorage());
    m_ui.ImageSelector->SetPredicate(CreatePredicate(1));

    m_seeds = mitk::PointSet::New();

    mitk::DataNode::Pointer seedNode = mitk::DataNode::New();
    seedNode->SetData(m_seeds);
    seedNode->SetProperty("helper object", mitk::BoolProperty::New(true));
    GetDataStorage()->Add(seedNode);
    m_ui.SeedPointList->SetPointSetNode(seedNode);
    m_ui.SeedPointList->SetPointSet(m_seeds);
    m_ui.SeedPointList->SetMultiWidget(m_pMitkRenderWindow->GetMitkStdMultiWidget());
}

void VesselnessFilteringView::Start()
{

}

double  VesselnessFilteringView::alphaFromSuppressPlatesPercentage(double suppressPlatesPercentage)
{
    return 3.0 * pow(suppressPlatesPercentage / 100.0, 2);
}

double  VesselnessFilteringView::betaFromSuppressBlobsPercentage(double suppressBlobsPercentage)
{
    return 0.001 + 1.0 * pow((100.0 - suppressBlobsPercentage) / 100.0, 2);
}

mitk::Point3D VesselnessFilteringView::ConvertFromWorldToIndex(mitk::Image* volume, const mitk::Point3D& worldPoint)
{
    mitk::Point3D index;
    volume->GetGeometry()->WorldToIndex(worldPoint, index);
    return index;
}

void VesselnessFilteringView::computeVesselnessVolume(mitk::Image* currentVolumeNode, mitk::Image*  currentOutputVolumeNode,
    mitk::Point3D previewRegionCenterRAS, int previewRegionSizeVoxel = -1, int minimumDiameterMm = 0, int maximumDiameterMm = 25,
    double alpha = 0.3, double beta = 0.3, int contrastMeasure = 150)
{
      if (!currentVolumeNode)
      {
          return;
      }
      auto inImage = vtkSmartPointer<vtkImageData>::New();

      if (previewRegionSizeVoxel>0)
      {
          auto imageclipper = vtkSmartPointer<vtkImageConstantPad>::New();
          imageclipper->SetInputData(currentVolumeNode->GetVtkImageData());

          mitk::Point3D previewRegionCenterIJK = ConvertFromWorldToIndex(currentVolumeNode, previewRegionCenterRAS);
          int previewRegionRadiusVoxel = int(round(previewRegionSizeVoxel / 2 + 0.5));
          imageclipper->SetOutputWholeExtent(
              previewRegionCenterIJK[0] - previewRegionRadiusVoxel, previewRegionCenterIJK[0] + previewRegionRadiusVoxel,
              previewRegionCenterIJK[1] - previewRegionRadiusVoxel, previewRegionCenterIJK[1] + previewRegionRadiusVoxel,
              previewRegionCenterIJK[2] - previewRegionRadiusVoxel, previewRegionCenterIJK[2] + previewRegionRadiusVoxel);
          imageclipper->Update();

          currentOutputVolumeNode->Initialize(imageclipper->GetOutput());
          currentOutputVolumeNode->GetVtkImageData()->ShallowCopy(imageclipper->GetOutput());
          inImage->DeepCopy(currentOutputVolumeNode->GetVtkImageData());
      }
      else
      {
          inImage->DeepCopy(currentVolumeNode->GetVtkImageData());
          currentOutputVolumeNode->Initialize(currentVolumeNode);
      }

      auto cast = vtkSmartPointer<vtkImageCast>::New();
      cast->SetInputData(inImage);
      cast->SetOutputScalarTypeToFloat();
      cast->Update();
      inImage->DeepCopy(cast->GetOutput());

      int discretizationSteps = 5;

      auto v = vtkSmartPointer<vtkvmtkVesselnessMeasureImageFilter>::New();
      v->SetInputData(inImage);
      v->SetSigmaMin(minimumDiameterMm);
      v->SetSigmaMax(maximumDiameterMm);
      v->SetNumberOfSigmaSteps(discretizationSteps);
      v->SetAlpha(alpha);
      v->SetBeta(beta);
      v->SetGamma(contrastMeasure);
      v->Update();

      auto outImage = vtkSmartPointer<vtkImageData>::New();
      outImage->DeepCopy(v->GetOutput());
      outImage->GetPointData()->GetScalars()->Modified();


      vtkImageData::
      outImage->SetSpacing(1, 1, 1);


}
