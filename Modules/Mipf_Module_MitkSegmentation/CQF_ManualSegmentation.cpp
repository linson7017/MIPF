#include "CQF_ManualSegmentation.h"

#include <QtConcurrent>

#include <mitkLookupTableProperty.h>
#include <mitkLevelWindowProperty.h>
#include <mitkImagePixelAccessor.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkLabelSetImage.h>
#include <mitkImageWriteAccessor.h>
#include <mitkVtkResliceInterpolationProperty.h>
#include <mitkToolManager.h>
#include <mitkToolManagerProvider.h>
#include <mitkDataNode.h>

#include <QmitkNewSegmentationDialog.h>
//#include "QmitkSegmentationOrganNamesHandling.cpp"
//#include "mitkDICOMSegmentationPropertyHelper.cpp"

#include <MitkMain/IQF_MitkDataManager.h>
#include <MitkMain/IQF_MitkReference.h>

#include <MitkSegmentation/mitk_seg_msg.h>
#include <iqf_main.h>

int CQF_ManualSegmentation::GetToolIdByToolName(const std::string &toolName)
{
    // find tool from toolname
    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    int numberOfTools = toolManager->GetTools().size();
    int toolId = 0;
    // std::vector<std::string> toolnames;
    for (; toolId < numberOfTools; ++toolId)
    {
        mitk::Tool *currentTool = toolManager->GetToolById(toolId);
        // toolnames.push_back(currentTool->GetName());
        if (toolName.compare(currentTool->GetName()) == 0)
        {
            return toolId;
        }
    }
    return -1;
}

CQF_ManualSegmentation::CQF_ManualSegmentation(QF::IQF_Main* pMain):m_pMain(pMain), 
m_pToolManager(NULL), 
m_SurfaceInterpolator(NULL),
m_bSurfaceInterpolateInited(false)
{
    m_surfaceInterpolatorWatcher = new SurfaceInterpolatorWatcher(pMain);
}


CQF_ManualSegmentation::~CQF_ManualSegmentation()
{
    delete m_surfaceInterpolatorWatcher;
}

void CQF_ManualSegmentation::SetSurfaceInterpolateOn(bool bEnableInterpolate)
{
    if (bEnableInterpolate)
    {
        if (m_SurfaceInterpolator.IsNull())
        {
            m_SurfaceInterpolator = mitk::SurfaceInterpolationController::GetInstance();
        }  
        if (m_pToolManager->GetWorkingData(0))
        {
            InitializeSurfaceInterpolate(m_pToolManager->GetWorkingData(0));
        }
        IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        itk::ReceptorMemberCommand<CQF_ManualSegmentation>::Pointer command2 = itk::ReceptorMemberCommand<CQF_ManualSegmentation>::New();
        command2->SetCallbackFunction(this, &CQF_ManualSegmentation::OnSurfaceInterpolationInfoChanged);
        SurfaceInterpolationInfoChangedObserverTag = m_SurfaceInterpolator->AddObserver(itk::ModifiedEvent(), command2);
        m_SurfaceInterpolator->SetDataStorage(pMitkDataManager->GetDataStorage());
        m_surfaceInterpolatorWatcher->SetSurfaceInterpolator(m_SurfaceInterpolator.GetPointer());
        QObject::connect(&m_Watcher, SIGNAL(finished()), m_surfaceInterpolatorWatcher, SLOT(OnSurfaceInterpolationFinished()));
        OnSurfaceInterpolationInfoChanged(itk::AnyEvent());
    }
    else
    {
        if (m_SurfaceInterpolator.IsNull())
        {
            m_SurfaceInterpolator = mitk::SurfaceInterpolationController::GetInstance();
        }
        m_SurfaceInterpolator->RemoveObserver(SurfaceInterpolationInfoChangedObserverTag);
        QObject::disconnect(&m_Watcher, SIGNAL(finished()), m_surfaceInterpolatorWatcher, SLOT(OnSurfaceInterpolationFinished()));
        m_SurfaceInterpolator = NULL;
    }
}

void CQF_ManualSegmentation::Initialize()
{
    IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    m_pToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    m_pToolManager->SetDataStorage(*(pMitkDataManager->GetDataStorage()));
    m_pToolManager->InitializeTools();
    m_pToolManager->RegisterClient();
}

void CQF_ManualSegmentation::Deinitialize()
{
    m_pToolManager->UnregisterClient();
}

mitk::ToolManager* CQF_ManualSegmentation::GetToolManager()
{
    return m_pToolManager;
}

void CQF_ManualSegmentation::SetReferenceData(const mitk::DataNode* node)
{
    m_pToolManager->SetReferenceData((mitk::DataNode*)node);
}

void CQF_ManualSegmentation::SetWorkingData(const mitk::DataNode* node)
{
    m_pToolManager->SetWorkingData((mitk::DataNode*)node);
    if (m_SurfaceInterpolator.IsNotNull())
    {
        InitializeSurfaceInterpolate(node);
    }
}

mitk::DataNode* CQF_ManualSegmentation::GetReferenceData()
{
    return m_pToolManager->GetReferenceData(0);
}

mitk::DataNode* CQF_ManualSegmentation::GetWorkingData()
{
    return m_pToolManager->GetWorkingData(0);
}

mitk::Tool* CQF_ManualSegmentation::ChangeTool(const char* szToolID)
{
    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    if (strcmp(szToolID,"")==0)
    {
        toolManager->ActivateTool(-1);
    }
    int toolID = GetToolIdByToolName(szToolID);
    toolManager->ActivateTool(toolID);
    return toolManager->GetToolById(toolID);
}

mitk::Tool* CQF_ManualSegmentation::GetActivedTool()
{
    return m_pToolManager->GetActiveTool();
}

bool CQF_ManualSegmentation::CreateSegmentationNode(const mitk::DataNode* pRefNode, mitk::DataNode* pSegmentationNode, const char* szName, SegRGBColor& rgbColor)
{
    if (!pRefNode ||!pSegmentationNode)
    {
        return false;
    }
    mitk::Image* image = dynamic_cast<mitk::Image*>(pRefNode->GetData());
    return CreateSegmentationNode(image, pSegmentationNode, szName, rgbColor);
}

bool CQF_ManualSegmentation::CreateSegmentationNode(const mitk::Image* pOriginImage, mitk::DataNode* pSegmentationNode, const char* szName, SegRGBColor& rgbColor)
{
    IQF_MitkReference* m_pReference = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);
    if (!pOriginImage)
    {
        return false;
    }

    mitk::Color color(rgbColor.data());

    mitk::PixelType pixelType(mitk::MakeScalarPixelType<mitk::Label::PixelType>());
    mitk::LabelSetImage::Pointer segmentation = mitk::LabelSetImage::New();
    if (pOriginImage->GetDimension() == 2)
    {
        const unsigned int dimensions[] = { pOriginImage->GetDimension(0), pOriginImage->GetDimension(1), 1 };
        segmentation->Initialize(pixelType, 3, dimensions);
        segmentation->AddLayer();
    }
    else
    {
        segmentation->Initialize(pOriginImage);
    }

    mitk::Label::Pointer label = mitk::Label::New();
    label->SetName(szName);
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

    if (pOriginImage->GetTimeGeometry())
    {
        mitk::TimeGeometry::Pointer originalGeometry = pOriginImage->GetTimeGeometry()->Clone();
        segmentation->SetTimeGeometry(originalGeometry);
    }
    else
    {
        //mitk::Tool::ErrorMessage("Original image does not have a 'Time sliced geometry'! Cannot create a segmentation.");
        return false;
    }

    pSegmentationNode->SetData(segmentation);
    // name
    pSegmentationNode->SetProperty("name", mitk::StringProperty::New(szName));

    // visualization properties
    pSegmentationNode->SetProperty("binary", mitk::BoolProperty::New(true));
    pSegmentationNode->SetProperty("color", mitk::ColorProperty::New(color));
    mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
    lut->SetType(mitk::LookupTable::MULTILABEL);
    mitk::LookupTableProperty::Pointer lutProp = mitk::LookupTableProperty::New();
    lutProp->SetLookupTable(lut);
    pSegmentationNode->SetProperty("LookupTable", lutProp);
    pSegmentationNode->SetProperty("texture interpolation", mitk::BoolProperty::New(false));
    //pSegmentationNode->SetProperty("layer", mitk::IntProperty::New(10));
    pSegmentationNode->SetProperty("levelwindow", mitk::LevelWindowProperty::New(mitk::LevelWindow(0.5, 1)));
    pSegmentationNode->SetProperty("opacity", mitk::FloatProperty::New(0.3));
    pSegmentationNode->SetProperty("segmentation", mitk::BoolProperty::New(true));
    pSegmentationNode->SetProperty("reslice interpolation",
        mitk::VtkResliceInterpolationProperty::New());
    pSegmentationNode->SetProperty("showVolume", mitk::BoolProperty::New(true));

    return true;
}

bool CQF_ManualSegmentation::CreateLabelSetImageNode(const mitk::DataNode* pRefNode, mitk::DataNode* pLabelSetNode, const char* szName)
{
    if (!pRefNode || !pLabelSetNode)
    {
        return false;
    }
    mitk::Image* image = dynamic_cast<mitk::Image*>(pRefNode->GetData());
    return CreateSegmentationNode(image, pLabelSetNode, szName);
}
bool CQF_ManualSegmentation::CreateLabelSetImageNode(const mitk::Image* pOriginImage, mitk::DataNode* pLabelSetNode, const char* szName)
{
    m_pToolManager->ActivateTool(-1);

    assert(pOriginImage);

    mitk::LabelSetImage::Pointer workingImage = mitk::LabelSetImage::New();

    try
    {
        workingImage->Initialize(pOriginImage);
    }
    catch (mitk::Exception &e)
    {
        return false;
    }

    mitk::DataNode::Pointer workingNode = mitk::DataNode::New();
    pLabelSetNode->SetData(workingImage);
    pLabelSetNode->SetName(szName);
    /*workingImage->GetExteriorLabel()->SetProperty("name.parent",
        mitk::StringProperty::New(referenceNode->GetName().c_str()));*/
    workingImage->GetExteriorLabel()->SetProperty("name.image", mitk::StringProperty::New(szName));
    // Set DICOM SEG properties for segmentation sesion
    //mitk::PropertyList::Pointer dicomSegPropertyList =
    //    mitk::DICOMSegmentationPropertyHandler::GetDICOMSegmentationProperties(pOriginImage->GetPropertyList());
    //workingImage->GetPropertyList()->ConcatenatePropertyList(dicomSegPropertyList);
    return true;
}

void CQF_ManualSegmentation::OnSurfaceInterpolationInfoChanged(const itk::EventObject &)
{
    if (m_SurfaceInterpolator)
    {
        if (m_Watcher.isRunning())
            m_Watcher.waitForFinished();
        m_Future = QtConcurrent::run(this, &CQF_ManualSegmentation::Run3DInterpolation);
        m_Watcher.setFuture(m_Future);
    }    
}

void CQF_ManualSegmentation::Run3DInterpolation()
{
    m_SurfaceInterpolator->Interpolate();
}

void CQF_ManualSegmentation::InitializeSurfaceInterpolate(const mitk::DataNode* imageNode)
{
    if (!imageNode)
    {
        return;
    }
    mitk::Vector3D spacing = imageNode->GetData()->GetGeometry()->GetSpacing();
    double minSpacing(100);
    double maxSpacing(0);
    for (int i = 0; i < 3; i++)
    {
        if (spacing[i] < minSpacing)
        {
            minSpacing = spacing[i];
        }
        else if (spacing[i] > maxSpacing)
        {
            maxSpacing = spacing[i];
        }
    }

    m_SurfaceInterpolator->SetMaxSpacing(maxSpacing);
    m_SurfaceInterpolator->SetMinSpacing(minSpacing);
    m_SurfaceInterpolator->SetDistanceImageVolume(50000);

    mitk::Image *segmentationImage = dynamic_cast<mitk::Image *>(imageNode->GetData());
    m_SurfaceInterpolator->SetCurrentInterpolationSession(segmentationImage);
}

void SurfaceInterpolatorWatcher::OnSurfaceInterpolationFinished()
{
    mitk::Surface::Pointer interpolatedSurface = m_pSurfaceInterpolator->GetInterpolationResult();
    if (interpolatedSurface.IsNotNull())
    {
        m_pMain->SendMessageQf(MITK_MESSAGE_SURFACE_INTERPOLATION_FINISHED, 1,
            interpolatedSurface.GetPointer());
    }
    else
    {
        m_pMain->SendMessageQf(MITK_MESSAGE_SURFACE_INTERPOLATION_FINISHED, 0,
            NULL);
    }
}
