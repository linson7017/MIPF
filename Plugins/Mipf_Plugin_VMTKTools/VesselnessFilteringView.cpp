#include "VesselnessFilteringView.h"


//vtk
#include <vtkImageConstantPad.h>
#include <vtkImageCast.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>


#include  <vmtk/vtkvmtkVesselnessMeasureImageFilter.h>
#include <vmtk/vtkvmtkVesselnessMeasureImageFilter.h>

#include <VTK_Helpers.h>


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
    m_ui.ImageSelector->SetPredicate(CreatePredicate(1));

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

    double maxDimeter = 7 * minSpacing;
    double minDimeter = 1*minSpacing;

    double alpha = alphaFromSuppressPlatesPercentage(0.1);
    double beta = betaFromSuppressBlobsPercentage(0.1);

    if (m_seeds->GetSize()>0)
    {
        mitk::Point3D index;
        mitkImage->GetGeometry()->WorldToIndex(m_seeds->GetPoint(0), index);
        double contrastMeasure = calculateContrastMeasure(mitkImage->GetVtkImageData(), index,getDiameter(mitkImage->GetVtkImageData(), index));
    }
    

    computeVesselnessVolume(mitkImage, resultImage, m_seeds->GetPoint(0), 100,
        minDimeter, maxDimeter, alpha, beta, 100);


    mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
    resultNode->SetData(resultImage);
    resultNode->SetName("Reslut");
    resultNode->SetColor(1.0, 0.0, 0.0);
    GetDataStorage()->Add(resultNode);
    RequestRenderWindowUpdate();

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
    mitk::Point3D previewRegionCenterRAS, int previewRegionSizeVoxel, double minimumDiameterMm, double maximumDiameterMm,
    double alpha, double beta, double contrastMeasure)
{
      if (!currentVolumeNode)
      {
          return;
      }

      auto cast = vtkSmartPointer<vtkImageCast>::New();
      cast->SetInputData(currentVolumeNode->GetVtkImageData());
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

      currentOutputVolumeNode->Initialize(v->GetOutput());
      currentOutputVolumeNode->GetGeometry()->InitializeGeometry(currentVolumeNode->GetGeometry());

      std::string componentType = currentOutputVolumeNode->GetPixelType().GetComponentTypeAsString();


      std::string scalarType = v->GetOutput()->GetScalarTypeAsString();
      int componentNum = v->GetOutput()->GetNumberOfScalarComponents();

      VTKHelpers::SaveVtkImageData(v->GetOutput(), "D:/temp/vessel_enhance.mha");

    //  auto outImage = vtkSmartPointer<vtkImageData>::New();
      currentOutputVolumeNode->GetVtkImageData()->DeepCopy(v->GetOutput());
      currentOutputVolumeNode->Modified();
    //  outImage->GetPointData()->GetScalars()->Modified();


}

double calculateContrastMeasure(vtkImageData* image, mitk::Point3D ijk, double diameter)
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
    
double  getDiameter(vtkImageData* image, mitk::Point3D ijk)
{
    return 0;
//    edgeImage = self.performLaplaceOfGaussian(image)
//
//        foundDiameter = False
//
//        edgeImageSeedValue = edgeImage.GetScalarComponentAsFloat(ijk[0], ijk[1], ijk[2], 0)
//        seedValueSign = cmp(edgeImageSeedValue, 0)  # returns 1 if > 0 or -1 if < 0
//
//        # the list of hits
//# [left, right, top, bottom, front, back]
//        hits = [False, False, False, False, False, False]
//
//        distanceFromSeed = 1
//        while not foundDiameter:
//
//    if (distanceFromSeed >= edgeImage.GetDimensions()[0]
//        or distanceFromSeed >= edgeImage.GetDimensions()[1]
//        or distanceFromSeed >= edgeImage.GetDimensions()[2]) :
//        # we are out of bounds
//        break
//
//        # get the values for the lookahead directions in the edgeImage
//        edgeValues = [edgeImage.GetScalarComponentAsFloat(ijk[0] - distanceFromSeed, ijk[1], ijk[2], 0), # left
//        edgeImage.GetScalarComponentAsFloat(ijk[0] + distanceFromSeed, ijk[1], ijk[2], 0), # right
//        edgeImage.GetScalarComponentAsFloat(ijk[0], ijk[1] + distanceFromSeed, ijk[2], 0), # top
//        edgeImage.GetScalarComponentAsFloat(ijk[0], ijk[1] - distanceFromSeed, ijk[2], 0), # bottom
//        edgeImage.GetScalarComponentAsFloat(ijk[0], ijk[1], ijk[2] + distanceFromSeed, 0), # front
//        edgeImage.GetScalarComponentAsFloat(ijk[0], ijk[1], ijk[2] - distanceFromSeed, 0)]  # back
//
//        # first loop, check if we have hits
//        for v in range(len(edgeValues)) :
//
//            if not hits[v] and cmp(edgeValues[v], 0) != seedValueSign :
//                # hit
//                hits[v] = True
//
//                # now check if we have two hits in opposite directions
//                if hits[0] and hits[1]:
//    # we have the diameter!
//        foundDiameter = True
//        break
//
//        if hits[2] and hits[3]:
//    foundDiameter = True
//        break
//
//        if hits[4] and hits[5] :
//            foundDiameter = True
//            break
//
//            # increase distance from seed for next iteration
//            distanceFromSeed += 1
//
//            # we now just return the distanceFromSeed
//# if the diameter was not detected properly, this can equal one of the image dimensions
//            return distanceFromSeed
}

