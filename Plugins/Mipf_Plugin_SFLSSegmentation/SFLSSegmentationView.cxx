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

//qmitk
#include <QmitkNewSegmentationDialog.h>

//ITK
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkCastImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>

#include "SFLSRobustStatSegmentor3DLabelMap_single.h"

#include <mitkToolManager.h>
#include <mitkToolManagerProvider.h>
#include <mitkDrawPaintbrushTool.h>
#include "MitkSegmentation/IQF_MitkSegmentationTool.h"

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


SFLSSegmentationView::SFLSSegmentationView() :MitkPluginView() , m_observeInterval(0) , m_bToolInited(false), m_pMitkSegTool(nullptr), m_tool(nullptr), m_seedImageNode(nullptr)
{
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
    m_pMain->Attach(this);
    m_Controls = new Ui::WxAutoSegmentationViewControls;
    m_Controls->setupUi(this);
    if (m_Controls)
    {
        connect(m_Controls->ApplyPushButton, SIGNAL(clicked()), this, SLOT(ApplySegment()));
        connect(m_Controls->StopBtn, SIGNAL(clicked()), this, SLOT(StopSegment()));

    }

    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
    mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    mitk::NodePredicateNot::Pointer isNotBinary = mitk::NodePredicateNot::New(isBinary);
    mitk::NodePredicateAnd::Pointer isImageNotBinary = mitk::NodePredicateAnd::New(isImage, isNotBinary);
    mitk::NodePredicateAnd::Pointer isImageAndBinary = mitk::NodePredicateAnd::New(isImage, isBinary);

    m_Controls->cmbbxOriginalImageSelector->SetDataStorage(GetDataStorage());
    m_Controls->cmbbxOriginalImageSelector->SetPredicate(isImageNotBinary);
    m_Controls->cmbbxLabelImageSelector->SetDataStorage(GetDataStorage());
    m_Controls->cmbbxLabelImageSelector->SetPredicate(isImageAndBinary);


    m_Controls->PenSizeSlider->setMinimum(1);
    m_Controls->PenSizeSlider->setMaximum(40);
    m_Controls->PenSizeSlider->setValue(5);
    connect(m_Controls->PaintBtn, SIGNAL(clicked()), this, SLOT(BeginPaint()));
    connect(m_Controls->EraseBtn, SIGNAL(clicked()), this, SLOT(BeginWipe()));
    connect(m_Controls->EndBtn, SIGNAL(clicked()), this, SLOT(EndTool()));
    connect(m_Controls->NewSeedImageBtn, SIGNAL(clicked()), this, SLOT(CreateNewSeedImage()));
    connect(m_Controls->PenSizeSlider, SIGNAL(valueChanged(double)), this, SLOT(PenSizeChanged(double)));
    connect(m_Controls->PresetComboBox, SIGNAL(currentTextChanged(const QString &)), this, SLOT(PresetChanged(const QString&)));
    connect(m_Controls->ObserveCB, SIGNAL(clicked(bool)), this, SLOT(ObserveChanged(bool)));


    m_pMitkSegTool = (IQF_MitkSegmentationTool*)m_pMain->GetInterfacePtr(QF_MitkSegmentation_Tool);

}

void SFLSSegmentationView::SetFocus()
{

}

void SFLSSegmentationView::PresetChanged(const QString& text)
{
     if (text.compare("liver",Qt::CaseInsensitive)==0)
     {
         m_Controls->dspbExpectedVolume->setValue(1500);
         m_Controls->dspbIntensityHomogeneity->setValue(0.9);
         m_Controls->dspbCurvatureWeight->setValue(0.7);
     }
     else if (text.compare("kidney", Qt::CaseInsensitive) == 0)
     {
         m_Controls->dspbExpectedVolume->setValue(200);
         m_Controls->dspbIntensityHomogeneity->setValue(0.1);
         m_Controls->dspbCurvatureWeight->setValue(0.5);
     }
     else if (text.compare("lung", Qt::CaseInsensitive) == 0)
     {
         m_Controls->dspbExpectedVolume->setValue(3000);
         m_Controls->dspbIntensityHomogeneity->setValue(0.7);
         m_Controls->dspbCurvatureWeight->setValue(0.4);
     }
     else if (text.compare("aorta", Qt::CaseInsensitive) == 0)
     {
         m_Controls->dspbExpectedVolume->setValue(60);
         m_Controls->dspbIntensityHomogeneity->setValue(1.0);
         m_Controls->dspbCurvatureWeight->setValue(0.0);
     }
}

void SFLSSegmentationView::ApplySegment()
{

    mitk::DataNode::Pointer node = m_Controls->cmbbxOriginalImageSelector->GetSelectedNode();
    mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
    if (node.IsNull() || image->IsEmpty() || image->GetDimension() < 3)
    {
        return;
    }
   // AccessFixedDimensionByItk(image, ItkImageRSSSegmentation, 3);
    Float3DImageType::Pointer itkImage;
    mitk::CastToItkImage(image, itkImage);
    ItkImageRSSSegmentation(itkImage);
}

void SFLSSegmentationView::ObserveChanged(bool checked)
{
      if (checked)
      {
          m_ObserveNode->SetVisibility(true);
      }
      else
      {
          m_ObserveNode->SetVisibility(false);
      }
}

void SFLSSegmentationView::SlotInteractionEnd(const itk::Image<float, 3>::Pointer& image, unsigned int currentInteraction)
{
    if (m_Controls->ObserveCB->isChecked())
    {
        if (m_observeInterval == m_Controls->ObserveIntervalSB->value()-1)
        {
            UShort3DImagePointerType finalMask = getFinalMask<float>(image, 1.0, 2.0);
            mitk::Image::Pointer mitkImage;
            mitk::CastToMitkImage(finalMask, mitkImage);
            m_ObserveNode->SetData(mitkImage);
            //Ë¢ÐÂºÄÊ±
            mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
        m_observeInterval++;
        if (m_observeInterval>m_Controls->ObserveIntervalSB->value())
        {
            m_observeInterval = 0;
        }  
    }  

    m_Controls->CurrentInteractionLabel->setText(QString("Current Interaction: %1").arg(currentInteraction, 6));
}

void SFLSSegmentationView::SlotSegmentationFinished()
{
    MITK_INFO << "Segmentation Finished";
     UShort3DImagePointerType finalMask = getFinalMask<float>(m_pSegmentation->mp_phi, 1.0, 2.0);
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
    /*if (m_ObserveNode.IsNotNull())
    {
        GetDataStorage()->Remove(m_ObserveNode);
        m_ObserveNode = nullptr;
    }  */
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    disconnect(this, &SFLSSegmentationView::SignalDoSegmentation, m_pSegmentation, &DSFSSegmentorNotifer::SlotDoSegmentation);
    disconnect(this, &SFLSSegmentationView::SignalStopSegmentation, m_pSegmentation, &DSFSSegmentorNotifer::SlotStopSegmentation);
    disconnect(m_pSegmentation, &DSFSSegmentorNotifer::SignalInteractionEnd, this, &SFLSSegmentationView::SlotInteractionEnd);
    disconnect(m_pSegmentation, &DSFSSegmentorNotifer::SignalSegmentationFinished, this, &SFLSSegmentationView::SlotSegmentationFinished);
}

void SFLSSegmentationView::StopSegment()
{
    emit SignalStopSegmentation();
}

void SFLSSegmentationView::ItkImageRSSSegmentation(Float3DImageType* itkImage)
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

    typename CSFLSRobustStatSegmentor3DLabelMap<float>::TLabelImage::Pointer InputMaskMap =
        preprocessLabelMap<typename CSFLSRobustStatSegmentor3DLabelMap<float>::TLabelImage::PixelType>
        (finalOrginalMask, OuputMaskValue);

    // RSS segmentation
    m_pSegmentation = new CSFLSRobustStatSegmentor3DLabelMap<float>();
    m_pSegmentation->setImage(itkImage);
    m_pSegmentation->setNumIter(IteratorNum);
    m_pSegmentation->setMaxVolume(expectedVolume);
    m_pSegmentation->setInputLabelImage(InputMaskMap);
    m_pSegmentation->setMaxRunningTime(MaxRunningTime);
    m_pSegmentation->setIntensityHomogeneity(intensityHomogeneity);
    m_pSegmentation->setCurvatureWeight(curvatureWeight / 1.5);


    m_segmentationThread = new QThread;
    m_pSegmentation->moveToThread(m_segmentationThread);
    disconnect(m_segmentationThread, &QThread::finished,
        m_segmentationThread, &QThread::deleteLater);
    disconnect(m_segmentationThread, &QThread::finished,
        m_pSegmentation, &QThread::deleteLater);

    connect(m_segmentationThread, &QThread::finished,
        m_segmentationThread, &QThread::deleteLater);
    connect(m_segmentationThread, &QThread::finished,
        m_pSegmentation, &QThread::deleteLater);

    if (!m_ObserveNode)
    {
        m_ObserveNode = mitk::DataNode::New();
        m_ObserveNode->SetName("Observer");
        //mitk::Surface::Pointer surface = mitk::Surface::New();
        // m_ObserveNode->SetData(surface);
        GetDataStorage()->Add(m_ObserveNode);
    }
    

    connect(this, &SFLSSegmentationView::SignalDoSegmentation, m_pSegmentation, &DSFSSegmentorNotifer::SlotDoSegmentation);
    connect(this, &SFLSSegmentationView::SignalStopSegmentation, m_pSegmentation, &DSFSSegmentorNotifer::SlotStopSegmentation, Qt::DirectConnection);
    connect(m_pSegmentation, &DSFSSegmentorNotifer::SignalInteractionEnd, this, &SFLSSegmentationView::SlotInteractionEnd, Qt::BlockingQueuedConnection);
    connect(m_pSegmentation, &DSFSSegmentorNotifer::SignalSegmentationFinished, this, &SFLSSegmentationView::SlotSegmentationFinished);

    // setEnabled(false);
    m_segmentationThread->start();
    emit SignalDoSegmentation();

    return;

   //UShort3DImagePointerType finalMask = getFinalMask<float>(m_pSegmentation->mp_phi, OuputMaskValue, 2.0);
   // //finalMask->CopyInformation(itkImage);

   // //need convert imagedata type
   // typedef itk::RescaleIntensityImageFilter<UShort3DImageType, itk::Image<unsigned char, 3> > RescaleShortToCharType;
   // RescaleShortToCharType::Pointer RescaleShortToChar = RescaleShortToCharType::New();
   // RescaleShortToChar->SetInput(finalMask);
   // RescaleShortToChar->SetOutputMinimum(0);
   // RescaleShortToChar->SetOutputMaximum(1);
   // RescaleShortToChar->UpdateLargestPossibleRegion();

   // //save data node
   // mitk::Image::Pointer finalmaskImage = mitk::Image::New();
   // finalmaskImage = mitk::ImportItkImage(RescaleShortToChar->GetOutput());
   // finalmaskImage->IsInitialized();

   // mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
   // resultNode->SetProperty("name", mitk::StringProperty::New(m_Controls->cmbbxOriginalImageSelector->GetSelectedNode()->GetName() + "_Mask"));
   // resultNode->SetProperty("color", mitk::ColorProperty::New(1.0, 0.0, 0.0));
   // resultNode->SetProperty("layer", mitk::IntProperty::New(100));
   // resultNode->SetProperty("opacity", mitk::FloatProperty::New(0.5));
   // resultNode->SetProperty("visible", mitk::BoolProperty::New(true));
   // if (resultNode)
   // {
   //     resultNode->SetData(finalmaskImage->Clone());		//Used Clone
   //     resultNode->Modified();
   // }
   // this->GetDataStorage()->Add(resultNode, m_Controls->cmbbxOriginalImageSelector->GetSelectedNode());
   // mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void SFLSSegmentationView::PenSizeChanged(double size)
{
    if (m_tool)
    {
        m_tool->SetSize(size);
    }
}

void SFLSSegmentationView::BeginPaint()
{
    ChangeTool("Paint");
}

void SFLSSegmentationView::BeginWipe()
{
    ChangeTool("Wipe");
}

void SFLSSegmentationView::EndTool()
{
    ChangeTool("");
}

void SFLSSegmentationView::ChangeTool(const char* toolName)
{
    if (!m_bToolInited) return;
    m_tool = dynamic_cast<mitk::PaintbrushTool*>(m_pMitkSegTool->ChangeTool(toolName));
    if (m_tool)
    {
        m_tool->SetSize(GetGuiProperty("GrowcutSegmentation.PenSize", "value").toInt());
    }
}

void SFLSSegmentationView::CreateNewSeedImage()
{
    QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog();
    int dialogReturnValue = dialog->exec();
    if (dialogReturnValue == QDialog::Rejected) return;

    mitk::Color color = dialog->GetColor();
    mitk::DataNode::Pointer newNode = mitk::DataNode::New();
    m_pMitkSegTool->CreateSegmentationNode(m_Controls->cmbbxOriginalImageSelector->GetSelectedNode(), newNode,
        dialog->GetSegmentationName().toStdString().c_str(), SegRGBColor(color.GetRed(), color.GetGreen(), color.GetBlue()));
    GetDataStorage()->Add(newNode, m_Controls->cmbbxOriginalImageSelector->GetSelectedNode());
    GetMitkRenderWindowInterface()->Reinit(newNode);
    m_Controls->cmbbxLabelImageSelector->SetSelectedNode(newNode);

    if (!m_bToolInited)
    {
        m_pMitkSegTool->Initialize();      
        m_bToolInited = true;
    }
    m_pMitkSegTool->SetReferenceData(m_Controls->cmbbxOriginalImageSelector->GetSelectedNode());
    m_pMitkSegTool->SetWorkingData(newNode);
}
