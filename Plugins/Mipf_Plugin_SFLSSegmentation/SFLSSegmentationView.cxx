#include "SFLSSegmentationView.h"
#include "iqf_main.h"
#include "Res/R.h"
#include "ITKImageTypeDef.h"

// MITK
#include <mitkProperties.h>
#include <mitkITKImageImport.h>
#include <mitkImageAccessByItk.h>
#include <mitkPixelType.h>
#include <mitkProperties.h>
#include <mitkNodePredicateData.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkImageCast.h>

//ITK
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkCastImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>

#include "SFLSRobustStatSegmentor3DLabelMap_single.h"

template <typename TPixel>
UShort3DImagePointerType getOriginalMask(typename itk::Image<TPixel, 3>::Pointer img, TPixel thod)
{
    typedef UShort3DImageType MaskType;

    MaskType::SizeType size = img->GetLargestPossibleRegion().GetSize();

    long nx = size[0];
    long ny = size[1];
    long nz = size[2];

    MaskType::Pointer   mask = MaskType::New();
    MaskType::IndexType start = { { 0, 0, 0 } };

    MaskType::RegionType region;
    region.SetSize(size);
    region.SetIndex(start);

    mask->SetRegions(region);

    mask->SetSpacing(img->GetSpacing());
    mask->SetOrigin(img->GetOrigin());

    mask->Allocate();
    mask->FillBuffer(0);
    for (long ix = 0; ix < nx; ++ix)
    {
        for (long iy = 0; iy < ny; ++iy)
        {
            for (long iz = 0; iz < nz; ++iz)
            {
                MaskType::IndexType idx = { { ix, iy, iz } };
                TPixel              v = img->GetPixel(idx);
                if (v >= thod)
                {
                    mask->SetPixel(idx, 1);

                }
                else
                {
                    mask->SetPixel(idx, 0);
                }
            }
        }
    }

    return mask;
}

template <typename TPixel>
UShort3DImagePointerType getFinalMask(typename itk::Image<TPixel, 3>::Pointer img, unsigned char l, TPixel thod)
{
    typedef UShort3DImageType MaskType;

    MaskType::SizeType size = img->GetLargestPossibleRegion().GetSize();

    long nx = size[0];
    long ny = size[1];
    long nz = size[2];

    MaskType::Pointer   mask = MaskType::New();
    MaskType::IndexType start = { { 0, 0, 0 } };

    MaskType::RegionType region;
    region.SetSize(size);
    region.SetIndex(start);

    mask->SetRegions(region);

    mask->SetSpacing(img->GetSpacing());
    mask->SetOrigin(img->GetOrigin());

    mask->Allocate();
    mask->FillBuffer(0);
    for (long ix = 0; ix < nx; ++ix)
    {
        for (long iy = 0; iy < ny; ++iy)
        {
            for (long iz = 0; iz < nz; ++iz)
            {
                MaskType::IndexType idx = { { ix, iy, iz } };
                TPixel              v = img->GetPixel(idx);

                mask->SetPixel(idx, v <= thod ? l : 0);
            }
        }
    }

    return mask;
}


SFLSSegmentationView::SFLSSegmentationView(QF::IQF_Main* pMain) :MitkPluginView(pMain)
{
    m_pMain->Attach(this);
}

void SFLSSegmentationView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "") == 0)
    {
        //do what you want for the message
    }
}

void SFLSSegmentationView::CreateView()
{
    m_Controls = new Ui::WxAutoSegmentationViewControls;
    m_Controls->setupUi(this);
    if (m_Controls)
    {
        connect(m_Controls->ApplyPushButton, SIGNAL(clicked()), this, SLOT(ApplySegment()));
    }

    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
    mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    mitk::NodePredicateNot::Pointer isNotBinary = mitk::NodePredicateNot::New(isBinary);
    mitk::NodePredicateAnd::Pointer isImageNotBinary = mitk::NodePredicateAnd::New(isImage, isNotBinary);
    mitk::NodePredicateAnd::Pointer isImageAndBinary = mitk::NodePredicateAnd::New(isImage, isBinary);

    m_Controls->cmbbxOriginalImageSelector->SetDataStorage(m_pMitkDataManager->GetDataStorage());
    m_Controls->cmbbxOriginalImageSelector->SetPredicate(isImageNotBinary);
    m_Controls->cmbbxLabelImageSelector->SetDataStorage(m_pMitkDataManager->GetDataStorage());
    m_Controls->cmbbxLabelImageSelector->SetPredicate(isImageAndBinary);

}

void SFLSSegmentationView::SetFocus()
{

}

void SFLSSegmentationView::ApplySegment()
{

    mitk::DataNode::Pointer node = m_Controls->cmbbxOriginalImageSelector->GetSelectedNode();
    mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
    if (node.IsNull() || image->IsEmpty() || image->GetDimension() < 3)
    {
        return;
    }
    AccessFixedDimensionByItk(image, ItkImageRSSSegmentation, 3);

}

template < typename TPixel >
void SFLSSegmentationView::ItkImageRSSSegmentation(itk::Image< TPixel, 3 >* itkImage)
{
    //MITK image ----> ITK image
    mitk::Image* maskImage = dynamic_cast<mitk::Image*>
        (m_Controls->cmbbxLabelImageSelector->GetSelectedNode()->GetData());
    if (maskImage->IsEmpty()) return;

    /*itk::SmartPointer<mitk::ImageToItk<Int3DImagePointerType > maskimagetoitk =
        mitk::ImageToItk<Int3DImagePointerType >::New();
    maskimagetoitk->SetInput(maskImage);
    maskimagetoitk->Update();*/
    UShort3DImagePointerType maskItkImage = UShort3DImageType::New();
    mitk::CastToItkImage<UShort3DImageType>(maskImage, maskItkImage);

    UShort3DImagePointerType finalOrginalMask = getOriginalMask<UShortPixelType>(maskItkImage, 1);
    //finalOrginalMask->CopyInformation(maskimagetoitk->GetOutput());

    double expectedVolume = m_Controls->dspbExpectedVolume->value();
    double intensityHomogeneity = m_Controls->dspbIntensityHomogeneity->value();
    double curvatureWeight = m_Controls->dspbCurvatureWeight->value();
    int        OuputMaskValue = 1.0;
    double MaxRunningTime = m_Controls->maxRunningTime->value();
    double IteratorNum = m_Controls->maxIteratorNum->text().toDouble();

    typename CSFLSRobustStatSegmentor3DLabelMap<TPixel>::TLabelImage::Pointer InputMaskMap =
        preprocessLabelMap<typename CSFLSRobustStatSegmentor3DLabelMap<TPixel>::TLabelImage::PixelType>
        (finalOrginalMask, OuputMaskValue);

    // RSS segmentation
    CSFLSRobustStatSegmentor3DLabelMap<TPixel>  rss;
    rss.setImage(itkImage);
    rss.setNumIter(IteratorNum);
    rss.setMaxVolume(expectedVolume);
    rss.setInputLabelImage(InputMaskMap);
    rss.setMaxRunningTime(MaxRunningTime);
    rss.setIntensityHomogeneity(intensityHomogeneity);
    rss.setCurvatureWeight(curvatureWeight / 1.5);
    rss.doSegmenation();

   UShort3DImagePointerType finalMask = getFinalMask<float>(rss.mp_phi, OuputMaskValue, 2.0);
    //finalMask->CopyInformation(itkImage);

    //need convert imagedata type
    typedef itk::RescaleIntensityImageFilter<UShort3DImageType, itk::Image<unsigned char, 3> > RescaleShortToCharType;
    RescaleShortToCharType::Pointer RescaleShortToChar = RescaleShortToCharType::New();
    RescaleShortToChar->SetInput(finalMask);
    RescaleShortToChar->SetOutputMinimum(0);
    RescaleShortToChar->SetOutputMaximum(1);
    RescaleShortToChar->UpdateLargestPossibleRegion();

    //save data node
    mitk::Image::Pointer finalmaskImage = mitk::Image::New();
    finalmaskImage = mitk::ImportItkImage(RescaleShortToChar->GetOutput());
    finalmaskImage->IsInitialized();

    mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
    resultNode->SetProperty("name", mitk::StringProperty::New(m_Controls->cmbbxOriginalImageSelector->GetSelectedNode()->GetName() + "_Mask"));
    resultNode->SetProperty("color", mitk::ColorProperty::New(1.0, 0.0, 0.0));
    resultNode->SetProperty("layer", mitk::IntProperty::New(100));
    resultNode->SetProperty("opacity", mitk::FloatProperty::New(0.5));
    resultNode->SetProperty("visible", mitk::BoolProperty::New(true));
    if (resultNode)
    {
        resultNode->SetData(finalmaskImage->Clone());		//Used Clone
        resultNode->Modified();
    }
    this->GetDataStorage()->Add(resultNode, m_Controls->cmbbxOriginalImageSelector->GetSelectedNode());
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
