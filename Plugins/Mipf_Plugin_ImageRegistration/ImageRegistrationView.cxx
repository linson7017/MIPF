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


ImageRegistrationView::ImageRegistrationView(QF::IQF_Main* pMain):MitkPluginView(pMain), m_FixedImageNode(NULL),m_MovingImageNode(NULL), m_bInited(false)
{
    m_pMain->Attach(this);
    m_registrationMatrix.setToIdentity();
}

ImageRegistrationView::~ImageRegistrationView()
{
  //  delete m_FixedDataStorageComboBox;
 //   delete m_MovingDataStorageComboBox;
   // delete m_RegistrationWorkStation;
  //  delete m_RegistrationThread;
}


void ImageRegistrationView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "ImageRegistration.Register") == 0)
    {
        IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        //Set Current Image
        if (!m_FixedImageNode||!m_MovingImageNode)
        {
            return;
        }
        //correct the origin of the fixed image
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


        Float3DImagePointerType itkFixedImage = Float3DImageType::New();
        mitk::CastToItkImage<Float3DImageType>(dynamic_cast<mitk::Image *>(m_FixedImageNode->GetData()), itkFixedImage);

        Float3DImagePointerType itkMovingImage = Float3DImageType::New();
        mitk::CastToItkImage<Float3DImageType>(dynamic_cast<mitk::Image *>(m_MovingImageNode->GetData()), itkMovingImage);

       /* float origin[] = { 0.0,0.0,0.0 };
        itkFixedImage->SetOrigin(origin);*/


        
        if (!m_bInited|| !m_pMitkDataManager->GetDataStorage()->GetNamedNode("DisplayMoving")|| !m_pMitkDataManager->GetDataStorage()->GetNamedNode("DisplayFixed"))
        {
            InitRegistration(itkFixedImage, itkMovingImage);
        }
        

       //init matrix
        QMatrix4x4 qm;
        qm.setToIdentity();
        bool alignCenter = false;
        VarientMap vmp = *(VarientMap*)pValue;
        variant v = variant::GetVariant(vmp, "checked");
        if (vmp.size()>0)
        {
            alignCenter = v.getBool();
        }
        QMatrix4x4 initMatrix = m_registrationMatrix;
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
        bool useMultiResolution = false;
        QCheckBox* useMultiResolutionCheckbox = (QCheckBox*)m_pR->getObjectFromGlobalMap("ImageRegistration.UseMultiResolution");
        if (useMultiResolutionCheckbox)
        {
            useMultiResolution = useMultiResolutionCheckbox->isChecked();
        }

        ///////////////Normal Version///////////////////////////
       /* RegistrationMI<Float3DImageType, Float3DImageType> rm;
        rm.Start(itkFixedImage.GetPointer(), itkMovingImage.GetPointer(), itkResultImage.GetPointer(), m);*/

        ///////////MultiThread Version////////////////////////

        m_RegistrationWorkStation = new RegistrationWorkStation;
        m_RegistrationWorkStation->SetRegistrationType(RegistrationWorkStation::MI);
        m_RegistrationWorkStation->SetUseMultiResolution(useMultiResolution);
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
        connect(this, &ImageRegistrationView::SignalStopRegistration, m_RegistrationWorkStation, &RegistrationWorkStation::SlotStopRegistration,Qt::DirectConnection);
        connect(m_RegistrationWorkStation, &RegistrationWorkStation::SignalRegistrationIterationEnd, this, &ImageRegistrationView::SlotRegistrationIterationEnd, Qt::BlockingQueuedConnection);
        connect(m_RegistrationWorkStation, &RegistrationWorkStation::SignalRegistrationFinished, this, &ImageRegistrationView::SlotRegistrationFinished);


        m_RegistrationThread->start();
        emit SignalDoRegistration(itkFixedImage, itkMovingImage, initMatrix.inverted());


        //add result image to datastorage
        /*mitk::Image::Pointer result = mitk::Image::New();
        mitk::CastToMitkImage(itkResultImage, result);
        mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
        imageNode->SetData(result);
        std::string name = "result";
        imageNode->SetProperty("name", mitk::StringProperty::New(name));
        imageNode->SetProperty("color", mitk::ColorProperty::New(0.0, 1.0, 0.0));
        imageNode->Update();
        pDataManager->GetDataStorage()->Add(imageNode);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();*/
        
    }
    else if (strcmp(szMessage, "ImageRegistration.AlignCenter") == 0)
    {
        
        mitk::Point3D movingImageCenter = m_MovingImageNode->GetData()->GetGeometry()->GetCenter();
        mitk::Point3D fixedImageCenter = m_FixedImageNode->GetData()->GetGeometry()->GetCenter();

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
    m_leMoveStep = (QLineEdit*)pR->getObjectFromGlobalMap("ImageRegistration.MoveStep");
    m_btnMoveXAdd = (QPushButton*)pR->getObjectFromGlobalMap("ImageRegistration.Btn.MoveXAdd");
    m_btnMoveYAdd = (QPushButton*)pR->getObjectFromGlobalMap("ImageRegistration.Btn.MoveYAdd");
    m_btnMoveZAdd = (QPushButton*)pR->getObjectFromGlobalMap("ImageRegistration.Btn.MoveZAdd");
    m_btnMoveXSub = (QPushButton*)pR->getObjectFromGlobalMap("ImageRegistration.Btn.MoveXSub");
    m_btnMoveYSub = (QPushButton*)pR->getObjectFromGlobalMap("ImageRegistration.Btn.MoveYSub");
    m_btnMoveZSub = (QPushButton*)pR->getObjectFromGlobalMap("ImageRegistration.Btn.MoveZSub");

    m_leRotateStep = (QLineEdit*)pR->getObjectFromGlobalMap("ImageRegistration.RotateStep");
    m_btnRotateXAdd = (QPushButton*)pR->getObjectFromGlobalMap("ImageRegistration.Btn.RotateXAdd");
    m_btnRotateYAdd = (QPushButton*)pR->getObjectFromGlobalMap("ImageRegistration.Btn.RotateYAdd");
    m_btnRotateZAdd = (QPushButton*)pR->getObjectFromGlobalMap("ImageRegistration.Btn.RotateZAdd");
    m_btnRotateXSub = (QPushButton*)pR->getObjectFromGlobalMap("ImageRegistration.Btn.RotateXSub");
    m_btnRotateYSub = (QPushButton*)pR->getObjectFromGlobalMap("ImageRegistration.Btn.RotateYSub");
    m_btnRotateZSub = (QPushButton*)pR->getObjectFromGlobalMap("ImageRegistration.Btn.RotateZSub");
    if (m_btnMoveXAdd&&m_btnMoveYAdd&&m_btnMoveZAdd
        &&m_btnMoveXSub&&m_btnMoveYSub&&m_btnMoveZSub
        &&m_btnRotateXAdd&&m_btnRotateYAdd&&m_btnRotateZAdd
        &&m_btnRotateXSub&&m_btnRotateYSub&&m_btnRotateZSub
        &&m_leMoveStep&&m_leRotateStep)

    {
        connect(m_btnMoveXAdd, SIGNAL(pressed()), this, SLOT(SlotMoveXAdd()));
        connect(m_btnMoveYAdd, SIGNAL(pressed()), this, SLOT(SlotMoveYAdd()));
        connect(m_btnMoveZAdd, SIGNAL(pressed()), this, SLOT(SlotMoveZAdd()));
        connect(m_btnMoveXSub, SIGNAL(pressed()), this, SLOT(SlotMoveXSub()));
        connect(m_btnMoveYSub, SIGNAL(pressed()), this, SLOT(SlotMoveYSub()));
        connect(m_btnMoveZSub, SIGNAL(pressed()), this, SLOT(SlotMoveZSub()));

        connect(m_btnRotateXAdd, SIGNAL(pressed()), this, SLOT(SlotRotateXAdd()));
        connect(m_btnRotateYAdd, SIGNAL(pressed()), this, SLOT(SlotRotateYAdd()));
        connect(m_btnRotateZAdd, SIGNAL(pressed()), this, SLOT(SlotRotateZAdd()));
        connect(m_btnRotateXSub, SIGNAL(pressed()), this, SLOT(SlotRotateXSub()));
        connect(m_btnRotateYSub, SIGNAL(pressed()), this, SLOT(SlotRotateYSub()));
        connect(m_btnRotateZSub, SIGNAL(pressed()), this, SLOT(SlotRotateZSub()));
    }
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
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            transform->SetElement(i, j, matrix(i, j));
            m_registrationMatrix(i, j) = matrix(i,j);
        }
    }

    vtkMatrix4x4::Multiply4x4(transform, m_originMatrix, rm);      
    if (displayNode != NULL)
    {
        displayNode->GetData()->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(rm);
        displayNode->Modified();
        RequestRenderWindowUpdate();
    }
}

void ImageRegistrationView::RefreshMovingImage(/*QMatrix4x4& matrix*/)
{
    mitk::DataNode* displayNode = m_pMitkDataManager->GetDataStorage()->GetNamedNode("DisplayMoving");
    if (!displayNode || !displayNode->GetData())
    {
        return;
    }
   // m_registrationMatrix = matrix*m_registrationMatrix;
    vtkMatrix4x4* transform = vtkMatrix4x4::New();
    vtkMatrix4x4* rm = vtkMatrix4x4::New();
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            transform->SetElement(i, j, m_registrationMatrix(i, j));
        }
    }
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
   // displayMovingNode->SetIntProperty("layer", 100);
    displayMovingNode->Update();

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
    
    

    //init to fixed image
    mitk::RenderingManager::GetInstance()->InitializeViews(
        m_FixedImageNode->GetData()->GetTimeGeometry(),
        mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    m_bInited = true;
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
}

void ImageRegistrationView::Stop()
{
    emit SignalStopRegistration();
}

void ImageRegistrationView::SlotRegistrationFinished()
{

    disconnect(this, &ImageRegistrationView::SignalDoRegistration, m_RegistrationWorkStation, &RegistrationWorkStation::SlotDoRegistration);
    disconnect(this, &ImageRegistrationView::SignalStopRegistration, m_RegistrationWorkStation, &RegistrationWorkStation::SlotStopRegistration);
    disconnect(m_RegistrationWorkStation, &RegistrationWorkStation::SignalRegistrationIterationEnd, this, &ImageRegistrationView::SlotRegistrationIterationEnd);
    disconnect(m_RegistrationWorkStation, &RegistrationWorkStation::SignalRegistrationFinished, this, &ImageRegistrationView::SlotRegistrationFinished);
    
    m_RegistrationThread->quit();
  //  m_pMitkRenderWindow->SetCrossHairVisibility(true);
    qDebug() << "Registration Finished And Quit !";
}


void ImageRegistrationView::TranslateMovingImage(const QVector3D& translate)
{
    QMatrix4x4 invertMatrix = m_registrationMatrix;
    invertMatrix = invertMatrix.inverted();
    invertMatrix(0, 3) = 0.0;
    invertMatrix(1, 3) = 0.0;
    invertMatrix(2, 3) = 0.0;
    invertMatrix(3, 3) = 1.0;
    invertMatrix(3, 0) = 0.0;
    invertMatrix(3, 1) = 0.0;
    invertMatrix(3, 2) = 0.0;
    QVector3D delta = invertMatrix*translate;
    m_registrationMatrix.translate(delta);
    RefreshMovingImage();
}

void ImageRegistrationView::RotateMovingImage(double angle, const QVector3D& normal)
{
    mitk::Point3D center = m_MovingImageNode->GetData()->GetGeometry()->GetCenter();
    m_registrationMatrix.translate(center[0], center[1], center[2]);
    m_registrationMatrix.rotate(angle, normal);
    m_registrationMatrix.translate(-center[0], -center[1], -center[2]);
    RefreshMovingImage();
}

void ImageRegistrationView::SlotMoveXAdd()
{
    TranslateMovingImage(QVector3D(m_leMoveStep->text().toDouble(), 0, 0));
}

void ImageRegistrationView::SlotMoveYAdd()
{
    TranslateMovingImage(QVector3D(0, m_leMoveStep->text().toDouble(), 0));
}

void ImageRegistrationView::SlotMoveZAdd()
{
    TranslateMovingImage(QVector3D(0, 0, m_leMoveStep->text().toDouble()));
}

void ImageRegistrationView::SlotMoveXSub()
{
    TranslateMovingImage(QVector3D(-m_leMoveStep->text().toDouble(), 0, 0));

}

void ImageRegistrationView::SlotMoveYSub()
{
    TranslateMovingImage(QVector3D(0, -m_leMoveStep->text().toDouble(), 0.0));
}

void ImageRegistrationView::SlotMoveZSub()
{
    TranslateMovingImage(QVector3D(0, 0, -m_leMoveStep->text().toDouble()));
}


void ImageRegistrationView::SlotRotateXAdd()
{
    RotateMovingImage(m_leRotateStep->text().toDouble(), QVector3D(1.0, 0, 0.0));
}


void ImageRegistrationView::SlotRotateYAdd()
{
    RotateMovingImage(m_leRotateStep->text().toDouble(), QVector3D(0, 1.0, 0.0));
}


void ImageRegistrationView::SlotRotateZAdd()
{
    RotateMovingImage(m_leRotateStep->text().toDouble(), QVector3D(0.0, 0, 1.0));
}


void ImageRegistrationView::SlotRotateXSub()
{
    RotateMovingImage(-m_leRotateStep->text().toDouble(), QVector3D(1.0, 0, 0.0));
}


void ImageRegistrationView::SlotRotateYSub()
{
    RotateMovingImage(-m_leRotateStep->text().toDouble(), QVector3D(0, 1.0, 0.0));
}


void ImageRegistrationView::SlotRotateZSub()
{
    RotateMovingImage(-m_leRotateStep->text().toDouble(), QVector3D(0, 0, 1.0));
}













