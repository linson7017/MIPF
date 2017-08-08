#include "SurfaceExtractView.h"
#include "iqf_main.h"
#include "Res/R.h"
#include "ITKImageTypeDef.h"
//Qt
#include <QInputDialog>
#include <QCheckBox>

//qmitk
#include "QmitkDataStorageComboBox.h"

//mitk
#include "mitkImageCast.h"
#include "mitkLabelSetImage.h"
#include <mitkShowSegmentationAsSmoothedSurface.h>
#include <mitkShowSegmentationAsSurface.h>
#include <mitkStatusBar.h>
#include "mitkImageToItk.h"
#include <itkAddImageFilter.h>
#include <itkBinaryMedianImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkConstantPadImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkMultiplyImageFilter.h>
#include <itkRegionOfInterestImageFilter.h>
#include <itkIntelligentBinaryClosingFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include "itkConnectedComponentImageFilter.h"
#include "itkLabelShapeKeepNObjectsImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include <mitkGeometry3D.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageToSurfaceFilter.h>
#include <mitkProgressBar.h>
#include <mitkStatusBar.h>
#include <mitkUIDGenerator.h>
#include <mitkVtkRepresentationProperty.h>
#include <mitkManualSegmentationToSurfaceFilter.h>
#include <vtkCleanPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkQuadricDecimation.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkPolyDataConnectivityFilter.h>

#include "MitkSegmentation/IQF_MitkSurfaceTool.h"

using namespace mitk;


SurfaceExtractView::SurfaceExtractView(QF::IQF_Main* pMain) : MitkPluginView(pMain)
{
    m_pMain->Attach(this);

    connect(&m_Watcher, SIGNAL(finished()), this, SLOT(ShowResult()));
}


SurfaceExtractView::~SurfaceExtractView()
{
}

void SurfaceExtractView::Constructed(R* pR)
{
    m_pImageSelector = (QmitkDataStorageComboBox*)pR->getObjectFromGlobalMap("SurfaceExtract.ImageSelector");

    if (m_pImageSelector)
    {
        m_pImageSelector->SetPredicate(CreatePredicate(1));
        m_pImageSelector->SetDataStorage(m_pMitkDataManager->GetDataStorage());
    }

    m_pSurfaceSelector = (QmitkDataStorageComboBox*)pR->getObjectFromGlobalMap("SurfaceExtract.SurfaceSelector");

    if (m_pSurfaceSelector)
    {
        auto surfaceType = mitk::TNodePredicateDataType<mitk::Surface>::New();
        m_pSurfaceSelector->SetPredicate(surfaceType);
        m_pSurfaceSelector->SetDataStorage(m_pMitkDataManager->GetDataStorage());
    }
}

void SurfaceExtractView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "SurfaceExtract.Extract") == 0)
    {
        m_SurfaceName = QInputDialog::getText(NULL, "Input Result Name", "Image Name:");
        if (m_SurfaceName.isEmpty())
        {
            return;
        }
        int smooth = 0;
        bool largestConnect = false;
        smooth = GetGuiProperty("SurfaceExtract.Smooth", "text").toString().toInt();
        largestConnect = GetGuiProperty("SurfaceExtract.LargestConnect", "checked").toBool();

        mitk::Image* image = dynamic_cast<mitk::Image*>(m_pImageSelector->GetSelectedNode()->GetData());

        if (m_Watcher.isRunning())
            m_Watcher.waitForFinished();
        m_Future = QtConcurrent::run(this, &SurfaceExtractView::ExtractSurface, image, smooth, largestConnect);
        m_Watcher.setFuture(m_Future);

        //ShowResult();
        return;
        
    }
    else if (strcmp(szMessage, "SurfaceExtract.Smooth") == 0)
    {
        
        IQF_MitkSurfaceTool* pSurfaceTool = (IQF_MitkSurfaceTool*)m_pMain->GetInterfacePtr(QF_MitkSurface_Tool);
        if (pSurfaceTool)
        {
            int smootTimes = GetGuiProperty("SurfaceExtract.SmoothTimes", "text").toString().toInt();
            mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_pSurfaceSelector->GetSelectedNode()->GetData());
            vtkSmartPointer<vtkPolyData> smoothSurface = vtkSmartPointer<vtkPolyData>::New();
            pSurfaceTool->SmoothTubeSurface(surface->GetVtkPolyData(), smoothSurface, smootTimes);

            mitk::Surface::Pointer mitkSurface = mitk::Surface::New();
            mitkSurface->SetVtkPolyData(smoothSurface);
            
            QString resultName = "Surface_Smoothed_";
            resultName.append(GetGuiProperty("SurfaceExtract.SmoothTimes", "text").toString());
            mitk::DataNode::Pointer result = mitk::DataNode::New();
            result->SetData(mitkSurface);
            result->SetName(resultName.toStdString());
            result->SetColor(1.0,1.0,0.0);
            GetDataStorage()->Add(result, m_pSurfaceSelector->GetSelectedNode());

        }

    }
}

void SurfaceExtractView::ExtractSmoothedSurface(mitk::Image* image)
{

    m_smoothing = 1.0;
    bool smoothingHint = true;
    if (smoothingHint)
    {
        m_smoothing = 0.0;
        Vector3D spacing = image->GetGeometry()->GetSpacing();

        for (Vector3D::Iterator iter = spacing.Begin(); iter != spacing.End(); ++iter)
            m_smoothing = max(m_smoothing, (float)*iter);
    }



    m_decimation = 0.5;

    m_closing = 0.0;

    m_timeNr = 0;

    if (image->GetDimension() == 4)
        MITK_INFO << "CREATING SMOOTHED POLYGON MODEL (t = " << m_timeNr << ')';
    else
        MITK_INFO << "CREATING SMOOTHED POLYGON MODEL";

    MITK_INFO << "  Smoothing  = " << m_smoothing;
    MITK_INFO << "  Decimation = " << m_decimation;
    MITK_INFO << "  Closing    = " << m_closing;

    mitk::BaseGeometry::Pointer geometry = dynamic_cast<mitk::BaseGeometry *>(image->GetGeometry()->Clone().GetPointer());

    // Make ITK image out of MITK image

    if (image->GetDimension() == 4)
    {
        mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
        imageTimeSelector->SetInput(image);
        imageTimeSelector->SetTimeNr(m_timeNr);
        imageTimeSelector->UpdateLargestPossibleRegion();
        image = imageTimeSelector->GetOutput(0);
    }

    ImageToItk<UChar3DImageType>::Pointer imageToItkFilter = ImageToItk<UChar3DImageType>::New();

    try
    {
        imageToItkFilter->SetInput(image);
    }
    catch (const itk::ExceptionObject &e)
    {
        // Most probably the input image type is wrong. Binary images are expected to be
        // >unsigned< char images.
        MITK_ERROR << e.GetDescription() << endl;
        return;
    }

    imageToItkFilter->Update();

    typedef itk::RescaleIntensityImageFilter<UChar3DImageType, UChar3DImageType> RescaleIntensityImageFilterType;
    RescaleIntensityImageFilterType::Pointer rescaleImageFileter = RescaleIntensityImageFilterType::New();
    rescaleImageFileter->SetInput(imageToItkFilter->GetOutput());
    rescaleImageFileter->SetOutputMinimum(0);
    rescaleImageFileter->SetOutputMaximum(1);
    rescaleImageFileter->Update();
    // Get bounding box and relabel

    UChar3DImageType::Pointer itkImage = rescaleImageFileter->GetOutput();

    MITK_INFO << "Extracting VOI...";

    int imageLabel = 1;
    bool roiFound = false;

    UChar3DImageType::IndexType minIndex;
    minIndex.Fill(numeric_limits<UChar3DImageType::IndexValueType>::max());

    UChar3DImageType::IndexType maxIndex;
    maxIndex.Fill(numeric_limits<UChar3DImageType::IndexValueType>::min());

    itk::ImageRegionIteratorWithIndex<UChar3DImageType> iter(itkImage, itkImage->GetLargestPossibleRegion());

    for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
    {
        if (iter.Get() == imageLabel)
        {
            roiFound = true;
            iter.Set(1);

            UChar3DImageType::IndexType currentIndex = iter.GetIndex();

            for (unsigned int dim = 0; dim < 3; ++dim)
            {
                minIndex[dim] = min(currentIndex[dim], minIndex[dim]);
                maxIndex[dim] = max(currentIndex[dim], maxIndex[dim]);
            }
        }
        else
        {
            iter.Set(0);
        }
    }

    if (!roiFound)
    {
        //ProgressBar::GetInstance()->Progress(8);
        MITK_ERROR << "Didn't found segmentation labeled with " << imageLabel << "!" << endl;
        return;
    }

    //ProgressBar::GetInstance()->Progress(1);

    // Extract and pad bounding box

    typedef itk::RegionOfInterestImageFilter<UChar3DImageType, UChar3DImageType> ROIFilterType;

    ROIFilterType::Pointer roiFilter = ROIFilterType::New();
    UChar3DImageType::RegionType region;
    UChar3DImageType::SizeType size;

    for (unsigned int dim = 0; dim < 3; ++dim)
    {
        size[dim] = maxIndex[dim] - minIndex[dim] + 1;
    }

    region.SetIndex(minIndex);
    region.SetSize(size);

    roiFilter->SetInput(itkImage);
    roiFilter->SetRegionOfInterest(region);
    roiFilter->ReleaseDataFlagOn();
    roiFilter->ReleaseDataBeforeUpdateFlagOn();

    typedef itk::ConstantPadImageFilter<UChar3DImageType, UChar3DImageType> PadFilterType;

    PadFilterType::Pointer padFilter = PadFilterType::New();
    const PadFilterType::SizeValueType pad[3] = { 10, 10, 10 };

    padFilter->SetInput(roiFilter->GetOutput());
    padFilter->SetConstant(0);
    padFilter->SetPadLowerBound(pad);
    padFilter->SetPadUpperBound(pad);
    padFilter->ReleaseDataFlagOn();
    padFilter->ReleaseDataBeforeUpdateFlagOn();
    padFilter->Update();

    UChar3DImageType::Pointer roiImage = padFilter->GetOutput();

    roiImage->DisconnectPipeline();
    roiFilter = nullptr;
    padFilter = nullptr;

    // Correct origin of real geometry (changed by cropping and padding)

    typedef Geometry3D::TransformType TransformType;

    TransformType::Pointer transform = TransformType::New();
    TransformType::OutputVectorType translation;

    for (unsigned int dim = 0; dim < 3; ++dim)
        translation[dim] = (int)minIndex[dim] - (int)pad[dim];

    transform->SetIdentity();
    transform->Translate(translation);
    geometry->Compose(transform, true);

    ////ProgressBar::GetInstance()->Progress(1);

    // Median

    MITK_INFO << "Median...";

    typedef itk::BinaryMedianImageFilter<UChar3DImageType, UChar3DImageType> MedianFilterType;

    MedianFilterType::Pointer medianFilter = MedianFilterType::New();
    UChar3DImageType::SizeType radius = { 0 };

    medianFilter->SetRadius(radius);
    medianFilter->SetBackgroundValue(0);
    medianFilter->SetForegroundValue(1);
    medianFilter->SetInput(roiImage);
    medianFilter->ReleaseDataFlagOn();
    medianFilter->ReleaseDataBeforeUpdateFlagOn();
    medianFilter->Update();

    ////ProgressBar::GetInstance()->Progress(1);

    // Intelligent closing

    MITK_INFO << "Intelligent closing...";

    unsigned int surfaceRatio = (unsigned int)((1.0f - m_closing) * 100.0f);

    typedef itk::IntelligentBinaryClosingFilter<UChar3DImageType, UShort3DImageType> ClosingFilterType;

    ClosingFilterType::Pointer closingFilter = ClosingFilterType::New();

    closingFilter->SetInput(medianFilter->GetOutput());
    closingFilter->ReleaseDataFlagOn();
    closingFilter->ReleaseDataBeforeUpdateFlagOn();
    closingFilter->SetSurfaceRatio(surfaceRatio);
    closingFilter->Update();

    UShort3DImageType::Pointer closedImage = closingFilter->GetOutput();

    closedImage->DisconnectPipeline();
    roiImage = nullptr;
    medianFilter = nullptr;
    closingFilter = nullptr;

    ////ProgressBar::GetInstance()->Progress(1);

    // Gaussian blur

    MITK_INFO << "Gauss...";

    typedef itk::BinaryThresholdImageFilter<UShort3DImageType, Float3DImageType> BinaryThresholdToFloatFilterType;

    BinaryThresholdToFloatFilterType::Pointer binThresToFloatFilter = BinaryThresholdToFloatFilterType::New();

    binThresToFloatFilter->SetInput(closedImage);
    binThresToFloatFilter->SetLowerThreshold(1);
    binThresToFloatFilter->SetUpperThreshold(1);
    binThresToFloatFilter->SetInsideValue(100);
    binThresToFloatFilter->SetOutsideValue(0);
    binThresToFloatFilter->ReleaseDataFlagOn();
    binThresToFloatFilter->ReleaseDataBeforeUpdateFlagOn();

    typedef itk::DiscreteGaussianImageFilter<Float3DImageType, Float3DImageType> GaussianFilterType;

    // From the following line on, IntelliSense (VS 2008) is broken. Any idea how to fix it?
    GaussianFilterType::Pointer gaussFilter = GaussianFilterType::New();

    gaussFilter->SetInput(binThresToFloatFilter->GetOutput());
    gaussFilter->SetUseImageSpacing(true);
    gaussFilter->SetVariance(m_smoothing);
    gaussFilter->ReleaseDataFlagOn();
    gaussFilter->ReleaseDataBeforeUpdateFlagOn();

    typedef itk::BinaryThresholdImageFilter<Float3DImageType, UChar3DImageType> BinaryThresholdFromFloatFilterType;

    BinaryThresholdFromFloatFilterType::Pointer binThresFromFloatFilter = BinaryThresholdFromFloatFilterType::New();

    binThresFromFloatFilter->SetInput(gaussFilter->GetOutput());
    binThresFromFloatFilter->SetLowerThreshold(50);
    binThresFromFloatFilter->SetUpperThreshold(255);
    binThresFromFloatFilter->SetInsideValue(1);
    binThresFromFloatFilter->SetOutsideValue(0);
    binThresFromFloatFilter->ReleaseDataFlagOn();
    binThresFromFloatFilter->ReleaseDataBeforeUpdateFlagOn();
    binThresFromFloatFilter->Update();

    UChar3DImageType::Pointer blurredImage = binThresFromFloatFilter->GetOutput();

    blurredImage->DisconnectPipeline();
    closedImage = nullptr;
    binThresToFloatFilter = nullptr;
    gaussFilter = nullptr;

    //ProgressBar::GetInstance()->Progress(1);

    // Fill holes

    MITK_INFO << "Filling cavities...";

    typedef itk::ConnectedThresholdImageFilter<UChar3DImageType, UChar3DImageType> ConnectedThresholdFilterType;

    ConnectedThresholdFilterType::Pointer connectedThresFilter = ConnectedThresholdFilterType::New();

    UChar3DImageType::IndexType corner;

    corner[0] = 0;
    corner[1] = 0;
    corner[2] = 0;

    connectedThresFilter->SetInput(blurredImage);
    connectedThresFilter->SetSeed(corner);
    connectedThresFilter->SetLower(0);
    connectedThresFilter->SetUpper(0);
    connectedThresFilter->SetReplaceValue(2);
    connectedThresFilter->ReleaseDataFlagOn();
    connectedThresFilter->ReleaseDataBeforeUpdateFlagOn();

    typedef itk::BinaryThresholdImageFilter<UChar3DImageType, UChar3DImageType> BinaryThresholdFilterType;

    BinaryThresholdFilterType::Pointer binThresFilter = BinaryThresholdFilterType::New();

    binThresFilter->SetInput(connectedThresFilter->GetOutput());
    binThresFilter->SetLowerThreshold(0);
    binThresFilter->SetUpperThreshold(0);
    binThresFilter->SetInsideValue(50);
    binThresFilter->SetOutsideValue(0);
    binThresFilter->ReleaseDataFlagOn();
    binThresFilter->ReleaseDataBeforeUpdateFlagOn();

    typedef itk::AddImageFilter<UChar3DImageType, UChar3DImageType, UChar3DImageType> AddFilterType;

    AddFilterType::Pointer addFilter = AddFilterType::New();

    addFilter->SetInput1(blurredImage);
    addFilter->SetInput2(binThresFilter->GetOutput());
    addFilter->ReleaseDataFlagOn();
    addFilter->ReleaseDataBeforeUpdateFlagOn();
    addFilter->Update();

    //ProgressBar::GetInstance()->Progress(1);

    // Surface extraction

    MITK_INFO << "Surface extraction...";

    Image::Pointer filteredImage = Image::New();
    CastToMitkImage(addFilter->GetOutput(), filteredImage);

    filteredImage->SetGeometry(geometry);

    ImageToSurfaceFilter::Pointer imageToSurfaceFilter = ImageToSurfaceFilter::New();

    imageToSurfaceFilter->SetInput(filteredImage);
    imageToSurfaceFilter->SetThreshold(50);
    imageToSurfaceFilter->SmoothOn();
    imageToSurfaceFilter->SetDecimate(ImageToSurfaceFilter::NoDecimation);

    m_pSurface = imageToSurfaceFilter->GetOutput(0);

    //ProgressBar::GetInstance()->Progress(1);

    // Mesh decimation

    if (m_decimation > 0.0f && m_decimation < 1.0f)
    {
        MITK_INFO << "Quadric mesh decimation...";

        vtkQuadricDecimation *quadricDecimation = vtkQuadricDecimation::New();
        quadricDecimation->SetInputData(m_pSurface->GetVtkPolyData());
        quadricDecimation->SetTargetReduction(m_decimation);
        quadricDecimation->AttributeErrorMetricOn();
        quadricDecimation->GlobalWarningDisplayOff();
        quadricDecimation->Update();

        vtkCleanPolyData *cleaner = vtkCleanPolyData::New();
        cleaner->SetInputConnection(quadricDecimation->GetOutputPort());
        cleaner->PieceInvariantOn();
        cleaner->ConvertLinesToPointsOn();
        cleaner->ConvertStripsToPolysOn();
        cleaner->PointMergingOn();
        cleaner->Update();

        m_pSurface->SetVtkPolyData(cleaner->GetOutput());
    }

    //ProgressBar::GetInstance()->Progress(1);

    // Compute Normals

    vtkPolyDataNormals *computeNormals = vtkPolyDataNormals::New();
    computeNormals->SetInputData(m_pSurface->GetVtkPolyData());
    computeNormals->SetFeatureAngle(360.0f);
    computeNormals->AutoOrientNormalsOn();
    computeNormals->FlipNormalsOff();
    computeNormals->Update();

    m_pSurface->SetVtkPolyData(computeNormals->GetOutput());
}

void SurfaceExtractView::ExtractSurface(mitk::Image* image, int smooth, bool largestConnect)
{
    IQF_MitkSurfaceTool* pSurfaceTool = (IQF_MitkSurfaceTool*)m_pMain->GetInterfacePtr(QF_MitkSurface_Tool);
    if (pSurfaceTool)
    {
        vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
        pSurfaceTool->ExtractSurface(image,polydata, smooth, largestConnect);
        m_pSurface = mitk::Surface::New();
        m_pSurface->SetVtkPolyData(polydata);
    }
}

void SurfaceExtractView::GetForeground( Float3DImageType* image, UChar3DImageType* outputImage)
{
    typedef itk::BinaryThresholdImageFilter<Float3DImageType, UInt3DImageType>  ThresholdFilterType;
    ThresholdFilterType::Pointer thFilter = ThresholdFilterType::New();
    thFilter->SetLowerThreshold(-300);
    thFilter->SetUpperThreshold(1500);
    thFilter->SetInsideValue(1);
    thFilter->SetOutsideValue(0);
    thFilter->SetInput(image);
    thFilter->UpdateLargestPossibleRegion();

    typedef itk::LabelShapeKeepNObjectsImageFilter< UInt3DImageType > LabelShapeKeepNObjectsImageFilterType;
    LabelShapeKeepNObjectsImageFilterType::Pointer labelShapeKeepNObjectsImageFilter = LabelShapeKeepNObjectsImageFilterType::New();
    labelShapeKeepNObjectsImageFilter->SetInput(thFilter->GetOutput());
    labelShapeKeepNObjectsImageFilter->SetBackgroundValue(0);
    labelShapeKeepNObjectsImageFilter->SetNumberOfObjects(1);
    labelShapeKeepNObjectsImageFilter->SetAttribute(LabelShapeKeepNObjectsImageFilterType::LabelObjectType::NUMBER_OF_PIXELS);

    typedef itk::RescaleIntensityImageFilter< UInt3DImageType, UChar3DImageType > RescaleFilterType;
    RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
    rescaleFilter->SetOutputMinimum(0);
    rescaleFilter->SetOutputMaximum(1);
    rescaleFilter->SetInput(labelShapeKeepNObjectsImageFilter->GetOutput());
    rescaleFilter->Update();

    outputImage->Graft(rescaleFilter->GetOutput());
}

void SurfaceExtractView::ShowResult()
{
    DataNode::Pointer node = DataNode::New();

    bool wireframe = false;

    if (wireframe)
    {
        VtkRepresentationProperty *representation =
            dynamic_cast<VtkRepresentationProperty *>(node->GetProperty("material.representation"));

        if (representation != nullptr)
            representation->SetRepresentationToWireframe();
    }

    node->SetProperty("opacity", FloatProperty::New(0.5));
    node->SetProperty("line width", IntProperty::New(1));
    node->SetProperty("scalar visibility", BoolProperty::New(false));

    std::string groupNodeName = "surface";
    DataNode *groupNode = m_pImageSelector->GetSelectedNode();

    if (groupNode != nullptr)
        groupNode->GetName(groupNodeName);

    node->SetProperty("name", StringProperty::New(m_SurfaceName.toStdString()));
    node->SetData(m_pSurface);

    BaseProperty *colorProperty = groupNode->GetProperty("color");

    node->SetProperty("color", ColorProperty::New(1.0f, 1.0f, 0.0f));

    bool showResult = true;

    bool syncVisibility = false;

    Image::Pointer image = dynamic_cast<mitk::Image*>(m_pImageSelector->GetSelectedNode()->GetData());

    BaseProperty *organTypeProperty = image->GetProperty("organ type");

    if (organTypeProperty != nullptr)
        m_pSurface->SetProperty("organ type", organTypeProperty);

    BaseProperty *visibleProperty = groupNode->GetProperty("visible");

    if (visibleProperty != nullptr && syncVisibility)
        node->ReplaceProperty("visible", visibleProperty->Clone());
    else
        node->SetProperty("visible", BoolProperty::New(showResult));

    GetDataStorage()->Add(node, groupNode);
}


void SurfaceExtractView::OnSurfaceCalculationDone()
{
    mitk::StatusBar::GetInstance()->Clear();
}
