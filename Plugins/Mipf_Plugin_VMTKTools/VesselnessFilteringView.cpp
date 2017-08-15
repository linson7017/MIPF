#include "VesselnessFilteringView.h"


//vtk
#include <vtkImageConstantPad.h>
#include <vtkImageCast.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkImageLaplacian.h>
#include <vtkCamera.h>


#include  <vmtk/vtkvmtkVesselnessMeasureImageFilter.h>
#include <vmtk/vtkvmtkVesselnessMeasureImageFilter.h>

#include <VTK_Helpers.h>
#include <ITKImageTypeDef.h>
#include <ITKVTK_Helpers.h>

//mitk
#include "mitkImageCast.h"


VesselnessFilteringView::VesselnessFilteringView() :m_contrast(100)
{
}


VesselnessFilteringView::~VesselnessFilteringView()
{
}

void VesselnessFilteringView::CreateView()
{
    m_ui.setupUi(this);

    m_ui.ImageSelector->SetDataStorage(GetDataStorage());
    m_ui.ImageSelector->SetPredicate(CreatePredicate(Image));

    m_seeds = mitk::PointSet::New();

    mitk::DataNode::Pointer seedNode = mitk::DataNode::New();
    seedNode->SetData(m_seeds);
    seedNode->SetProperty("helper object", mitk::BoolProperty::New(true));
    GetDataStorage()->Add(seedNode);
    m_ui.SeedPointList->SetPointSetNode(seedNode);
    m_ui.SeedPointList->SetPointSet(m_seeds);
    m_ui.SeedPointList->SetMultiWidget(m_pMitkRenderWindow->GetMitkStdMultiWidget());

    connect(m_ui.ApplyBtn, SIGNAL(clicked()), this, SLOT(Start()));
}

void VesselnessFilteringView::Start()
{
    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());

    mitk::Image::Pointer resultImage = mitk::Image::New();

    double spacing[3];
    mitkImage->GetVtkImageData()->GetSpacing(spacing);
    double minSpacing = min(spacing[0], min(spacing[1], spacing[2]));

    double alpha = alphaFromSuppressPlatesPercentage(0.1);
    double beta = betaFromSuppressBlobsPercentage(0.1);

    m_contrast = m_ui.Contrast->text().toInt();
    if (m_seeds->GetSize()>0)
    {
        itk::Index<3> index;
        mitkImage->GetGeometry()->WorldToIndex(m_seeds->GetPoint(0), index);
        int extent[6];
        mitkImage->GetVtkImageData()->GetExtent(extent);
        int upDimeter = getDiameter(mitkImage->GetVtkImageData(), index);
        m_ui.MaxDiameter->setValue(upDimeter>0 ? upDimeter : m_ui.MaxDiameter->value());
        m_contrast = calculateContrastMeasure(mitkImage->GetVtkImageData(), index, m_ui.MaxDiameter->value());       
    }
    double maxDimeter = m_ui.MaxDiameter->value() * minSpacing;
    double minDimeter = m_ui.MinDiameter->value() * minSpacing;

    vtkSmartPointer<vtkImageData> resultVtkImage = vtkSmartPointer<vtkImageData>::New();

    MITK_INFO << "Parameters:";
    MITK_INFO << "Diameter:" << minDimeter << ", " << maxDimeter;
    MITK_INFO << "Alpha:" << alpha;
    MITK_INFO << "Beta:" << beta;
    MITK_INFO << "Contrast:" << m_contrast;
    computeVesselnessVolume(mitkImage->GetVtkImageData(), resultVtkImage.Get(), m_seeds->GetPoint(0), 100,
        minDimeter, maxDimeter, alpha, beta, m_contrast);

    Float3DImageType::Pointer itkImage = Float3DImageType::New();
    ITKVTKHelpers::ConvertVTKImageToITKImage(resultVtkImage.GetPointer(), itkImage.GetPointer());
    mitk::CastToMitkImage<Float3DImageType>(itkImage, resultImage);

    mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
    resultNode->SetData(resultImage);
    resultNode->SetName("Reslut");
    resultNode->SetColor(1.0, 0.0, 0.0);
    GetDataStorage()->Add(resultNode);
    //m_pMitkRenderWindow->Reinit(resultNode);

}

double  VesselnessFilteringView::alphaFromSuppressPlatesPercentage(double suppressPlatesPercentage)
{
    return 3.0 * pow(suppressPlatesPercentage / 100.0, 2);
}

double  VesselnessFilteringView::betaFromSuppressBlobsPercentage(double suppressBlobsPercentage)
{
    return 0.001 + 1.0 * pow((100.0 - suppressBlobsPercentage) / 100.0, 2);
}

itk::Index<3> VesselnessFilteringView::ConvertFromWorldToIndex(mitk::Image* volume, const mitk::Point3D& worldPoint)
{
    itk::Index<3> index;
    volume->GetGeometry()->WorldToIndex(worldPoint, index);
    return index;
}


void VesselnessFilteringView::computeVesselnessVolume(vtkImageData* currentVolumeNode, vtkImageData*  currentOutputVolumeNode,
    mitk::Point3D previewRegionCenterRAS, int previewRegionSizeVoxel, double minimumDiameterMm, double maximumDiameterMm,
    double alpha, double beta, double contrastMeasure)
{
      if (!currentVolumeNode)
      {
          return;
      }

      auto cast = vtkSmartPointer<vtkImageCast>::New();
      cast->SetInputData(currentVolumeNode);
      cast->SetOutputScalarTypeToFloat();
      cast->Update();

      int discretizationSteps = 5;

      auto v = vtkSmartPointer<vtkvmtkVesselnessMeasureImageFilter>::New();
      v->SetInputData(cast->GetOutput());
      v->SetSigmaMin(minimumDiameterMm);
      v->SetSigmaMax(maximumDiameterMm);
      v->SetNumberOfSigmaSteps(discretizationSteps);
      v->SetAlpha(alpha);
      v->SetBeta(beta);
      v->SetGamma(contrastMeasure);
      v->Update();


      VTKHelpers::SaveVtkImageData(v->GetOutput(), "D:/temp/vessel_enhance.mha");

      currentOutputVolumeNode->DeepCopy(v->GetOutput());
}

double VesselnessFilteringView::calculateContrastMeasure(vtkImageData* image, itk::Index<3> ijk, double diameter)
{
    float* pixel = static_cast<float*>(image->GetScalarPointer(ijk[0], ijk[1], ijk[2]));
    float* pixelRight = static_cast<float*>(image->GetScalarPointer(ijk[0] + (2 * diameter), ijk[1], ijk[2]));
    float* pixelLeft = static_cast<float*>(image->GetScalarPointer(ijk[0] - (2 * diameter), ijk[1], ijk[2]));
    float* pixelTop = static_cast<float*>(image->GetScalarPointer(ijk[0], ijk[1] + (2 * diameter), ijk[2]));
    float* pixelBottom = static_cast<float*>(image->GetScalarPointer(ijk[0], ijk[1] - (2 * diameter), ijk[2]));
    float* pixelFront = static_cast<float*>(image->GetScalarPointer(ijk[0], ijk[1], ijk[2] + (2 * diameter)));
    float* pixelBack = static_cast<float*>(image->GetScalarPointer(ijk[0], ijk[1], ijk[2] - (2 * diameter)));


    float outsideValues[6] = { pixel[0] - pixelRight[0], // right
        pixel[0] - pixelLeft[0],    // left
        pixel[0] - pixelTop[0],    // top
        pixel[0] - pixelBottom[0],    //bottom
        pixel[0] - pixelFront[0],    // front
        pixel[0] - pixelBack[0] };   // back

    double differenceValue = outsideValues[0];
    for (int i=0;i<7;i++)
    {
        if (differenceValue<outsideValues[i])
        {
            differenceValue = outsideValues[i];
        }
    }
    double contrastMeasure = differenceValue / 10;
    return 2 * contrastMeasure;
}

void  VesselnessFilteringView::performLaplaceOfGaussian(vtkImageData* image, vtkImageData* output)
{
    auto gaussian = vtkSmartPointer<vtkImageGaussianSmooth>::New();
    gaussian->SetInputData(image);
    gaussian->Update();

    auto laplacian = vtkSmartPointer<vtkImageLaplacian>::New();
    laplacian->SetInputData(gaussian->GetOutput());
    laplacian->Update();

    output->DeepCopy(laplacian->GetOutput());
}
    
template <class T>
int cmp(const T& a, const T& b)
{
      if (a==b)
      {
          return 0;
      }
      return a > b ? 1 : -1;
}
    
int VesselnessFilteringView::getDiameter(vtkImageData* image, itk::Index<3> ijk)
{
   // return 0;
    auto edgeImage = vtkSmartPointer<vtkImageData>::New();
    performLaplaceOfGaussian(image, edgeImage);

    bool foundDiameter = false;

    float edgeImageSeedValue = edgeImage->GetScalarComponentAsFloat(ijk[0], ijk[1], ijk[2], 0);
    int seedValueSign = cmp<float>(edgeImageSeedValue, 0);

    // [left, right, top, bottom, front, back]
    bool hits[] = { false, false, false, false, false, false };
    int distanceFromSeed = 1;
    while (!foundDiameter)
    {
           if (distanceFromSeed >= edgeImage->GetDimensions()[0]
               || distanceFromSeed >= edgeImage->GetDimensions()[1]
               || distanceFromSeed >= edgeImage->GetDimensions()[2])
           {
               break;
           }

           float edgeValues[] = { edgeImage->GetScalarComponentAsFloat(ijk[0] - distanceFromSeed, ijk[1], ijk[2], 0), // left
               edgeImage->GetScalarComponentAsFloat(ijk[0] + distanceFromSeed, ijk[1], ijk[2], 0), //# right
               edgeImage->GetScalarComponentAsFloat(ijk[0], ijk[1] + distanceFromSeed, ijk[2], 0), //# top
               edgeImage->GetScalarComponentAsFloat(ijk[0], ijk[1] - distanceFromSeed, ijk[2], 0), //# bottom
               edgeImage->GetScalarComponentAsFloat(ijk[0], ijk[1], ijk[2] + distanceFromSeed, 0), //# front
               edgeImage->GetScalarComponentAsFloat(ijk[0], ijk[1], ijk[2] - distanceFromSeed, 0) };// # back

           for (int i = 0; i < 6; i++)
           {
               if(!hits[i] && cmp<float>(edgeValues[i], 0) != seedValueSign)
               {
                   hits[i] = true;
               }
           }
           if (hits[0] && hits[1])
           {
               foundDiameter = true;
               break;
           }
           if (hits[2] && hits[3])
           {
               foundDiameter = true;
               break;
           }
           if (hits[4] && hits[5])
           {
               foundDiameter = true;
               break;
           }
           distanceFromSeed += 1;
               
    }
    return distanceFromSeed;
}

