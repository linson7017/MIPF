#include "SkinExtractView.h"

#include "mitkImageCast.h"

#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryMorphologicalClosingImageFilter.h"
#include "itkBinaryMorphologicalOpeningImageFilter.h"
#include "itkConnectedThresholdImageFilter.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkVotingBinaryIterativeHoleFillingImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"

#include "vtkPolyData.h"

#include "ITKImageTypeDef.h"
#include "ITK_Helpers.h"

#include "MitkSegmentation/IQF_MitkSurfaceTool.h"

SkinExtractView::SkinExtractView()
{
}

SkinExtractView::~SkinExtractView()
{
}

void SkinExtractView::CreateView()
{
    m_ui.setupUi(this);

    m_ui.ImageSelector->SetDataStorage(GetDataStorage());
    m_ui.ImageSelector->SetPredicate(CreatePredicate(Image));


    m_ui.ThresholdSlider->setDecimals(1);
    m_ui.ThresholdSlider->setSpinBoxAlignment(Qt::AlignVCenter);
    m_ui.ThresholdSlider->setMaximum(1000);
    m_ui.ThresholdSlider->setMinimum(0);
    m_ui.ThresholdSlider->setMaximumValue(5000);
    m_ui.ThresholdSlider->setMinimumValue(-200);

    connect(m_ui.ExtractBtn, SIGNAL(clicked()), this, SLOT(Extract()));
    connect(m_ui.ImageSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(SelectionChanged(const mitk::DataNode *)));
}

void SkinExtractView::Extract()
{
    mitk::DataNode* node = m_ui.ImageSelector->GetSelectedNode();
    IQF_MitkSurfaceTool* pSurfaceTool = (IQF_MitkSurfaceTool*)m_pMain->GetInterfacePtr(QF_MitkSurface_Tool);
    if (node|| pSurfaceTool)
    {
        mitk::Image* imageData = dynamic_cast<mitk::Image*>(node->GetData());
        if (imageData)
        {
            Int3DImageType::Pointer itkImage;
            mitk::CastToItkImage(imageData, itkImage);
            typedef itk::BinaryThresholdImageFilter<Int3DImageType, UChar3DImageType> btImageFilterType;
            btImageFilterType::Pointer btFilter = btImageFilterType::New();
            btFilter->SetInput(itkImage);
            btFilter->SetLowerThreshold(m_ui.ThresholdSlider->minimumValue());
            btFilter->SetUpperThreshold(m_ui.ThresholdSlider->maximumValue());
            btFilter->SetInsideValue(255);
            btFilter->SetOutsideValue(0);
            btFilter->Update();
           // ImportITKImage(btFilter->GetOutput(), "bt1");

            //open
            typedef itk::BinaryBallStructuringElement<UChar3DImageType::PixelType, UChar3DImageType::ImageDimension>
                StructuringElementType;
            StructuringElementType structuringElementOpening;
            structuringElementOpening.SetRadius(2);
            structuringElementOpening.CreateStructuringElement();
            typedef itk::BinaryMorphologicalOpeningImageFilter <UChar3DImageType, UChar3DImageType, StructuringElementType>
                BinaryMorphologicalOpeningImageFilterType;
            BinaryMorphologicalOpeningImageFilterType::Pointer openingFilter
                = BinaryMorphologicalOpeningImageFilterType::New();
            openingFilter->SetInput(btFilter->GetOutput());
            openingFilter->SetKernel(structuringElementOpening);
            openingFilter->SetForegroundValue(255);
            try
            {
                openingFilter->Update();
               // ImportITKImage(openingFilter->GetOutput(), "opening");
            }
            catch (itk::ExceptionObject& ex)
            {
                std::cout << "Closing Failed! Exception code: " << std::endl;
                std::cout << ex.what() << std::endl;
                return;
            }
            UChar3DImageType* openedImage = openingFilter->GetOutput();
            //close roof and floor
            UChar3DImageType::SizeType inSize = openedImage->GetLargestPossibleRegion().GetSize();
            typedef itk::ExtractImageFilter< UChar3DImageType, UChar3DImageType > ExtractImageFilterType;

            UChar3DImageType::IndexType start;
            start[0] = 0;
            start[1] = 0;
            start[2] = 0;
            UChar3DImageType::SizeType size;
            size[0] = inSize[0];
            size[1] = inSize[1];
            size[2] = 1;
            //top
            UChar3DImageType::RegionType topRegion;
            topRegion.SetSize(size);
            topRegion.SetIndex(start);
            ExtractImageFilterType::Pointer topFilter = ExtractImageFilterType::New();
            topFilter->SetExtractionRegion(topRegion);
            topFilter->SetInput(openedImage);
            topFilter->Update();
            UChar3DImageType::Pointer topSlice = UChar3DImageType::New();
            ITKHelpers::BinaryFillLargeHolesByRegionGrowing<UChar3DImageType>(topFilter->GetOutput(), topSlice, start);
            itk::ImageRegionIterator<UChar3DImageType> topSliceIterator(topSlice, topRegion);
            while (!topSliceIterator.IsAtEnd())
            {
                openedImage->SetPixel(topSliceIterator.GetIndex(), topSliceIterator.Value());
                ++topSliceIterator;
            }
           //bottom
            start[2] = inSize[2]-1;
            UChar3DImageType::RegionType bottomRegion;
            bottomRegion.SetSize(size);
            bottomRegion.SetIndex(start);
            ExtractImageFilterType::Pointer bottomFilter = ExtractImageFilterType::New();
            bottomFilter->SetExtractionRegion(bottomRegion);
            bottomFilter->SetInput(openedImage);
            bottomFilter->Update();
            UChar3DImageType::Pointer bottomSlice = UChar3DImageType::New();
            ITKHelpers::BinaryFillLargeHolesByRegionGrowing<UChar3DImageType>(bottomFilter->GetOutput(), bottomSlice, start);
            itk::ImageRegionIterator<UChar3DImageType> bottomSliceIterator(bottomSlice, bottomRegion);
            while (!bottomSliceIterator.IsAtEnd())
            {
                openedImage->SetPixel(bottomSliceIterator.GetIndex(), bottomSliceIterator.Value());
                ++bottomSliceIterator;
            }
            //ImportITKImage(openedImage, "capping image");
            //return;


            //region growing
            UChar3DImageType::Pointer filledHolesImage = UChar3DImageType::New();
            UChar3DImageType::IndexType seed;
            seed.Fill(0);
            ITKHelpers::BinaryFillLargeHolesByRegionGrowing<UChar3DImageType>(openedImage, filledHolesImage, seed);
            //ImportITKImage(filledHolesImage.GetPointer(), "final image");
            //return;

            mitk::Image::Pointer tempImage;
            mitk::CastToMitkImage(filledHolesImage, tempImage);
            vtkSmartPointer<vtkPolyData> surfaceData = vtkSmartPointer<vtkPolyData>::New();
            pSurfaceTool->ExtractSurface(tempImage, surfaceData,15,1);


            ImportVtkPolyData(surfaceData.Get(), "skin", node);
        }
    }
}

void SkinExtractView::SelectionChanged(const mitk::DataNode* node)
{
    if (node)
    {
        mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
        if (image)
        {
            m_ui.ThresholdSlider->setMaximum(image->GetScalarValueMax());
            m_ui.ThresholdSlider->setMinimum(image->GetScalarValueMin());
            m_ui.ThresholdSlider->setMaximumValue(image->GetScalarValueMax());
            m_ui.ThresholdSlider->setMinimumValue(-500);
        }
    }
}
