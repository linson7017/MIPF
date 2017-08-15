#include "BoneExtract.h"

#include "ITKImageTypeDef.h"

//mitk
#include "mitkImageCast.h"

#include "itkOtsuMultipleThresholdsImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkConstantPadImageFilter.h"
#include "itkCropImageFilter.h"
#include "itkCurvatureFlowImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryMorphologicalClosingImageFilter.h"

#include "vtkPolyData.h"

#include "ITK_Helpers.h"

#include "MitkSegmentation/IQF_MitkSurfaceTool.h"


BoneExtract::BoneExtract()
{
}


BoneExtract::~BoneExtract()
{
}

void BoneExtract::CreateView()
{
    m_ui.setupUi(this);

    m_ui.ImageSelector->SetDataStorage(GetDataStorage());
    m_ui.ImageSelector->SetPredicate(CreatePredicate(Image));


    m_ui.ThresholdSlider->setDecimals(1);
    m_ui.ThresholdSlider->setSpinBoxAlignment(Qt::AlignVCenter);
    m_ui.ThresholdSlider->setMaximum(1000);
    m_ui.ThresholdSlider->setMinimum(0);
    m_ui.ThresholdSlider->setMaximumValue(1000);
    m_ui.ThresholdSlider->setMinimumValue(0);

    connect(m_ui.ExtractBtn, SIGNAL(clicked()), this, SLOT(Extract()));

    connect(m_ui.ImageSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(SelectionChanged(const mitk::DataNode *)));
}

void BoneExtract::SelectionChanged(const mitk::DataNode* node)
{
     if (node)
     {
         mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
         if (image)
         {
             m_ui.ThresholdSlider->setMaximum(image->GetScalarValueMax());
             m_ui.ThresholdSlider->setMinimum(image->GetScalarValueMin());
             m_ui.ThresholdSlider->setMaximumValue(image->GetScalarValueMax());
             m_ui.ThresholdSlider->setMinimumValue(image->GetScalarValueMin());
             m_ui.ThresholdSlider->setValues(250,2000);
         }
     }
}

void BoneExtract::AdaptiveThresholdExtract()
{
    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());
    Int3DImageType::Pointer itkImage = Int3DImageType::New();
    mitk::CastToItkImage<Int3DImageType>(mitkImage, itkImage);
}

void BoneExtract::Extract()
{
    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());
    Float3DImageType::Pointer itkImage = Float3DImageType::New();
    mitk::CastToItkImage<Float3DImageType>(mitkImage, itkImage);

    typedef itk::CurvatureFlowImageFilter<Float3DImageType, Float3DImageType> diffuseFilterType;
    diffuseFilterType::Pointer diffuseFilter = diffuseFilterType::New();
    diffuseFilter->SetInput(itkImage);
    diffuseFilter->SetTimeStep(0.0625);
    diffuseFilter->SetNumberOfIterations(5);
    diffuseFilter->Update();


    typedef itk::BinaryThresholdImageFilter<Float3DImageType, Int3DImageType> btFilterType;
    btFilterType::Pointer btFilter = btFilterType::New();
    btFilter->SetUpperThreshold(mitkImage->GetScalarValueMax());
    btFilter->SetLowerThreshold(-200);
    btFilter->SetInsideValue(1);
    btFilter->SetOutsideValue(0);
    btFilter->SetInput(diffuseFilter->GetOutput());
    btFilter->Update();

    UChar3DImageType::Pointer largestConnectedImage = UChar3DImageType::New();
    ITKHelpers::ExtractLargestConnected<Int3DImageType, UChar3DImageType>(btFilter->GetOutput(), largestConnectedImage.GetPointer());


    typedef itk::MaskImageFilter< Float3DImageType, UChar3DImageType,Int3DImageType > MaskFilterType;
    MaskFilterType::Pointer maskFilter = MaskFilterType::New();
    maskFilter->SetInput(itkImage);
    maskFilter->SetMaskImage(largestConnectedImage);
    maskFilter->Update();

    typedef itk::BinaryThresholdImageFilter<Int3DImageType, UShort3DImageType> ThresholdFilterType;
    ThresholdFilterType::Pointer thFilter = ThresholdFilterType::New();
    double valueRange[2];
    m_ui.ThresholdSlider->values(valueRange[0], valueRange[1]);
    thFilter->SetLowerThreshold(valueRange[0]);
    thFilter->SetUpperThreshold(valueRange[1]);
    thFilter->SetInsideValue(1);
    thFilter->SetOutsideValue(0);
    thFilter->SetInput(maskFilter->GetOutput());
    thFilter->UpdateLargestPossibleRegion();

    //ITKBasicAlgorithms::ExtractConnectedLargerThan<UShort3DImageType, UChar3DImageType>(thFilter->GetOutput(), largestConnectedImage.GetPointer(), 200);
    //mitk::Image::Pointer tempImage = mitk::Image::New();
    //mitk::CastToMitkImage(largestConnectedImage, tempImage);
    //mitk::DataNode::Pointer tempImageNode = mitk::DataNode::New();
    //tempImageNode->SetName("Temp");
    //tempImageNode->SetColor(0.7, 0.7, 0.0);
    //tempImageNode->SetData(tempImage);
    //GetDataStorage()->Add(tempImageNode);
    //return;


    //padding image
    typedef itk::ConstantPadImageFilter <UShort3DImageType, UShort3DImageType>
        ConstantPadImageFilterType;
    UShort3DImageType::SizeType lowerExtendRegion;
    lowerExtendRegion[0] = 0;
    lowerExtendRegion[1] = 0;
    lowerExtendRegion[2] = 1;

    UShort3DImageType::SizeType upperExtendRegion;
    upperExtendRegion[0] = 0;
    upperExtendRegion[1] = 0;
    upperExtendRegion[2] = 1;

    UShort3DImageType::PixelType constantPixel = 1;

    ConstantPadImageFilterType::Pointer padFilter
        = ConstantPadImageFilterType::New();
    padFilter->SetInput(thFilter->GetOutput());
    padFilter->SetPadLowerBound(lowerExtendRegion);
    padFilter->SetPadUpperBound(upperExtendRegion);
    padFilter->SetConstant(constantPixel);
    padFilter->Update();

    UShort3DImageType::Pointer coarseBone = UShort3DImageType::New();
    coarseBone->Graft(padFilter->GetOutput());

    ITKHelpers::ExtractLargestConnected<UShort3DImageType, UShort3DImageType>(coarseBone.GetPointer(), coarseBone.GetPointer());
    
    //crop the bottom and roof
    UShort3DImageType::SizeType imageSize = coarseBone->GetLargestPossibleRegion().GetSize();
    UShort3DImageType::SizeType cropSize;
    cropSize[0] = 0;
    cropSize[1] = 0;
    cropSize[2] = 1;
    typedef itk::CropImageFilter <UShort3DImageType, UShort3DImageType>
        CropImageFilterType;
    CropImageFilterType::Pointer cropFilter
        = CropImageFilterType::New();
    cropFilter->SetInput(coarseBone);
    cropFilter->SetBoundaryCropSize(cropSize);
    cropFilter->Update();
    coarseBone->Graft(cropFilter->GetOutput());

    if (m_ui.Radius->value()>0)
    {
        typedef itk::BinaryBallStructuringElement<UShort3DImageType::PixelType, UShort3DImageType::ImageDimension>
            StructuringElementType;
        StructuringElementType structuringElementClosing;
        structuringElementClosing.SetRadius(1.0);
        structuringElementClosing.CreateStructuringElement();
        typedef itk::BinaryMorphologicalClosingImageFilter <UShort3DImageType, UShort3DImageType, StructuringElementType>
            BinaryMorphologicalClosingImageFilterType;
        BinaryMorphologicalClosingImageFilterType::Pointer closingFilter
            = BinaryMorphologicalClosingImageFilterType::New();
        closingFilter->SetInput(cropFilter->GetOutput());
        closingFilter->SetKernel(structuringElementClosing);
        closingFilter->SetForegroundValue(m_ui.Radius->value());
        closingFilter->Update();
        coarseBone->Graft(closingFilter->GetOutput());
    }   

    ITKHelpers::ExtractConnectedLargerThan<UShort3DImageType, UShort3DImageType>(coarseBone.GetPointer(), coarseBone.GetPointer(), 1000);


    mitk::Image::Pointer resultImage = mitk::Image::New();
    mitk::CastToMitkImage(coarseBone, resultImage);

    IQF_MitkSurfaceTool* pSurfaceTool = (IQF_MitkSurfaceTool*)m_pMain->GetInterfacePtr(QF_MitkSurface_Tool);
    vtkSmartPointer<vtkPolyData> resultSurface = vtkSmartPointer<vtkPolyData>::New();
    pSurfaceTool->ExtractSurface(resultImage, resultSurface, 5,m_ui.LargestConnectedCB->isChecked());

    mitk::Surface::Pointer mitkResultSurface = mitk::Surface::New();
    mitkResultSurface->SetVtkPolyData(resultSurface);
    mitk::DataNode::Pointer resultImageNode = mitk::DataNode::New();
    resultImageNode->SetName("Result");
    resultImageNode->SetColor(0.7, 0.7, 0.7);
    resultImageNode->SetData(mitkResultSurface);
    GetDataStorage()->Add(resultImageNode);
}
