#include "LargestConnectedComponentView.h"
#include "iqf_main.h"
#include "Res/R.h"
#include "ITKImageTypeDef.h"

//Qt
#include <QInputDialog>

//qmitk
#include "QmitkDataStorageComboBox.h"

//mitk
#include "mitkImageCast.h"
#include "mitkLabelSetImage.h"
//itk
#include "itkConnectedComponentImageFilter.h"
#include "itkLabelShapeKeepNObjectsImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

mitk::NodePredicateBase::Pointer CreateUserPredicate(int type)
{
    auto imageType = mitk::TNodePredicateDataType<mitk::Image>::New();
    auto labelSetImageType = mitk::TNodePredicateDataType<mitk::LabelSetImage>::New();
    auto surfaceType = mitk::TNodePredicateDataType<mitk::Surface>::New();
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

    default:
        assert(false && "Unknown predefined predicate!");
        return nullptr;
    }

    return mitk::NodePredicateAnd::New(returnValue, nonHelperObject).GetPointer();
}

LargestConnectedComponentView::LargestConnectedComponentView(QF::IQF_Main* pMain) :MitkPluginView(pMain)
{
    m_pMain->Attach(this);
}

void LargestConnectedComponentView::Constructed(R* pR)
{
    m_pImageSelector = (QmitkDataStorageComboBox*)pR->getObjectFromGlobalMap("LargestConnectedComponentWidget.ImageSelector");
    if (m_pImageSelector)
    {
        m_pImageSelector->SetPredicate(CreatePredicate(1));
        m_pImageSelector->SetDataStorage(m_pMitkDataManager->GetDataStorage());
    }
    
}

void LargestConnectedComponentView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "LargestConnectedComponentWidget.Extract") == 0)
    {
        //do what you want for the message
        //get the result image name
        QString imageName = QInputDialog::getText(NULL, "Input Result Name", "Image Name:");

        mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_pImageSelector->GetSelectedNode()->GetData());
        UInt3DImageType::Pointer itkImage = UInt3DImageType::New();
        mitk::CastToItkImage<UInt3DImageType>(mitkImage, itkImage);

        typedef itk::ConnectedComponentImageFilter <UInt3DImageType, UInt3DImageType >
            ConnectedComponentImageFilterType;
        ConnectedComponentImageFilterType::Pointer connected = ConnectedComponentImageFilterType::New();
        connected->SetInput(itkImage);
        connected->Update();

        std::cout << "Number of objects: " << connected->GetObjectCount() << std::endl;

        typedef itk::LabelShapeKeepNObjectsImageFilter< UInt3DImageType > LabelShapeKeepNObjectsImageFilterType;
        LabelShapeKeepNObjectsImageFilterType::Pointer labelShapeKeepNObjectsImageFilter = LabelShapeKeepNObjectsImageFilterType::New();
        labelShapeKeepNObjectsImageFilter->SetInput(connected->GetOutput());
        labelShapeKeepNObjectsImageFilter->SetBackgroundValue(0);
        labelShapeKeepNObjectsImageFilter->SetNumberOfObjects(1);
        labelShapeKeepNObjectsImageFilter->SetAttribute(LabelShapeKeepNObjectsImageFilterType::LabelObjectType::NUMBER_OF_PIXELS);

        typedef itk::RescaleIntensityImageFilter< UInt3DImageType, UChar3DImageType > RescaleFilterType;
        RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
        rescaleFilter->SetOutputMinimum(0);
        rescaleFilter->SetOutputMaximum(itk::NumericTraits<UChar3DImageType::PixelType>::max());
        rescaleFilter->SetInput(labelShapeKeepNObjectsImageFilter->GetOutput());
        rescaleFilter->Update();

        mitk::DataNode::Pointer largestConnectedImageNode = mitk::DataNode::New();
        mitk::Image::Pointer lcMitkImage = mitk::Image::New();
        mitk::CastToMitkImage(rescaleFilter->GetOutput(), lcMitkImage);
        largestConnectedImageNode->SetData(lcMitkImage);
        largestConnectedImageNode->SetColor(1, 0, 0);
        largestConnectedImageNode->SetOpacity(0.6);
        largestConnectedImageNode->SetProperty("binary", mitk::BoolProperty::New(true));
        largestConnectedImageNode->SetName(imageName.toStdString());

        m_pMitkDataManager->GetDataStorage()->Add(largestConnectedImageNode);

    }
}