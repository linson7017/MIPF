#include "LungSegmentationView.h" 
#include "iqf_main.h"  
  
#include "ITKImageTypeDef.h"
#include "ITK_Helpers.h"

//itk
#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryMorphologicalClosingImageFilter.h"
#include "itkBinaryMorphologicalOpeningImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkLabelShapeKeepNObjectsImageFilter.h"


#include "Segmentation/IQF_RSSSegmentation.h"
#include "Segmentation/IQF_SegmentationMethodFactory.h"

//mitk
#include "mitkImageCast.h"

LungSegmentationView::LungSegmentationView() :MitkPluginView() 
{
}
 
LungSegmentationView::~LungSegmentationView() 
{
}
 
void LungSegmentationView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(CreateImagePredicate());

    connect(m_ui.ApplyBtn, &QPushButton::clicked, this, &LungSegmentationView::Apply);
} 
 
WndHandle LungSegmentationView::GetPluginHandle() 
{
    return this; 
}

void LungSegmentationView::Apply()
{

   /* if (m_ui.DataSelector->GetSelectedNode().IsNull());
     {
        MITK_INFO << "Null node!";
        return;
     }*/
     mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());
     if (!mitkImage)
     {
         MITK_INFO << "Null Image!";
         return;
     }

     MITK_INFO << "Lung Segmention begin!";
     Float3DImageType::Pointer image;
     mitk::CastToItkImage(mitkImage, image);

     typedef itk::BinaryThresholdImageFilter<Float3DImageType, UShort3DImageType> btImageFilterType;
     btImageFilterType::Pointer btFilter = btImageFilterType::New();
     btFilter->SetInput(image);
     btFilter->SetLowerThreshold(-500);
     btFilter->SetUpperThreshold(mitkImage->GetScalarValueMax());
     btFilter->SetInsideValue(1);
     btFilter->SetOutsideValue(0);
     btFilter->Update();


     //open
     typedef itk::BinaryBallStructuringElement<UShort3DImageType::PixelType, UShort3DImageType::ImageDimension>
         StructuringElementType;
     StructuringElementType structuringElementOpening;
     structuringElementOpening.SetRadius(2);
     structuringElementOpening.CreateStructuringElement();
     typedef itk::BinaryMorphologicalOpeningImageFilter <UShort3DImageType, UShort3DImageType, StructuringElementType>
         BinaryMorphologicalOpeningImageFilterType;
     BinaryMorphologicalOpeningImageFilterType::Pointer openingFilter
         = BinaryMorphologicalOpeningImageFilterType::New();
     openingFilter->SetInput(btFilter->GetOutput());
     openingFilter->SetKernel(structuringElementOpening);
     openingFilter->SetForegroundValue(1);
     openingFilter->SetBackgroundValue(0);
     try
     {
         openingFilter->Update();
     }
     catch (itk::ExceptionObject& ex)
     {
         std::cout << "Closing Failed! Exception code: " << std::endl;
         std::cout << ex.what() << std::endl;
         return;
     }
     UShort3DImageType* openedImage = openingFilter->GetOutput();
     //close roof and floor
     UShort3DImageType::SizeType inSize = openedImage->GetLargestPossibleRegion().GetSize();
     typedef itk::ExtractImageFilter< UShort3DImageType, UShort3DImageType > ExtractImageFilterType;

     UShort3DImageType::IndexType start;
     start[0] = 0;
     start[1] = 0;
     start[2] = 0;
     UShort3DImageType::SizeType size;
     size[0] = inSize[0];
     size[1] = inSize[1];
     size[2] = 1;
     //top
     UShort3DImageType::RegionType topRegion;
     topRegion.SetSize(size);
     topRegion.SetIndex(start);
     ExtractImageFilterType::Pointer topFilter = ExtractImageFilterType::New();
     topFilter->SetExtractionRegion(topRegion);
     topFilter->SetInput(openedImage);
     topFilter->Update();
     UShort3DImageType::Pointer topSlice = UShort3DImageType::New();
     ITKHelpers::BinaryFillLargeHolesByRegionGrowing<UShort3DImageType>(topFilter->GetOutput(), topSlice, start);
     itk::ImageRegionIterator<UShort3DImageType> topSliceIterator(topSlice, topRegion);
     while (!topSliceIterator.IsAtEnd())
     {
         openedImage->SetPixel(topSliceIterator.GetIndex(), topSliceIterator.Value());
         ++topSliceIterator;
     }
     //bottom
     start[2] = inSize[2] - 1;
     UShort3DImageType::RegionType bottomRegion;
     bottomRegion.SetSize(size);
     bottomRegion.SetIndex(start);
     ExtractImageFilterType::Pointer bottomFilter = ExtractImageFilterType::New();
     bottomFilter->SetExtractionRegion(bottomRegion);
     bottomFilter->SetInput(openedImage);
     bottomFilter->Update();
     UShort3DImageType::Pointer bottomSlice = UShort3DImageType::New();
     ITKHelpers::BinaryFillLargeHolesByRegionGrowing<UShort3DImageType>(bottomFilter->GetOutput(), bottomSlice, start);
     itk::ImageRegionIterator<UShort3DImageType> bottomSliceIterator(bottomSlice, bottomRegion);
     while (!bottomSliceIterator.IsAtEnd())
     {
         openedImage->SetPixel(bottomSliceIterator.GetIndex(), bottomSliceIterator.Value());
         ++bottomSliceIterator;
     }
     //region growing
     UShort3DImageType::Pointer filledHolesImage = UShort3DImageType::New();
     UShort3DImageType::IndexType seed;
     seed.Fill(0);
     ITKHelpers::BinaryFillLargeHolesByRegionGrowing<UShort3DImageType>(openedImage, filledHolesImage, seed);

     ITKHelpers::ExtractLargestConnected<UShort3DImageType, UShort3DImageType>(filledHolesImage, filledHolesImage);
     typedef itk::MaskImageFilter< Float3DImageType, UShort3DImageType, Float3DImageType > MaskFilterType;
     MaskFilterType::Pointer maskFilter = MaskFilterType::New();
     maskFilter->SetInput(image);
     maskFilter->SetMaskImage(filledHolesImage);
     maskFilter->Update();
     ImportITKImage(maskFilter->GetOutput(), "mask");


     btImageFilterType::Pointer btFilter2 = btImageFilterType::New();
     btFilter2->SetInput(maskFilter->GetOutput());
     btFilter2->SetLowerThreshold(-1000);
     btFilter2->SetUpperThreshold(-500);
     btFilter2->SetInsideValue(1);
     btFilter2->SetOutsideValue(0);
     btFilter2->Update();
     ImportITKImage(btFilter2->GetOutput(), "bt2");

     StructuringElementType structuringElement;
     structuringElement.SetRadius(5);
     structuringElement.CreateStructuringElement();
     typedef itk::BinaryErodeImageFilter <UShort3DImageType, UShort3DImageType, StructuringElementType>
         BinaryErodeImageFilterType;
     BinaryErodeImageFilterType::Pointer erodeFilter
         = BinaryErodeImageFilterType::New();
     erodeFilter->SetInput(btFilter2->GetOutput());
     erodeFilter->SetKernel(structuringElement);
     erodeFilter->SetBackgroundValue(0);
     erodeFilter->SetForegroundValue(1);
     erodeFilter->Update();
     ImportITKImage(erodeFilter->GetOutput(), "erode");

     typedef itk::ConnectedComponentImageFilter <UShort3DImageType, UInt3DImageType >
         ConnectedComponentImageFilterType;
     ConnectedComponentImageFilterType::Pointer connected = ConnectedComponentImageFilterType::New();
     connected->SetInput(erodeFilter->GetOutput());
     connected->Update();

     typedef itk::LabelShapeKeepNObjectsImageFilter< UInt3DImageType > LabelShapeKeepNObjectsImageFilterType;
     LabelShapeKeepNObjectsImageFilterType::Pointer labelShapeKeepNObjectsImageFilter = LabelShapeKeepNObjectsImageFilterType::New();
     labelShapeKeepNObjectsImageFilter->SetInput(connected->GetOutput());
     labelShapeKeepNObjectsImageFilter->SetBackgroundValue(0);
     labelShapeKeepNObjectsImageFilter->SetNumberOfObjects(2);
     labelShapeKeepNObjectsImageFilter->SetAttribute(LabelShapeKeepNObjectsImageFilterType::LabelObjectType::NUMBER_OF_PIXELS);
     labelShapeKeepNObjectsImageFilter->Update();
     
     itk::ImageRegionIterator<UInt3DImageType> iter(labelShapeKeepNObjectsImageFilter->GetOutput(), labelShapeKeepNObjectsImageFilter->GetOutput()->GetLargestPossibleRegion());
     iter.GoToBegin();
     std::set<int> values;
     while (!iter.IsAtEnd())
     {
         if (iter.Value()>0)
         {
             values.insert(iter.Value());
         }
         ++iter;
     }
     ImportITKImage(labelShapeKeepNObjectsImageFilter->GetOutput(), "labeled");

     typedef itk::BinaryThresholdImageFilter<UInt3DImageType, UShort3DImageType> uiBtImageFilterType;
     int i = 0;
     IQF_SegmentationMethodFactory* pFactory = (IQF_SegmentationMethodFactory*)GetInterfacePtr(QF_Segmentation_Factory);
    
     for (std::set<int>::iterator it = values.begin();it!=values.end();it++,i++)
     {
         MITK_INFO << "Active contour segment lung " << i;
         uiBtImageFilterType::Pointer bt1 = uiBtImageFilterType::New();
         bt1->SetInput(labelShapeKeepNObjectsImageFilter->GetOutput());
         bt1->SetUpperThreshold(*it);
         bt1->SetLowerThreshold(*it);
         bt1->SetInsideValue(1);
         bt1->SetOutsideValue(0);
         bt1->Update();

         StructuringElementType se;
         se.SetRadius(5);
         se.CreateStructuringElement();
         BinaryErodeImageFilterType::Pointer filter = BinaryErodeImageFilterType::New();
         filter->SetInput(bt1->GetOutput());
         filter->SetKernel(se);
         filter->SetBackgroundValue(0);
         filter->SetForegroundValue(1);
         filter->Update();

         QString seedName = QString("seed%1").arg(i);
         ImportITKImage(filter->GetOutput(), seedName.toLocal8Bit().constData());
         //continue;;
         IQF_RSSSegmentation* pSegmentor = pFactory->CreateRSSSegmentationMethod();
         pSegmentor->SetImage(image);
         pSegmentor->SetInputLabeledImage(filter->GetOutput());
         pSegmentor->SetIntensityHomogeneity(0.7);
         pSegmentor->SetCurvatureWeight(0.4);
         pSegmentor->SetMaxRunningTime(5);
         pSegmentor->SetNumIter(1000);
         pSegmentor->SetMaxVolume(4000);
         pSegmentor->PerformSegmentation();
         UShort3DImageType::Pointer result = UShort3DImageType::New();
         pSegmentor->GetFinalMask(pSegmentor->GetResult(), result.GetPointer(), 1.0, 2.0);

         QString name = QString("lung%1").arg(i);
         ImportITKImage(result.GetPointer(), name.toLocal8Bit().constData());
         pSegmentor->Release();
     }
     

}