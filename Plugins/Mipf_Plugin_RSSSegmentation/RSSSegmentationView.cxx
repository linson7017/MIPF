#include "RSSSegmentationView.h" 
#include "iqf_main.h"  

#include "Segmentation/IQF_SegmentationMethodFactory.h"
#include "Segmentation/IQF_RSSSegmentation.h"

#include "mitkImageCast.h"

#include "itkBinaryThresholdImageFilter.h"

template <typename TPixel>
UShort3DImageType::Pointer getFinalMask(typename itk::Image<TPixel, 3>::Pointer img, unsigned char l, TPixel thod)
{
    typedef itk::BinaryThresholdImageFilter< itk::Image<TPixel, 3>, UShort3DImageType> FilterType;
    FilterType::Pointer filter = FilterType::New();

    filter->SetInput(img);
    filter->SetUpperThreshold(thod);
    filter->SetInsideValue(l);
    filter->SetOutsideValue(0);
    filter->Update();

    return filter->GetOutput();


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
  
RSSSegmentationView::RSSSegmentationView() :MitkPluginView() 
{
}
 
RSSSegmentationView::~RSSSegmentationView() 
{
}
 
void RSSSegmentationView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(CreateImagePredicate());

    m_ui.LabelSelector->SetDataStorage(GetDataStorage());
    m_ui.LabelSelector->SetPredicate(CreateImagePredicate());

    connect(m_ui.ApplyBtn, &QPushButton::clicked, this, &RSSSegmentationView::Apply);


    IQF_SegmentationMethodFactory* pFactory = (IQF_SegmentationMethodFactory*)GetInterfacePtr(QF_Segmentation_Factory);
    m_pSegmentor = pFactory->CreateRSSSegmentationMethod();
} 
 
WndHandle RSSSegmentationView::GetPluginHandle() 
{
    return this; 
}

void RSSSegmentationView::Apply()
{
    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());
    mitk::Image* mitkLabel = dynamic_cast<mitk::Image*>(m_ui.LabelSelector->GetSelectedNode()->GetData());

    Float3DImageType::Pointer image;
    UShort3DImageType::Pointer lable;
    mitk::CastToItkImage(mitkImage, image);
    mitk::CastToItkImage(mitkLabel, lable);

    m_pSegmentor->SetImage(image);
    m_pSegmentor->SetInputLabeledImage(lable);
    m_pSegmentor->SetNumIter(500);
    m_pSegmentor->SetMaxVolume(1400);

    m_pSegmentor->PerformSegmentation();

    UShort3DImageType::Pointer result = getFinalMask<float>(m_pSegmentor->GetResult(), 1.0, 2.0);
    ImportITKImage(result.GetPointer(), "result");
}