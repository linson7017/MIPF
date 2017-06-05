#include "ImageRegistrationView.h"
#include "iqf_main.h"
#include "Res/R.h"
#include "Utils/variant.h"
//qt
#include <QtWidgets>

//mitk
#include "mitkImage.h"
#include "mitkImageCast.h"
#include "mitkSurface.h"

#include "mitkNodePredicateData.h"
//qmitk
#include <QmitkDataStorageComboBox.h> 

#include "MitkMain/IQF_MitkDataManager.h"
#include "MitkMain/IQF_MitkRenderWindow.h"
#include "Grid/Math/IQF_MathUtil.h"

#include "vtkPolyData.h"

#include "itkBinaryThresholdImageFilter.h"
#include "vnl/vnl_matrix.h"

#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkLabelSetImage.h>
#include <mitkContourModel.h>
#include <mitkContourModelSet.h>

#include "MatrixUtil.h"
#include "Registration_MI.h"
#include "RegistrationWorkStation.h"

#include "QmitkStdMultiWidget.h"

#include "usGetModuleContext.h"
#include "usModuleContext.h"
#include <usModuleInitialization.h>
#include "ImageNavigationInteractor.h"

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


ImageRegistrationView::ImageRegistrationView(QF::IQF_Main* pMain):MitkPluginView(pMain), m_FixedImageNode(NULL),m_MovingImageNode(NULL), m_bInited(false),
m_EventConfig("DisplayConfigMITK.xml"), m_ScrollEnabled(true)
{
    m_pMain->Attach(this);
    m_registrationMatrix.setToIdentity();

    qRegisterMetaType<QfResult>("QfResult");
}

ImageRegistrationView::~ImageRegistrationView()
{

}


void ImageRegistrationView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "ImageRegistration.Register") == 0)
    {
        DoRegistration();
    }
    else if (strcmp(szMessage, "ImageRegistration.AlignCenter") == 0)
    {
        
       // mitk::Point3D movingImageCenter = m_MovingImageNode->GetData()->GetGeometry()->GetCenter();
        //mitk::Point3D fixedImageCenter = m_FixedImageNode->GetData()->GetGeometry()->GetCenter();

        //mitk::Vector3D translate;
        //translate[0] = +fixedImageCenter[0] - movingImageCenter[0];
        //translate[1] = +fixedImageCenter[1] - movingImageCenter[1];
        //translate[2] = +fixedImageCenter[2] - movingImageCenter[2];
        //m_MovingImageNode->GetData()->GetGeometry()->Translate(translate);
        //mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    else if (strcmp(szMessage, "ImageRegistration.InitRegister") == 0)
    {
        IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        //Set Current Image
        if (!m_FixedImageNode || !m_MovingImageNode)
        {
            return;
        }
        mitk::Point3D origin;
        origin.SetElement(0, 0);
        origin.SetElement(1, 0);
        origin.SetElement(2, 0);

        Float3DImagePointerType itkFixedImage = Float3DImageType::New();
        mitk::CastToItkImage<Float3DImageType>(dynamic_cast<mitk::Image *>(m_FixedImageNode->GetData()), itkFixedImage);

        Float3DImagePointerType itkMovingImage = Float3DImageType::New();
        mitk::CastToItkImage<Float3DImageType>(dynamic_cast<mitk::Image *>(m_MovingImageNode->GetData()), itkMovingImage);

        if (!m_bInited)
        {
            InitRegistration(itkFixedImage, itkMovingImage);
        }
    }
    else if (strcmp(szMessage, "ImageRegistration.ResetRegister") == 0)
    {
        Reset();
    }
    else if (strcmp(szMessage, "ImageRegistration.StopRegister") == 0)
    {
        Stop();
    }
    else if (strcmp(szMessage, "ImageRegistration.EndRegister") == 0)
    {
        EndRegistration();
    }
    else if (strcmp(szMessage, MITK_MESSAGE_NODE_REMOVED) == 0)
    {
        mitk::DataNode* removedNode = (mitk::DataNode*)pValue;
        if (removedNode== m_pMitkDataManager->GetDataStorage()->GetNamedNode("DisplayMoving")||
            removedNode == m_pMitkDataManager->GetDataStorage()->GetNamedNode("DisplayFixed"))
        {
            m_bInited = false;
        }
    }
}

void ImageRegistrationView::DoRegistration()
{
    IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    //Set Current Image
    if (!m_FixedImageNode || !m_MovingImageNode)
    {
        return;
    }
    //correct the origin of the fixed image
    bool correctCenter = false;
    QCheckBox* cb = (QCheckBox*)m_pR->getObjectFromGlobalMap("ImageRegistration.CorrectFixedCenter");
    correctCenter = cb->isChecked();
    if (correctCenter)
    {
        mitk::Point3D preFixedOrigin = m_FixedImageNode->GetData()->GetGeometry()->GetOrigin();
        mitk::Point3D preMovingOrigin = m_MovingImageNode->GetData()->GetGeometry()->GetOrigin();
        mitk::Point3D origin;
        origin[0] = 0.0;
        origin[1] = 0.0;
        origin[2] = 0.0;
        m_FixedImageNode->GetData()->GetGeometry()->SetOrigin(origin);
        origin[0] = preMovingOrigin[0] - preFixedOrigin[0];
        origin[1] = preMovingOrigin[1] - preFixedOrigin[1];
        origin[2] = preMovingOrigin[2] - preFixedOrigin[2];
        m_MovingImageNode->GetData()->GetGeometry()->SetOrigin(origin);
    }



    Float3DImagePointerType itkFixedImage = Float3DImageType::New();
    mitk::CastToItkImage<Float3DImageType>(dynamic_cast<mitk::Image *>(m_FixedImageNode->GetData()), itkFixedImage);

    Float3DImagePointerType itkMovingImage = Float3DImageType::New();
    mitk::CastToItkImage<Float3DImageType>(dynamic_cast<mitk::Image *>(m_MovingImageNode->GetData()), itkMovingImage);

    if (!m_bInited || !m_pMitkDataManager->GetDataStorage()->GetNamedNode("DisplayMoving") || !m_pMitkDataManager->GetDataStorage()->GetNamedNode("DisplayFixed"))
    {
        InitRegistration(itkFixedImage, itkMovingImage);
    }


    //init matrix
    QMatrix4x4 qm;
    qm.setToIdentity();
    bool alignCenter = false;
    cb = (QCheckBox*)m_pR->getObjectFromGlobalMap("ImageRegistration.AlignCenter");
    alignCenter = cb->isChecked();
    //QMatrix4x4 initMatrix = m_registrationMatrix;
    QMatrix4x4 initMatrix = static_cast<ImageNavigationInteractor*>(m_movingImageInteractor.GetPointer())->GetTransformMatrix();
    if (alignCenter)
    {
        initMatrix.setToIdentity();
        mitk::Point3D movingImageCenter = m_MovingImageNode->GetData()->GetGeometry()->GetCenter();
        mitk::Point3D fixedImageCenter = m_FixedImageNode->GetData()->GetGeometry()->GetCenter();
        initMatrix.translate(fixedImageCenter[0] - movingImageCenter[0],
            fixedImageCenter[1] - movingImageCenter[1],
            fixedImageCenter[2] - movingImageCenter[2]);
    }

    //use multi resolution
    QCheckBox* useMultiResolutionCheckbox = (QCheckBox*)m_pR->getObjectFromGlobalMap("ImageRegistration.UseMultiResolution");

    ///////////////Normal Version///////////////////////////
    /* RegistrationMI<Float3DImageType, Float3DImageType> rm;
    rm.Start(itkFixedImage.GetPointer(), itkMovingImage.GetPointer(), itkResultImage.GetPointer(), m);*/

    ///////////MultiThread Version////////////////////////
    cb = (QCheckBox*)m_pR->getObjectFromGlobalMap("ImageRegistration.DeformableRegistration");
    m_RegistrationWorkStation = new RegistrationWorkStation;
    if (cb->isChecked())
    {
        m_RegistrationWorkStation->SetRegistrationType(RegistrationWorkStation::SFD);
    }
    else
    {
        m_RegistrationWorkStation->SetRegistrationType(RegistrationWorkStation::MI);
    } 
    cb = (QCheckBox*)m_pR->getObjectFromGlobalMap("ImageRegistration.BeginWithTranslation");
    m_RegistrationWorkStation->SetBeginWithTranslation(cb->isChecked());

    m_RegistrationWorkStation->SetUseMultiResolution(useMultiResolutionCheckbox->isChecked());
    m_RegistrationThread = new QThread;
    m_RegistrationWorkStation->moveToThread(m_RegistrationThread);
    disconnect(m_RegistrationThread, &QThread::finished,
        m_RegistrationThread, &QThread::deleteLater);
    disconnect(m_RegistrationThread, &QThread::finished,
        m_RegistrationWorkStation, &QThread::deleteLater);

    connect(m_RegistrationThread, &QThread::finished,
        m_RegistrationThread, &QThread::deleteLater);
    connect(m_RegistrationThread, &QThread::finished,
        m_RegistrationWorkStation, &QThread::deleteLater);

    qRegisterMetaType<Float3DImagePointerType>("Float3DImagePointerType");
    connect(this, &ImageRegistrationView::SignalDoRegistration, m_RegistrationWorkStation, &RegistrationWorkStation::SlotDoRegistration);
    connect(this, &ImageRegistrationView::SignalStopRegistration, m_RegistrationWorkStation, &RegistrationWorkStation::SlotStopRegistration, Qt::DirectConnection);
    connect(m_RegistrationWorkStation, &RegistrationWorkStation::SignalRegistrationIterationEnd, this, &ImageRegistrationView::SlotRegistrationIterationEnd, Qt::BlockingQueuedConnection);
    connect(m_RegistrationWorkStation, &RegistrationWorkStation::SignalRegistrationFinished, this, &ImageRegistrationView::SlotRegistrationFinished);
    connect(m_RegistrationWorkStation, &RegistrationWorkStation::SignalReslutImageGenerated, this, &ImageRegistrationView::SlotReslutImageGenerated);


    //init view to fixed image
    mitk::RenderingManager::GetInstance()->InitializeViews(
        m_FixedImageNode->GetData()->GetTimeGeometry(),
        mitk::RenderingManager::REQUEST_UPDATE_ALL, true);

    //start registration
    m_RegistrationThread->start();
    emit SignalDoRegistration(itkFixedImage, itkMovingImage, initMatrix.inverted());
}

void ImageRegistrationView::InitResource(R* pR)
{
    m_pR = pR;
    IQF_MitkDataManager* m_pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);

    mitk::NodePredicateData::Pointer predicate = mitk::NodePredicateData::New(0);
    m_FixedDataStorageComboBox = new QmitkDataStorageComboBox(m_pDataManager->GetDataStorage(), CreatePredicate(4));
    connect(m_FixedDataStorageComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnFixedImageSelectionChanged(const mitk::DataNode *)));

    m_MovingDataStorageComboBox = new QmitkDataStorageComboBox(m_pDataManager->GetDataStorage(), CreatePredicate(4));
    connect(m_MovingDataStorageComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnMovingImageSelectionChanged(const mitk::DataNode *)));

    m_pR->registerCustomWidget("FixedImageComboBox", m_FixedDataStorageComboBox);
    m_pR->registerCustomWidget("MovingImageComboBox", m_MovingDataStorageComboBox);

}

void ImageRegistrationView::ResourceContructed(R* pR)
{

}

void ImageRegistrationView::OnFixedImageSelectionChanged(const mitk::DataNode* node)
{
    m_bInited = false;
    m_FixedImageNode = (mitk::DataNode*)node;
}

void ImageRegistrationView::OnMovingImageSelectionChanged(const mitk::DataNode* node)
{
    m_bInited = false;
    m_MovingImageNode = (mitk::DataNode*)node;
}

void ImageRegistrationView::SlotRegistrationIterationEnd(const itk::Matrix<double, 4, 4>& result)
{
    mitk::DataNode* displayNode = m_pMitkDataManager->GetDataStorage()->GetNamedNode("DisplayMoving");
    if (!displayNode || !displayNode->GetData())
    {
        return;
    }
    QMatrix4x4 m;
    vnl_matrix_fixed<double, 4, 4> matrix = result.GetInverse();
    vtkMatrix4x4* transform = vtkMatrix4x4::New();
    vtkMatrix4x4* rm = vtkMatrix4x4::New();

    MatrixUtil::VnlMatrixToVtkMatrix(matrix, transform);
    MatrixUtil::VtkMatrixToQMatrix(transform, m_registrationMatrix);
    static_cast<ImageNavigationInteractor*>(m_movingImageInteractor.GetPointer())->SetTransformMatrix(m_registrationMatrix);

    vtkMatrix4x4::Multiply4x4(transform, m_originMatrix, rm);      
    if (displayNode != NULL)
    {
        displayNode->GetData()->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(rm);
        displayNode->Modified();
        RequestRenderWindowUpdate();
    }
}

void ImageRegistrationView::InitRegistration(Float3DImageType* itkFixedImage, Float3DImageType* itkMovingImage)
{
    //set resource image invisible
    m_FixedImageNode->SetVisibility(false);
    m_MovingImageNode->SetVisibility(false);

    m_pMitkDataManager->GetDataStorage()->Remove(m_pMitkDataManager->GetDataStorage()->GetNamedNode("DisplayMoving"));
    m_pMitkDataManager->GetDataStorage()->Remove(m_pMitkDataManager->GetDataStorage()->GetNamedNode("DisplayFixed"));
    //display  moving image
    mitk::DataNode::Pointer displayMovingNode = mitk::DataNode::New();
    mitk::Image::Pointer mitkDisplayMovingImage = mitk::Image::New();
    
    mitk::CastToMitkImage<Float3DImageType>(itkMovingImage, mitkDisplayMovingImage);
    displayMovingNode->SetData(mitkDisplayMovingImage);
    displayMovingNode->SetProperty("name", mitk::StringProperty::New("DisplayMoving"));
    displayMovingNode->SetProperty("color", mitk::ColorProperty::New(0.0, 1.0, 0.0));
    displayMovingNode->SetProperty("volumerendering.uselod", mitk::BoolProperty::New(true));
    displayMovingNode->SetOpacity(0.5);
    displayMovingNode->SetVisibility(true);
    displayMovingNode->Update();
    m_movingImageInteractor = displayMovingNode->GetDataInteractor();
    if (m_movingImageInteractor.IsNull())
    {
        m_movingImageInteractor = ImageNavigationInteractor::New();
        m_movingImageInteractor->LoadStateMachine("S:/Codes/MIPF/Plugins/Mipf_Plugin_ImageRegistration/resource/Interactions/ImageNavigation.xml");
        m_movingImageInteractor->SetEventConfig("S:/Codes/MIPF/Plugins/Mipf_Plugin_ImageRegistration/resource/Interactions/ImageNavigationConfig.xml");
        m_movingImageInteractor->SetDataNode(displayMovingNode);
        displayMovingNode->SetDataInteractor(m_movingImageInteractor);
    }

    //display fixed image
    mitk::DataNode::Pointer displayFixedNode = mitk::DataNode::New();
    mitk::Image::Pointer mitkDisplayFixedImage = mitk::Image::New();
    mitk::CastToMitkImage<Float3DImageType>(itkFixedImage, mitkDisplayFixedImage);
    displayFixedNode->SetData(mitkDisplayFixedImage);
    displayFixedNode->SetProperty("name", mitk::StringProperty::New("DisplayFixed"));
    displayFixedNode->SetProperty("color", mitk::ColorProperty::New(1.0, 0.0, 0.0));
    displayFixedNode->SetProperty("volumerendering.uselod", mitk::BoolProperty::New(true));
    displayFixedNode->SetOpacity(0.8);
    displayFixedNode->SetVisibility(true);
    displayFixedNode->Update();

    //copy the moving image origin matrix
    m_originMatrix = vtkMatrix4x4::New();
    m_originMatrix->DeepCopy(mitkDisplayMovingImage->GetGeometry()->GetVtkMatrix());

    m_pMitkDataManager->GetDataStorage()->Add(displayFixedNode);
    m_pMitkDataManager->GetDataStorage()->Add(displayMovingNode);
    

    //init to fixed and moving image
    m_pMitkRenderWindow->ResetCrossHair();

    m_bInited = true;
}

void ImageRegistrationView::EndRegistration()
{
    m_FixedImageNode->SetVisibility(true);
    m_MovingImageNode->SetVisibility(true);
    m_pMitkDataManager->GetDataStorage()->Remove(m_pMitkDataManager->GetDataStorage()->GetNamedNode("DisplayMoving"));
    m_pMitkDataManager->GetDataStorage()->Remove(m_pMitkDataManager->GetDataStorage()->GetNamedNode("DisplayFixed"));

    m_movingImageInteractor = NULL;

    m_bInited = false;
}

void ImageRegistrationView::DisableDefaultInteraction()
{
    // dont deactivate twice, else we will clutter the config list ...
    if (m_ScrollEnabled == false)
        return;

    // As a legacy solution the display interaction of the new interaction framework is disabled here  to avoid conflicts with tools
    // Note: this only affects InteractionEventObservers (formerly known as Listeners) all DataNode specific interaction will still be enabled
    m_DisplayInteractorConfigs.clear();

    auto eventObservers = us::GetModuleContext()->GetServiceReferences<mitk::InteractionEventObserver>();

    for (const auto& eventObserver : eventObservers)
    {
        auto displayInteractor = dynamic_cast<mitk::DisplayInteractor*>(us::GetModuleContext()->GetService<mitk::InteractionEventObserver>(eventObserver));

        if (displayInteractor != nullptr)
        {
            EnableDefaultInteraction();
            // remember the original configuration
            m_DisplayInteractorConfigs.insert(std::make_pair(eventObserver, displayInteractor->GetEventConfig()));
            displayInteractor->SetEventConfig("DisplayConfigMITKLimited.xml");
        }
    }

    m_ScrollEnabled = false;
}
void ImageRegistrationView::EnableDefaultInteraction()
{
    for (const auto& displayInteractorConfig : m_DisplayInteractorConfigs)
    {
        if (displayInteractorConfig.first)
        {
            auto displayInteractor = static_cast<mitk::DisplayInteractor*>(us::GetModuleContext()->GetService<mitk::InteractionEventObserver>(displayInteractorConfig.first));

            if (displayInteractor != nullptr)
            {
                // here the regular configuration is loaded again
                displayInteractor->SetEventConfig(displayInteractorConfig.second);
            }
        }
    }

    m_DisplayInteractorConfigs.clear();
    m_ScrollEnabled = true;
}

void ImageRegistrationView::Reset()
{
    
    //if (m_RegistrationThread&&m_RegistrationThread->isRunning())
    //{
    //    return;
    //}
    mitk::DataNode* displayNode = m_pMitkDataManager->GetDataStorage()->GetNamedNode("DisplayMoving");
    if (displayNode)
    {
        displayNode->GetData()->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(m_originMatrix);
        displayNode->Update();
        mitk::RenderingManager::GetInstance()->InitializeViews(m_FixedImageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
        RequestRenderWindowUpdate();
    }
    m_registrationMatrix.setToIdentity();
    static_cast<ImageNavigationInteractor*>(m_movingImageInteractor.GetPointer())->SetTransformMatrix(m_registrationMatrix);
}

void ImageRegistrationView::Stop()
{
    emit SignalStopRegistration();
}

void ImageRegistrationView::SlotRegistrationFinished(const QfResult& result)
{
    if (result.IsSuccess())
    {
        qDebug() << "Registration Successfully !";
    }
    else
    {
        qDebug() << "Registration Failed !";
    }
    qDebug() << result.GetResultMessage();

    disconnect(this, &ImageRegistrationView::SignalDoRegistration, m_RegistrationWorkStation, &RegistrationWorkStation::SlotDoRegistration);
    disconnect(this, &ImageRegistrationView::SignalStopRegistration, m_RegistrationWorkStation, &RegistrationWorkStation::SlotStopRegistration);
    disconnect(m_RegistrationWorkStation, &RegistrationWorkStation::SignalRegistrationIterationEnd, this, &ImageRegistrationView::SlotRegistrationIterationEnd);
    disconnect(m_RegistrationWorkStation, &RegistrationWorkStation::SignalRegistrationFinished, this, &ImageRegistrationView::SlotRegistrationFinished);
    disconnect(m_RegistrationWorkStation, &RegistrationWorkStation::SignalReslutImageGenerated, this, &ImageRegistrationView::SlotReslutImageGenerated);
    
    m_RegistrationThread->quit();
    qDebug() << "Registration Thread Quit !";
}

void ImageRegistrationView::SlotReslutImageGenerated(const Float3DImagePointerType resultImage)
{
    mitk::DataNode::Pointer resultImageNode = mitk::DataNode::New();
    mitk::Image::Pointer mitkResultImage = mitk::Image::New();

    mitk::CastToMitkImage<Float3DImageType>(resultImage.GetPointer(), mitkResultImage);
    resultImageNode->SetData(mitkResultImage);
    resultImageNode->SetProperty("name", mitk::StringProperty::New("ResultImage"));
    resultImageNode->SetProperty("color", mitk::ColorProperty::New(1.0, 1.0, 0.0));
    resultImageNode->SetOpacity(0.5);
    resultImageNode->SetVisibility(true);
    resultImageNode->Update();

    m_pMitkDataManager->GetDataStorage()->Add(resultImageNode);
    RequestRenderWindowUpdate();
}














