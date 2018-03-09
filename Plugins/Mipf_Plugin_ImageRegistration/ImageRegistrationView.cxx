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
#include "QmitkPointListWidget.h"

#include "usGetModuleContext.h"
#include "usModuleContext.h"
#include <usModuleInitialization.h>
#include "Interactions/ImageInteractor.h"


ImageRegistrationView::ImageRegistrationView():MitkPluginView(), m_FixedImageNode(NULL),m_MovingImageNode(NULL), m_bInited(false),
m_EventConfig("DisplayConfigMITK.xml"), m_ScrollEnabled(true)
{

    m_registrationMatrix = vtkMatrix4x4::New();
    m_registrationMatrix->Identity();

    qRegisterMetaType<QfResult>("QfResult");
}

ImageRegistrationView::~ImageRegistrationView()
{

}

void ImageRegistrationView::CreateView()
{
    m_pMain->Attach(this);

    m_ui.setupUi(this);

    m_ui.FixedImageComboBox->SetDataStorage(GetDataStorage());
    m_ui.MovingImageComboBox->SetDataStorage(GetDataStorage());
    m_ui.FixedImageComboBox->SetPredicate(CreatePredicate(MitkPluginView::Image));
    m_ui.MovingImageComboBox->SetPredicate(CreatePredicate(MitkPluginView::Image));

    connect(m_ui.FixedImageComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnFixedImageSelectionChanged(const mitk::DataNode *)));
    connect(m_ui.MovingImageComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnMovingImageSelectionChanged(const mitk::DataNode *)));

    connect(m_ui.InitRegisterBtn, SIGNAL(clicked()), this, SLOT(InitRegistration()));
    connect(m_ui.RegisterBtn, SIGNAL(clicked()), this, SLOT(DoRegistration()));
    connect(m_ui.StopRegisterBtn, SIGNAL(clicked()), this, SLOT(Stop()));
    connect(m_ui.ResetRegisterBtn, SIGNAL(clicked()), this, SLOT(Reset()));
    connect(m_ui.EndRegisterBtn, SIGNAL(clicked()), this, SLOT(EndRegistration()));

}


void ImageRegistrationView::Update(const char* szMessage, int iValue, void* pValue)
{
    
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
    if (m_ui.CorrectFixedCenterCB->checkState()==Qt::Checked)
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

    if (!m_bInited || !GetDataStorage()->GetNamedNode("DisplayMoving") || !GetDataStorage()->GetNamedNode("DisplayFixed"))
    {
        InitRegistration();
    }


    //init matrix
    QMatrix4x4 qm;
    qm.setToIdentity();
    //QMatrix4x4 initMatrix = m_registrationMatrix;
    auto initMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    initMatrix->DeepCopy(static_cast<ImageInteractor*>(m_movingImageInteractor.GetPointer())->GetTransform());
    if (m_ui.AlignCenterCB->isChecked())
    {
        initMatrix->Identity();
        mitk::Point3D movingImageCenter = m_MovingImageNode->GetData()->GetGeometry()->GetCenter();
        mitk::Point3D fixedImageCenter = m_FixedImageNode->GetData()->GetGeometry()->GetCenter();
        auto translate = vtkSmartPointer<vtkTransform>::New();
        translate->Translate(fixedImageCenter[0] - movingImageCenter[0],
            fixedImageCenter[1] - movingImageCenter[1],
            fixedImageCenter[2] - movingImageCenter[2]);
        vtkMatrix4x4::Multiply4x4(translate->GetMatrix(), initMatrix, initMatrix);
    }

    ///////////////Normal Version///////////////////////////
    /* RegistrationMI<Float3DImageType, Float3DImageType> rm;
    rm.Start(itkFixedImage.GetPointer(), itkMovingImage.GetPointer(), itkResultImage.GetPointer(), m);*/

    ///////////MultiThread Version////////////////////////
    m_RegistrationWorkStation = new RegistrationWorkStation;
    if (m_ui.DeformableRegistrationCB->isChecked())
    {
        m_RegistrationWorkStation->SetRegistrationType(RegistrationWorkStation::SFD);
    }
    else
    {
        m_RegistrationWorkStation->SetRegistrationType(RegistrationWorkStation::MI);
    } 
    m_RegistrationWorkStation->SetBeginWithTranslation(m_ui.BeginWithTranslationCB->isChecked());
    m_RegistrationWorkStation->SetOnlyTranslation(m_ui.OnlyTranslationCB->isChecked());


    m_RegistrationWorkStation->SetUseMultiResolution(m_ui.UseMultiResolutionCB->isChecked());
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
    auto tempMatrix = vtkSmartPointer<vtkMatrix4x4>::New();

    initMatrix->Invert();
    emit SignalDoRegistration(itkFixedImage, itkMovingImage, initMatrix.Get());
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
    
    mitk::DataNode* displayNode = GetDataStorage()->GetNamedNode("DisplayMoving");
    if (!displayNode || !displayNode->GetData())
    {
        return;
    }
    QMatrix4x4 m;
    vnl_matrix_fixed<double, 4, 4> matrix = result.GetInverse();
    vtkMatrix4x4* transform = vtkMatrix4x4::New();
    vtkMatrix4x4* rm = vtkMatrix4x4::New();

    MatrixUtil::VnlMatrixToVtkMatrix(matrix, m_registrationMatrix);
    static_cast<ImageInteractor*>(m_movingImageInteractor.GetPointer())->SetTransform(m_registrationMatrix);

    vtkMatrix4x4::Multiply4x4(m_registrationMatrix, m_originMatrix, rm);
    UpdataRegistrationText(*m_registrationMatrix);
    if (displayNode != NULL)
    {
        displayNode->GetData()->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(rm);
        displayNode->Modified();
        RequestRenderWindowUpdate();
    }
}

void ImageRegistrationView::UpdataRegistrationText(const vtkMatrix4x4& matrix)
{
    QString matrixStr = QString("%1 %2 %3 %4\n%5 %6 %7 %8\n%9 %10 %11 %12\n%13 %14 %15 %16")
        .arg(matrix.GetElement(0,0)).arg(matrix.GetElement(0,1)).arg(matrix.GetElement(0,2)).arg(matrix.GetElement(0,3))
        .arg(matrix.GetElement(1,0)).arg(matrix.GetElement(1, 1)).arg(matrix.GetElement(1, 2)).arg(matrix.GetElement(1, 3))
        .arg(matrix.GetElement(2, 0)).arg(matrix.GetElement(2, 1)).arg(matrix.GetElement(2, 2)).arg(matrix.GetElement(2, 3))
        .arg(matrix.GetElement(3, 0)).arg(matrix.GetElement(3, 1)).arg(matrix.GetElement(3, 2)).arg(matrix.GetElement(3, 3));
    m_ui.RegistrationMatrixTE->setPlainText(matrixStr);
}

void ImageRegistrationView::InitRegistration()
{
    if (m_bInited)
    {
        return;
    }

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

    //set resource image invisible
    m_FixedImageNode->SetVisibility(false);
    m_MovingImageNode->SetVisibility(false);

    GetDataStorage()->Remove(GetDataStorage()->GetNamedNode("DisplayMoving"));
    GetDataStorage()->Remove(GetDataStorage()->GetNamedNode("DisplayFixed"));
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
        std::string configpath = m_pMain->GetConfigPath();
        configpath.append("/mitk/Interactions/");
        m_movingImageInteractor = ImageInteractor::New();
        m_movingImageInteractor->LoadStateMachine(configpath + "ImageNavigation.xml");
        m_movingImageInteractor->SetEventConfig(configpath + "ImageNavigationConfig.xml");
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

    GetDataStorage()->Add(displayFixedNode);
    GetDataStorage()->Add(displayMovingNode);
    

    //init to fixed and moving image
    m_pMitkRenderWindow->ResetCrossHair();

    m_bInited = true;
}

void ImageRegistrationView::EndRegistration()
{
    m_FixedImageNode->SetVisibility(true);
    m_MovingImageNode->SetVisibility(true);
    GetDataStorage()->Remove(GetDataStorage()->GetNamedNode("DisplayMoving"));
    GetDataStorage()->Remove(GetDataStorage()->GetNamedNode("DisplayFixed"));

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
    mitk::DataNode* displayNode = GetDataStorage()->GetNamedNode("DisplayMoving");
    if (displayNode)
    {
        displayNode->GetData()->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(m_originMatrix);
        displayNode->Update();
        mitk::RenderingManager::GetInstance()->InitializeViews(m_FixedImageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
        RequestRenderWindowUpdate();
    }
    m_registrationMatrix->Identity();
    static_cast<ImageInteractor*>(m_movingImageInteractor.GetPointer())->SetTransform(m_registrationMatrix);
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

    GetDataStorage()->Add(resultImageNode);
    RequestRenderWindowUpdate();
}














