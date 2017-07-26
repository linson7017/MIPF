#include "ImageCropper.h"

//mitk

#include "mitkBaseRenderer.h"
#include "mitkBoundingShapeCropper.h"
#include "mitkLabelSetImage.h"
#include "mitkBoundingShapeObjectFactory.h"


#include "MitkMain/IQF_MitkDataManager.h"

#include "Common/app_env.h"

#include "iqf_main.h"
ImageCropper::ImageCropper(QF::IQF_Main* pMain) :m_pMain(pMain)
{
    mitk::RegisterBoundingShapeObjectFactory();
    CreateBoundingShapeInteractor(false);
}


ImageCropper::~ImageCropper()
{
    if (m_BoundingShapeInteractor != nullptr)
    {
        m_BoundingShapeInteractor->SetDataNode(nullptr);
        m_BoundingShapeInteractor->EnableInteraction(false);
    }
}

void ImageCropper::EnableInteraction(bool bEnable)
{
    m_BoundingShapeInteractor->EnableInteraction(bEnable);
}

void ImageCropper::SetDataNode(mitk::DataNode* pNode)
{
    m_BoundingShapeInteractor->SetDataNode(pNode);
}

void ImageCropper::CreateBoundingBoxNode(mitk::DataNode* pInNode, mitk::DataNode*pOutNode, const char* szName, bool bAutoUse)
{
    if (pInNode|| pOutNode)
    {
        bool ok = false;


        // to do: check whether stdmulti.widget is valid
        // get current timestep to support 3d+t images //to do: check if stdmultiwidget is valid
        int timeStep = mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1"))->GetTimeStep();
        mitk::BaseGeometry::Pointer imageGeometry = static_cast<mitk::BaseGeometry*>(pInNode->GetData()->GetGeometry(timeStep));

        mitk::GeometryData::Pointer pCroppingObject = mitk::GeometryData::New();
        pCroppingObject->SetGeometry(static_cast<mitk::Geometry3D*>(this->InitializeWithImageGeometry(imageGeometry)));
        pOutNode->SetData(pCroppingObject);
        pOutNode->SetProperty("name", mitk::StringProperty::New(szName));
        pOutNode->SetProperty("color", mitk::ColorProperty::New(1.0, 1.0, 1.0));
        pOutNode->SetProperty("opacity", mitk::FloatProperty::New(0.6));
        pOutNode->SetProperty("layer", mitk::IntProperty::New(99));
        pOutNode->AddProperty("handle size factor", mitk::DoubleProperty::New(1.0 / 40.0));
        pOutNode->SetBoolProperty("pickable", true);

        IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        if (!pMitkDataManager->GetDataStorage()->Exists(pOutNode)&& bAutoUse)
        {
            pMitkDataManager->GetDataStorage()->Add(pOutNode, pInNode);
            pOutNode->SetVisibility(true);
    
            m_BoundingShapeInteractor->SetDataNode(pOutNode);
        }
    }
    // Adjust coordinate system by doing a reinit on
    auto tempDataStorage = mitk::DataStorage::SetOfObjects::New();
    tempDataStorage->InsertElement(0, pOutNode);
}

void ImageCropper::CreateBoundingShapeInteractor(bool rotationEnabled)
{
    std::string configPath = m_pMain->GetConfigPath();
    configPath.append("/mitk/Interactions/");
    if (m_BoundingShapeInteractor.IsNull())
    {
        m_BoundingShapeInteractor = mitk::BoundingShapeInteractor::New();
        bool s = m_BoundingShapeInteractor->LoadStateMachine(configPath+"BoundingShapeInteraction.xml", NULL);
        bool s2 = m_BoundingShapeInteractor->SetEventConfig(configPath+"BoundingShapeMouseConfig.xml", NULL);
        m_BoundingShapeInteractor->SetRotationEnabled(rotationEnabled);
    }
}

mitk::Geometry3D::Pointer ImageCropper::InitializeWithImageGeometry(mitk::BaseGeometry::Pointer geometry)
{
    // convert a basegeometry into a Geometry3D (otherwise IO is not working properly)
    if (geometry == nullptr)
        mitkThrow() << "Geometry is not valid.";

    auto boundingGeometry = mitk::Geometry3D::New();
    boundingGeometry->SetBounds(geometry->GetBounds());
    boundingGeometry->SetImageGeometry(geometry->GetImageGeometry());
    boundingGeometry->SetOrigin(geometry->GetOrigin());
    boundingGeometry->SetSpacing(geometry->GetSpacing());
    boundingGeometry->SetIndexToWorldTransform(geometry->GetIndexToWorldTransform());
    boundingGeometry->Modified();
    return boundingGeometry;
}

mitk::DataNode::Pointer ImageCropper::ProcessImage(mitk::DataNode* pDataNode, mitk::GeometryData* pCropGeometry, const char* szName, bool mask)
{
    // cropping only possible if valid bounding shape as well as a valid image are loaded
    // to do: check whether stdmultiwidget is valid
    int timeStep = mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1"))->GetTimeStep();

    if (pDataNode == nullptr)
    {
        return nullptr;
    }
    if (pCropGeometry == nullptr)
    {
        return nullptr;
    }

    mitk::BaseData* data = pDataNode->GetData(); //get data from node
    if (data != nullptr)
    {
        // image and bounding shape ok, set as input
        auto croppedImageNode = mitk::DataNode::New();
        auto cutter = mitk::BoundingShapeCropper::New();
        cutter->SetGeometry(pCropGeometry);

        // adjustable in advanced settings
        cutter->SetUseWholeInputRegion(mask); //either mask (mask=true) or crop (mask=false)
        cutter->SetOutsideValue(0);
        cutter->SetUseCropTimeStepOnly(false);
        cutter->SetCurrentTimeStep(timeStep);

        // TODO: Add support for MultiLayer (right now only Mulitlabel support)
        mitk::LabelSetImage* labelsetImageInput = dynamic_cast<mitk::LabelSetImage*>(data);
        if (labelsetImageInput != nullptr)
        {
            cutter->SetInput(labelsetImageInput);
            // do the actual cutting
            try
            {
                cutter->Update();
            }
            catch (const itk::ExceptionObject& e)
            {
                std::string message = std::string("The Cropping filter could not process because of: \n ") + e.GetDescription();
                return nullptr;
            }

            auto labelSetImage = mitk::LabelSetImage::New();
            labelSetImage->InitializeByLabeledImage(cutter->GetOutput());

            for (int i = 0; i < labelsetImageInput->GetNumberOfLayers(); i++)
            {
                labelSetImage->AddLabelSetToLayer(i, labelsetImageInput->GetLabelSet(i));
            }

            croppedImageNode->SetData(labelSetImage);
            croppedImageNode->SetProperty("name", mitk::StringProperty::New(szName));
            return  croppedImageNode;
        }
        else
        {
            mitk::Image::Pointer imageInput = dynamic_cast<mitk::Image*>(data);
            if (imageInput != nullptr)
            {
                cutter->SetInput(imageInput);
                // do the actual cutting
                try
                {
                    cutter->Update();
                }
                catch (const itk::ExceptionObject& e)
                {
                    std::string message = std::string("The Cropping filter could not process because of: \n ") + e.GetDescription();
                    return nullptr;
                }
                croppedImageNode->SetData(cutter->GetOutput());
                croppedImageNode->SetProperty("name", mitk::StringProperty::New(szName));
                croppedImageNode->SetProperty("color", mitk::ColorProperty::New(1.0, 0.0, 0.0));
                croppedImageNode->SetProperty("opacity", mitk::FloatProperty::New(0.4));
                croppedImageNode->SetProperty("layer", mitk::IntProperty::New(99));
                return   croppedImageNode;
            }
        }
    }
    else
    {
        return nullptr;
    }
}

