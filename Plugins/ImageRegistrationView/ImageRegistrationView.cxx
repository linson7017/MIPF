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

#include "ITKImageTypeDef.h"

#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkLabelSetImage.h>
#include <mitkContourModel.h>
#include <mitkContourModelSet.h>


#include "Registration_MI.h"

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


ImageRegistrationView::ImageRegistrationView(QF::IQF_Main* pMain):PluginView(pMain), m_FixedImageNode(NULL),m_MovingImageNode(NULL)
{
    m_pMain->Attach(this);
}

ImageRegistrationView::~ImageRegistrationView()
{
  //  delete m_FixedDataStorageComboBox;
 //   delete m_MovingDataStorageComboBox;
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
        Float3DImagePointerType itkFixedImage = Float3DImageType::New();
        mitk::CastToItkImage<Float3DImageType>(dynamic_cast<mitk::Image *>(m_FixedImageNode->GetData()), itkFixedImage);

        Float3DImagePointerType itkMovingImage = Float3DImageType::New();
        mitk::CastToItkImage<Float3DImageType>(dynamic_cast<mitk::Image *>(m_MovingImageNode->GetData()), itkMovingImage);

        Float3DImagePointerType itkResultImage = Float3DImageType::New();;

        RegistrationMI<Float3DImageType,Float3DImageType> rm;
        
        itk::Matrix<double,4,4> m;
        bool alignCenter = false;
        VarientMap vmp = *(VarientMap*)pValue;
        variant v = variant::GetVariant(vmp, "checked");
        if (vmp.size()>0)
        {
            alignCenter = v.getBool();
        }
        m.SetIdentity();
        if (alignCenter)
        {
            mitk::Point3D movingImageCenter = m_MovingImageNode->GetData()->GetGeometry()->GetCenter();
            mitk::Point3D fixedImageCenter = m_FixedImageNode->GetData()->GetGeometry()->GetCenter();
            m[0][3] = fixedImageCenter[0] - movingImageCenter[0];
            m[1][3] = fixedImageCenter[1] - movingImageCenter[1];
            m[2][3] = fixedImageCenter[2] - movingImageCenter[2];
        }
        rm.Start(itkFixedImage.GetPointer(), itkMovingImage.GetPointer(), itkResultImage.GetPointer(), m);

        //add result image to datastorage
        mitk::Image::Pointer result = mitk::Image::New();
        mitk::CastToMitkImage(itkResultImage, result);
        mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
        imageNode->SetData(result);
        std::string name = "result";
        imageNode->SetProperty("name", mitk::StringProperty::New(name));
        imageNode->SetProperty("color", mitk::ColorProperty::New(0.0, 1.0, 0.0));
        imageNode->Update();
        pDataManager->GetDataStorage()->Add(imageNode);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        
    }
    else if (strcmp(szMessage, "ImageRegistration.AlignCenter") == 0)
    {
        
        mitk::Point3D movingImageCenter = m_MovingImageNode->GetData()->GetGeometry()->GetCenter();
        mitk::Point3D fixedImageCenter = m_FixedImageNode->GetData()->GetGeometry()->GetCenter();

        /*mitk::Vector3D translate;
        translate[0] = +fixedImageCenter[0] - movingImageCenter[0];
        translate[1] = +fixedImageCenter[1] - movingImageCenter[1];
        translate[2] = +fixedImageCenter[2] - movingImageCenter[2];
        m_MovingImageNode->GetData()->GetGeometry()->Translate(translate);*/
        
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
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

void ImageRegistrationView::OnFixedImageSelectionChanged(const mitk::DataNode* node)
{
    m_FixedImageNode = (mitk::DataNode*)node;
}

void ImageRegistrationView::OnMovingImageSelectionChanged(const mitk::DataNode* node)
{
    m_MovingImageNode = (mitk::DataNode*)node;
}

