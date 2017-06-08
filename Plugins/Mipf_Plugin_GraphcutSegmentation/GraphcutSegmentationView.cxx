#include "GraphcutSegmentationView.h"
#include "iqf_main.h"
#include "Res/R.h"
#include "Utils/variant.h"

#include <QtWidgets>

#include "mitkSurface.h"
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkLabelSetImage.h>
#include <mitkContourModel.h>
#include <mitkImageCast.h>
#include <mitkContourModelSet.h>
#include <mitkToolManager.h>
#include <mitkToolManagerProvider.h>

#include <mitkImageToSurfaceFilter.h>

#include <QmitkDataStorageComboBox.h>

#include <mitkImageWriteAccessor.h>

#include <mitkVtkResliceInterpolationProperty.h>
#include <mitkLookupTableProperty.h>
#include <mitkLevelWindowProperty.h>

#include <QVTKWidget.h>
#include <QmitkRenderWindow.h>
#include <QmitkStdMultiWidget.h>

#include <vtkPolyData.h>

#include "VTK_Helpers.h"
#include "ITKVTK_Helpers.h"
#include "ITK_Helpers.h"

#include <itkRescaleIntensityImageFilter.h>
#include "itkCurvatureFlowImageFilter.h"
#include "itkSurfaceInfoCombineImageFilter.h"


mitk::NodePredicateBase::Pointer CreatePredicate(int type)
{
    auto imageType = mitk::TNodePredicateDataType<mitk::Image>::New();
    auto labelSetImageType = mitk::TNodePredicateDataType<mitk::LabelSetImage>::New();
    auto surfaceType = mitk::TNodePredicateDataType<mitk::Surface>::New();
    auto contourModelType = mitk::TNodePredicateDataType<mitk::ContourModel>::New();
    auto contourModelSetType = mitk::TNodePredicateDataType<mitk::ContourModelSet>::New();
    auto nonLabelSetImageType = mitk::NodePredicateAnd::New(imageType, mitk::NodePredicateNot::New(labelSetImageType));
    auto nonHelperObject = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"));
    auto isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    auto isSegmentation = mitk::NodePredicateProperty::New("segmentation", mitk::BoolProperty::New(true));
    auto isBinaryOrSegmentation = mitk::NodePredicateOr::New(isBinary, isSegmentation);

    mitk::NodePredicateBase::Pointer returnValue;

    switch (type)
    {
    case 1:
        returnValue = mitk::NodePredicateAnd::New(
            mitk::NodePredicateNot::New(isBinaryOrSegmentation),
            nonLabelSetImageType).GetPointer();
        break;

    case 2:
        returnValue = mitk::NodePredicateOr::New(
            mitk::NodePredicateAnd::New(imageType, isBinaryOrSegmentation),
            labelSetImageType).GetPointer();
        break;

    case 3:
        returnValue = surfaceType.GetPointer();
        break;

    case 4:
        returnValue = imageType.GetPointer();
        break;

    case 5:
        returnValue = mitk::NodePredicateOr::New(
            contourModelSetType,
            contourModelSetType).GetPointer();
        break;

    default:
        assert(false && "Unknown predefined predicate!");
        return nullptr;
    }

    return mitk::NodePredicateAnd::New(returnValue, nonHelperObject).GetPointer();
}



GraphcutSegmentationView::GraphcutSegmentationView(QF::IQF_Main* pMain) :MitkPluginView(pMain), m_bInited(false), m_tool(NULL), m_bPaintForeground(true)
{
    m_pMain->Attach(this);
    //GraphCut<Float3DImageType>* m_graphcut = new GraphCut<Float3DImageType>();
}

void GraphcutSegmentationView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "MITK_MESSAGE_GRAPHCUT_CREATE_NEW_SEGMENTATION") == 0)
    {
        //do what you want for the message
        if (!m_refImageNode)
        {
            return;
        }
        mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(m_refImageNode->GetData());
        mitk::DataNode::Pointer segNode = CreateSegmentationNode(image.GetPointer(), "", mitk::Color());
        segNode->SetColor(1, 0, 0);
        segNode->SetName("seg");
        m_pMitkDataManager->GetDataStorage()->Add(segNode);
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_GRAPHCUT_INIT") == 0)
    {
        Init();
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_GRAPHCUT_REMOVEALL") == 0)
    {
        Reset();
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_GRAPHCUT_SEGMENT") == 0)
    {
        Segment();
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_GRAPHCUT_SAVEMASK") == 0)
    {
        SaveMask();
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_GRAPHCUT_RESAMPLE") == 0)
    {
        QLineEdit* lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("GraphcutSegmentation.Resample");
        Float3DImageType::Pointer itkImage = Float3DImageType::New();
        mitk::CastToItkImage<Float3DImageType>(dynamic_cast<mitk::Image *>(m_refImageNode->GetData()), itkImage);
        Float3DImageType::Pointer resampleImage = Float3DImageType::New();
        ITKHelpers::Resample(itkImage.GetPointer(), resampleImage.GetPointer(), lineEdit->text().toInt());

        ITKHelpers::SaveImage(resampleImage.GetPointer(), "D:/temp/resample.mhd");
        mitk::Image::Pointer mitkImage = mitk::Image::New();
        mitk::CastToMitkImage<Float3DImageType>(resampleImage, mitkImage);
        mitk::DataNode::Pointer resampleNode = mitk::DataNode::New();
        std::string name = "Resample-";
        name.append(lineEdit->text().toStdString());
        resampleNode->SetData(mitkImage);
        resampleNode->SetName(name);
        resampleNode->SetColor(0, 1, 0);
        resampleNode->Update();
        m_pMitkDataManager->GetDataStorage()->Add(resampleNode);

    }
    else if (strcmp(szMessage, "MITK_MESSAGE_GRAPHCUT_LAMBDA_CHANGED") == 0)
    {
        VarientMap* vm= (VarientMap*)pValue;
        variant v = variant::GetVariant(*vm, "text");
        QString vs = v.getString();
        m_graphcut.SetLambda(vs.toDouble());
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_GRAPHCUT_HISTOGRAMBINS_CHANGED") == 0)
    {
        VarientMap* vm = (VarientMap*)pValue;
        variant v = variant::GetVariant(*vm, "text");
        QString vs = v.getString();
        m_graphcut.SetNumberOfHistogramBins(vs.toInt());
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_GRAPHCUT_PENSIZE_CHANGED") == 0)
    {
        if (m_tool)
        {
            VarientMap* vm = (VarientMap*)pValue;
            variant v = variant::GetVariant(*vm, "text");
            QString vs = v.getString();
            m_tool->SetSize(vs.toDouble());
        }
        
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_GRAPHCUT_FOREGROUND") == 0)
    {
        VarientMap* vm = (VarientMap*)pValue;
        variant v = variant::GetVariant(*vm, "checked");
        if (v.getBool())
        {
            SwitchToForeground();
        }
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_GRAPHCUT_BACKGROUND") == 0)
    {
        VarientMap* vm = (VarientMap*)pValue;
        variant v = variant::GetVariant(*vm, "checked");
        if (v.getBool())
        {
            SwitchToBackground();
        }
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_GRAPHCUT_GENERATESURFACE") == 0)
    {
        GenerateSurface();
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_GRAPHCUT_BEGIN_PAINT") == 0)
    {
        ChangeTool("Paint");
        if (m_bPaintForeground)
        {
            SwitchToForeground();
        }
        else
        {
            SwitchToBackground();
        }
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_GRAPHCUT_BEGIN_WIPE") == 0)
    {
        ChangeTool("Wipe");
        if (m_bPaintForeground)
        {
            SwitchToForeground();
        }
        else
        {
            SwitchToBackground();
        }
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_GRAPHCUT_END_PAINT") == 0)
    {
        ChangeTool("");
    }
}

void GraphcutSegmentationView::Reset()
{
    m_sources.clear();
    m_sinks.clear();

    m_pMitkDataManager->GetDataStorage()->Remove(m_pMitkDataManager->GetDataStorage()->GetNamedNode("source"));
    m_pMitkDataManager->GetDataStorage()->Remove(m_pMitkDataManager->GetDataStorage()->GetNamedNode("sink"));
    m_pMitkDataManager->GetDataStorage()->Remove(m_pMitkDataManager->GetDataStorage()->GetNamedNode("Result"));
    m_sourceImageNode=NULL;
    m_sinkImageNode= NULL;
    InitSourceAndSinkNodes();
    
}

void GraphcutSegmentationView::Init()
{
    if (!m_refImageNode|| !m_workImageNode)
    {
        return;
    }
    m_originMitkImage = dynamic_cast<mitk::Image*>(m_refImageNode->GetData());
    mitk::BaseGeometry* originImageGeometry = m_originMitkImage->GetGeometry();
    mitk::Point3D originOrigin = m_originMitkImage->GetGeometry()->GetOrigin();
    if (!m_originMitkImage)
    {
        return;
    }
    vtkImageData* originImage = m_originMitkImage->GetVtkImageData();
    double range[2];
    originImage->GetScalarRange(range);
   
    {
        Float3DImageType::Pointer itkImage = Float3DImageType::New();
        mitk::CastToItkImage<Float3DImageType>(dynamic_cast<mitk::Image *>(m_refImageNode->GetData()), itkImage);

        /*typedef itk::CurvatureFlowImageFilter< Float3DImageType, Float3DImageType > diffuseFilterType;
        diffuseFilterType::Pointer diffuseFilter = diffuseFilterType::New();
        diffuseFilter->SetInput(itkImage);
        diffuseFilter->SetNumberOfIterations(10);
        diffuseFilter->SetTimeStep(0.0625);
        try
        {
            diffuseFilter->Update();
        }
        catch (itk::ExceptionObject& ex)
        {
            std::cout << "Diffuse Image Failed! Exception code: " << std::endl;
            std::cout << ex.what() << std::endl;
            return;
        }*/


        /*typedef itk::RescaleIntensityImageFilter<Float3DImageType, Float3DImageType> RescaleIntensityImageFilterType;
        RescaleIntensityImageFilterType::Pointer rescale = RescaleIntensityImageFilterType::New();
        rescale->SetInput(diffuseFilter->GetOutput());
        rescale->SetOutputMinimum(0.0);
        rescale->SetOutputMaximum(255);
        rescale->Update();*/

        typedef itk::CastImageFilter<Float3DImageType, IntArray3DImageType> CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(itkImage);
        caster->Update();

        m_graphcut.SetImage(caster->GetOutput());
        m_graphcut.SetScalarRange(range[0], range[1]);
        m_graphcut.SetNumberOfHistogramBins((range[1] - range[0])/2);
        //m_graphcut.SetNumberOfHistogramBins(64);
    }
    InitSourceAndSinkNodes();
    InitTool();

    m_bInited = true; 
}

void GraphcutSegmentationView::InitSourceAndSinkNodes()
{
    if (m_bInited)
    {
        return;
    }
    if (1)
    {
        mitk::Color color;
        color.SetRed(0);
        color.SetGreen(1);
        color.SetBlue(0);
        m_sourceImageNode = CreateSegmentationNode(m_originMitkImage, "source", color);
        m_pMitkDataManager->GetDataStorage()->Add(m_sourceImageNode);
    }
    if (1)
    {
        mitk::Color color;
        color.SetRed(0);
        color.SetGreen(0);
        color.SetBlue(1);
        m_sinkImageNode = CreateSegmentationNode(m_originMitkImage, "sink", color);
        m_pMitkDataManager->GetDataStorage()->Add(m_sinkImageNode);
    }
    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    toolManager->SetWorkingData(m_bPaintForeground ? m_sourceImageNode : m_sinkImageNode);
}

int GetToolIdByToolName(const std::string &toolName)
{
    // find tool from toolname
    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    int numberOfTools = toolManager->GetTools().size();
    int toolId = 0;
    std::vector<std::string> toolnames;
    for (; toolId < numberOfTools; ++toolId)
    {
        mitk::Tool *currentTool = toolManager->GetToolById(toolId);
        toolnames.push_back(currentTool->GetName());
        if (toolName.compare(currentTool->GetName()) == 0)
        {
            return toolId;
        }
    }
    return -1;
}

void GraphcutSegmentationView::InitTool()
{
    if (m_tool)
    {
        return;
    }
    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    toolManager->SetDataStorage(*(m_pMitkDataManager->GetDataStorage()));
    toolManager->InitializeTools();
    toolManager->RegisterClient();

    mitk::Image* patientImage = dynamic_cast<mitk::Image*>(m_refImageNode->GetData());
   
    toolManager->SetWorkingData(m_sourceImageNode);
    toolManager->SetReferenceData(m_refImageNode);

    // load interaction events
    ChangeTool("Paint");
    m_tool->SetSize(5);
}


void GraphcutSegmentationView::ChangeTool(const QString& toolName)
{
    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    if (toolName.isEmpty())
    {
        toolManager->ActivateTool(-1);
    }
    int toolID = GetToolIdByToolName(toolName.toStdString());
    m_tool = dynamic_cast<mitk::PaintbrushTool *>(toolManager->GetToolById(toolID));
    toolManager->ActivateTool(toolID);
}

void GraphcutSegmentationView::SwitchToForeground()
{
    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    toolManager->SetWorkingData(m_sourceImageNode);
    toolManager->SetReferenceData(m_refImageNode);
    m_bPaintForeground = true;
}

void GraphcutSegmentationView::SwitchToBackground()
{
    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    toolManager->SetWorkingData(m_sinkImageNode);
    toolManager->SetReferenceData(m_refImageNode);
    m_bPaintForeground = false;
}

void GraphcutSegmentationView::ScribbleEventHandler(vtkObject* caller, long unsigned int eventId, void* callData)
{
    // Dilate the path and mark the path and the dilated pixels as part of the currently selected group
    unsigned int dilateRadius = 2;

}

void GraphcutSegmentationView::RefreshSourceAndSink()
{
    if (m_sourceImageNode.IsNull()|| m_sinkImageNode.IsNull())
    {
        return;
    }
    m_sources.clear();
    m_sinks.clear();
    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_sourceImageNode->GetData());
    if (mitkImage)
    {
        vtkImageData* sourceImage = mitkImage->GetVtkImageData();
        ITKVTKHelpers::GetNonzeroPixels(sourceImage, m_sources);
    }
    mitkImage = dynamic_cast<mitk::Image*>(m_sinkImageNode->GetData());
    if (mitkImage)
    {
        vtkImageData* sinkImage = mitkImage->GetVtkImageData();
        ITKVTKHelpers::GetNonzeroPixels(sinkImage, m_sinks);
    }

}

void GraphcutSegmentationView::SaveMask()
{
    UChar3DImageType* maskImage = m_graphcut.GetSegmentMask();
    Float3DImageType::Pointer itkImage = Float3DImageType::New();
    ITKHelpers::DeepCopy(maskImage, itkImage.GetPointer());

    Float3DImageType::Pointer image = Float3DImageType::New();
    mitk::CastToItkImage<Float3DImageType>(dynamic_cast<mitk::Image *>(m_refImageNode->GetData()), image);
    
    itkImage->SetOrigin(image->GetOrigin());
    itkImage->SetDirection(image->GetDirection());
    itkImage->SetSpacing(image->GetSpacing());

    ITKHelpers::SaveImage(itkImage.GetPointer(), "D:/temp/maskImge.mhd");
    
}

void GraphcutSegmentationView::UpdateSelections()
{
    mitk::DataNode* resultNode = m_pMitkDataManager->GetDataStorage()->GetNamedNode("Result");
    if (!resultNode)
    {
        return;
    }

}

void GraphcutSegmentationView::GenerateSurface()
{
    mitk::DataNode* resultNode = m_pMitkDataManager->GetDataStorage()->GetNamedNode("Result");
    if (!resultNode)
    {
        return;
    }
    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(resultNode->GetData());
    UChar3DImageType::Pointer resultImage = UChar3DImageType::New();
    mitk::CastToItkImage<UChar3DImageType>(mitkImage, resultImage);

    mitkImage = dynamic_cast<mitk::Image*>(m_refImageNode->GetData());
    Float3DImageType::Pointer originImage = Float3DImageType::New();
    mitk::CastToItkImage<Float3DImageType>(mitkImage, originImage);

    //diffuse origin image
    typedef itk::CurvatureFlowImageFilter< Float3DImageType, Float3DImageType > diffuseFilterType;
    diffuseFilterType::Pointer diffuseFilter = diffuseFilterType::New();
    diffuseFilter->SetInput(originImage);
    diffuseFilter->SetNumberOfIterations(5);
    diffuseFilter->SetTimeStep(0.0625);
    try
    {
        diffuseFilter->Update();
    }
    catch (itk::ExceptionObject& ex)
    {
        std::cout << "Diffuse Image Failed! Exception code: " << std::endl;
        std::cout << ex.what() << std::endl;
        return;
    }
    //calculate scalar range
    typedef itk::MinimumMaximumImageCalculator <Float3DImageType>
        imageCalculatorFilterType;
    imageCalculatorFilterType::Pointer imageCalculatorFilter
        = imageCalculatorFilterType::New();
    imageCalculatorFilter->SetImage(diffuseFilter->GetOutput());
    imageCalculatorFilter->Compute();
    double maxScalar = imageCalculatorFilter->GetMaximum();
    double minScalar = imageCalculatorFilter->GetMinimum();

    //surface info
    typedef itk::SurfaceInfoCombineImageFilter <UChar3DImageType, Float3DImageType, Float3DImageType>
        algorithmProcessImageFilterType;
    algorithmProcessImageFilterType::Pointer processFilter =
        algorithmProcessImageFilterType::New();
    processFilter->SetForeground(255);
    processFilter->SetBackground(0);
    processFilter->SetInputImage(resultImage);
    processFilter->SetInfoImage(diffuseFilter->GetOutput());
    processFilter->SetMaxScalar(maxScalar);
    processFilter->SetMinScalar(minScalar);
    try
    {
        processFilter->Update();
    }
    catch (itk::ExceptionObject& ex)
    {
        std::cout << "GrayScale Combine Failed! Exception code: " << std::endl;
        std::cout << ex.what() << std::endl;
        return;
    }
    m_ContourValue = processFilter->GetContourValue();
    m_ContourValueMin = processFilter->GetContourValueMin();
    m_ContourValueMax = processFilter->GetContourValueMax();
    RefreshContourValueRange(m_ContourValueMin, m_ContourValueMax, m_ContourValue);
    ////**************smooth****************//
    //With moderate smooth
    typedef itk::CurvatureFlowImageFilter< Float3DImageType, Float3DImageType > curvatureFlowImageFilterType;
    curvatureFlowImageFilterType::Pointer smoothing = curvatureFlowImageFilterType::New();
    smoothing->SetInput(processFilter->GetOutput());
    smoothing->SetNumberOfIterations(15);
    smoothing->SetTimeStep(0.0625);
    try
    {
        smoothing->Update();
    }
    catch (itk::ExceptionObject& ex)
    {
        std::cout << "Smooth Surface Failed! Exception code: " << std::endl;
        std::cout << ex.what() << std::endl;
        return;
    }

    typedef itk::RescaleIntensityImageFilter<Float3DImageType, Float3DImageType> castImageFilterType;
    castImageFilterType::Pointer outputCaster = castImageFilterType::New();
    outputCaster->SetInput(smoothing->GetOutput());
    outputCaster->SetOutputMinimum(minScalar);
    outputCaster->SetOutputMaximum(maxScalar);
    outputCaster->Update();

    if (!m_resultSurfaceImageNode)
    {
        m_resultSurfaceImageNode = mitk::DataNode::New();
        m_pMitkDataManager->GetDataStorage()->Add(m_resultSurfaceImageNode);
    }
    mitk::Image::Pointer resultSurfaceImage = mitk::Image::New();
    mitk::CastToMitkImage<Float3DImageType>(outputCaster->GetOutput(), resultSurfaceImage);
    m_resultSurfaceImageNode->SetData(resultSurfaceImage);
    
    vtkMarchingCubes *surfaceCreator = vtkMarchingCubes::New();
    surfaceCreator->SetInputData(resultSurfaceImage->GetVtkImageData());
    surfaceCreator->SetValue(m_ContourValue, m_ContourValue);
    surfaceCreator->Update();

    mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
    surfaceNode->SetName("Surface");
    surfaceNode->SetColor(1, 1, 0);
    mitk::Surface::Pointer surface = mitk::Surface::New();
    surface->SetVtkPolyData(surfaceCreator->GetOutput()); 
    surface->SetOrigin(mitkImage->GetGeometry()->GetOrigin());
    surfaceNode->SetData(surface);
    m_pMitkDataManager->GetDataStorage()->Add(surfaceNode);


    RequestRenderWindowUpdate();
}

void GraphcutSegmentationView::RefreshContourValueRange(double min, double max, double value)
{
    if (m_contourValueSlider)
    {
        m_contourValueSlider->setMaximum(max);
        m_contourValueSlider->setMinimum(min);
        m_contourValueSlider->setValue(value);
    }
}

void GraphcutSegmentationView::OnSelectPoint(const QVector3D& pixelIndex)
{
    //mitk::Image* image = dynamic_cast<mitk::Image*>(m_refImageNode->GetData());
    //if (image)
    //{
    //   // VTKHelpers::MakeImageTransparent(image->GetVtkImageData());
    //    int extent[6];
    //    image->GetVtkImageData()->GetExtent(extent);
    //    if (pixelIndex.x()>extent[0]&& pixelIndex.x()<extent[1]&&
    //        pixelIndex.y()>extent[2] && pixelIndex.y()<extent[3]&&
    //        pixelIndex.z()>extent[4] && pixelIndex.z()<extent[5])
    //    {
    //        QCheckBox* box = (QCheckBox*)m_pR->getObjectFromGlobalMap("main.Foreground");
    //        itk::Index<3> index;
    //        index[0] = pixelIndex[0];
    //        index[1] = pixelIndex[1];
    //        index[2] = pixelIndex[2];
    //        if (box->isChecked())
    //        {
    //            m_sources.push_back(index);
    //        }
    //        else
    //        {
    //            m_sinks.push_back(index);
    //        }
    //    }
    //    
    //}
}

mitk::DataNode::Pointer GraphcutSegmentationView::CreateSegmentationNode(mitk::Image* origin, const std::string& organName, const mitk::Color& color)
{
    if (!origin)
    {
        return nullptr;
    }

    mitk::PixelType pixelType(mitk::MakeScalarPixelType<mitk::Label::PixelType>());
    mitk::LabelSetImage::Pointer segmentation = mitk::LabelSetImage::New();
    if (origin->GetDimension() == 2)
    {
        const unsigned int dimensions[] = { origin->GetDimension(0), origin->GetDimension(1), 1 };
        segmentation->Initialize(pixelType, 3, dimensions);
        segmentation->AddLayer();
    }
    else
    {
        segmentation->Initialize(origin);
    }

    mitk::Label::Pointer label = mitk::Label::New();
    label->SetName(organName);
    label->SetColor(color);
    label->SetValue(1);
    segmentation->GetActiveLabelSet()->AddLabel(label);
    segmentation->GetActiveLabelSet()->SetActiveLabel(1);

    unsigned int byteSize = sizeof(mitk::Label::PixelType);

    if (segmentation->GetDimension() < 4)
    {
        for (unsigned int dim = 0; dim < segmentation->GetDimension(); ++dim)
        {
            byteSize *= segmentation->GetDimension(dim);
        }

        mitk::ImageWriteAccessor writeAccess(segmentation.GetPointer(), segmentation->GetVolumeData(0));

        memset(writeAccess.GetData(), 0, byteSize);
    }
    else
    {
        // if we have a time-resolved image we need to set memory to 0 for each time step
        for (unsigned int dim = 0; dim < 3; ++dim)
        {
            byteSize *= segmentation->GetDimension(dim);
        }

        for (unsigned int volumeNumber = 0; volumeNumber < segmentation->GetDimension(3); volumeNumber++)
        {
            mitk::ImageWriteAccessor writeAccess(segmentation.GetPointer(), segmentation->GetVolumeData(volumeNumber));

            memset(writeAccess.GetData(), 0, byteSize);
        }
    }

    if (origin->GetTimeGeometry())
    {
        mitk::TimeGeometry::Pointer originalGeometry = origin->GetTimeGeometry()->Clone();
        segmentation->SetTimeGeometry(originalGeometry);
    }
    else
    {
        //mitk::Tool::ErrorMessage("Original image does not have a 'Time sliced geometry'! Cannot create a segmentation.");
        return nullptr;
    }

    mitk::DataNode::Pointer segmentationNode = mitk::DataNode::New();
    segmentationNode->SetData(segmentation);

    // name
    segmentationNode->SetProperty("name", mitk::StringProperty::New(organName));

    // visualization properties
    segmentationNode->SetProperty("binary", mitk::BoolProperty::New(true));
    segmentationNode->SetProperty("color", mitk::ColorProperty::New(color));
    mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
    lut->SetType(mitk::LookupTable::MULTILABEL);
    mitk::LookupTableProperty::Pointer lutProp = mitk::LookupTableProperty::New();
    lutProp->SetLookupTable(lut);
    segmentationNode->SetProperty("LookupTable", lutProp);
    segmentationNode->SetProperty("texture interpolation", mitk::BoolProperty::New(false));
    segmentationNode->SetProperty("layer", mitk::IntProperty::New(10));
    segmentationNode->SetProperty("levelwindow", mitk::LevelWindowProperty::New(mitk::LevelWindow(0.5, 1)));
    segmentationNode->SetProperty("opacity", mitk::FloatProperty::New(0.3));
    segmentationNode->SetProperty("segmentation", mitk::BoolProperty::New(true));
    segmentationNode->SetProperty("reslice interpolation",
        mitk::VtkResliceInterpolationProperty::New()); 
    segmentationNode->SetProperty("showVolume", mitk::BoolProperty::New(true));

    return segmentationNode;
}

void GraphcutSegmentationView::Segment()
{
    RefreshSourceAndSink();
    m_graphcut.SetSources(m_sources);
    m_graphcut.SetSinks(m_sinks);
    m_graphcut.PerformSegmentation();

    UChar3DImageType* maskImage = m_graphcut.GetSegmentMask();
    Float3DImageType::Pointer itkImage = Float3DImageType::New();
    ITKHelpers::DeepCopy(maskImage, itkImage.GetPointer());

    Float3DImageType::Pointer image = Float3DImageType::New();
    mitk::CastToItkImage<Float3DImageType>(dynamic_cast<mitk::Image *>(m_refImageNode->GetData()), image);

    itkImage->SetOrigin(image->GetOrigin());
    itkImage->SetDirection(image->GetDirection());
    itkImage->SetSpacing(image->GetSpacing());


    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitk::CastToMitkImage<Float3DImageType>(itkImage,mitkImage);

    m_pMitkDataManager->GetDataStorage()->Remove(m_pMitkDataManager->GetDataStorage()->GetNamedNode("Result"));
    mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
    resultNode->SetName("Result");
    resultNode->SetColor(1, 0, 0);
    resultNode->SetData(mitkImage);
    resultNode->Update();
    m_pMitkDataManager->GetDataStorage()->Add(resultNode);
    resultNode->SetOpacity(0.5);
    RequestRenderWindowUpdate();
}

void GraphcutSegmentationView::InitResource(R* pR)
{
    m_imageComboBox = new QmitkDataStorageComboBox(m_pMitkDataManager->GetDataStorage(),CreatePredicate(4));
    connect(m_imageComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnImageSelectionChanged(const mitk::DataNode *)));

    m_workingImageComboBox = new QmitkDataStorageComboBox(m_pMitkDataManager->GetDataStorage(), CreatePredicate(4));
    connect(m_workingImageComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnWorkingImageSelectionChanged(const mitk::DataNode *)));

    m_pR->registerCustomWidget("ImageComboBox", m_imageComboBox);
    m_pR->registerCustomWidget("WorkImageComboBox", m_workingImageComboBox);

    
}

void GraphcutSegmentationView::Contructed(R* pR)
{
    m_contourValueSlider = (QSlider*)pR->getObjectFromGlobalMap("GraphcutSegmentation.ContourValueSlider");
    if (m_contourValueSlider)
    {
        connect(m_contourValueSlider, &QSlider::valueChanged, this, &GraphcutSegmentationView::OnContourValueChanged);
    }
}

void GraphcutSegmentationView::OnContourValueChanged(int value)
{
    if (!m_resultSurfaceImageNode)
    {
        return;
    }
    mitk::Image* image = dynamic_cast<mitk::Image*>(m_resultSurfaceImageNode->GetData());
    if (!image)
    {
        return;
    }
    mitk::DataNode::Pointer surfaceNode = m_pMitkDataManager->GetDataStorage()->GetNamedNode("Surface");
    if (!surfaceNode)
    {
        return;
    }

    vtkMarchingCubes *surfaceCreator = vtkMarchingCubes::New();
    surfaceCreator->SetInputData(image->GetVtkImageData());
    surfaceCreator->SetValue(value, value);
    surfaceCreator->Update();

 
    mitk::Surface::Pointer surface = mitk::Surface::New();
    surface->SetVtkPolyData(surfaceCreator->GetOutput());
    surface->SetOrigin(image->GetGeometry()->GetOrigin());
    surfaceNode->SetData(surface);

    RequestRenderWindowUpdate();
}

void GraphcutSegmentationView::OnImageSelectionChanged(const mitk::DataNode* node)
{
    m_refImageNode = (mitk::DataNode*)node;
    m_bInited = false;
}

void GraphcutSegmentationView::OnWorkingImageSelectionChanged(const mitk::DataNode* node)
{
    m_workImageNode = (mitk::DataNode*)node;
    m_bInited = false;
}