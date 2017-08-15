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

#include "ITK_Helpers.h"

LargestConnectedComponentView::LargestConnectedComponentView() :MitkPluginView()
{

}

void LargestConnectedComponentView::CreateView()
{
    m_ui.setupUi(this);

    m_ui.ImageSelector->SetDataStorage(GetDataStorage());
    m_ui.ImageSelector->SetPredicate(CreatePredicate(Image));
    
    connect(m_ui.ApplyBtn, SIGNAL(clicked()), this, SLOT(Extract()));
    
}

void LargestConnectedComponentView::Extract()
{
    QString imageName = QInputDialog::getText(NULL, "Input Result Name", "Image Name:");

    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());
    if (!mitkImage)
    {
        return;
    }

    UChar3DImageType::Pointer itkImage = UChar3DImageType::New();
    mitk::CastToItkImage<UChar3DImageType>(mitkImage, itkImage);

    UChar3DImageType* inputImage = itkImage.GetPointer();
    UChar3DImageType* resultImage = itkImage.GetPointer();
    if (m_ui.Operator->currentText().compare("largest") == 0)
    {
        ITKHelpers::ExtractLargestConnected(inputImage, resultImage);
    }
    else 
    {
        itk::ShapeLabelObject<unsigned char, 3>::AttributeType  attributeType = 
            itk::ShapeLabelObject<unsigned char, 3>::GetAttributeFromName(m_ui.LambdaType->currentText().toStdString());
        double lambda = m_ui.LambdaValue->text().toDouble();

        if (m_ui.Operator->currentText().compare(">")==0)
        {
            ITKHelpers::ExtractConnectedLargerThan(inputImage, resultImage, lambda, attributeType);
        }
        else if (m_ui.Operator->currentText().compare("<") == 0)
        {
            ITKHelpers::ExtractConnectedLargerThan(inputImage, resultImage, lambda, attributeType,true);
        }
        else if (m_ui.Operator->currentText().compare("==") == 0)
        {
            ITKHelpers::ExtractConnectedLargerThan(inputImage, resultImage, lambda, attributeType);
            ITKHelpers::ExtractConnectedLargerThan(resultImage, resultImage, lambda, attributeType, true);
        }
    }

    mitk::DataNode::Pointer resultImageNode = mitk::DataNode::New();
    mitk::Image::Pointer resultMitkImage = mitk::Image::New();
    mitk::CastToMitkImage(resultImage, resultMitkImage);
    resultImageNode->SetData(resultMitkImage);
    resultImageNode->SetColor(1, 0, 0);
    resultImageNode->SetOpacity(0.6);
    resultImageNode->SetProperty("binary", mitk::BoolProperty::New(true));
    resultImageNode->SetProperty("volumerendering", mitk::BoolProperty::New(true));
    resultImageNode->SetName(imageName.toStdString());

    m_pMitkDataManager->GetDataStorage()->Add(resultImageNode);

    RequestRenderWindowUpdate();
}
