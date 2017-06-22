#include "MaskImageView.h"
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
#include "itkMaskImageFilter.h"

MaskImageView::MaskImageView(QF::IQF_Main* pMain) :MitkPluginView(pMain)
{
    m_pMain->Attach(this);
}


MaskImageView::~MaskImageView()
{
}

void MaskImageView::Constructed(R* pR)
{
    m_pImageSelector = (QmitkDataStorageComboBox*)pR->getObjectFromGlobalMap("MaskImage.ImageSelector");
    m_pMaskSelector = (QmitkDataStorageComboBox*)pR->getObjectFromGlobalMap("MaskImage.MaskSelector");
    
    if (m_pImageSelector&&m_pMaskSelector)
    {
        m_pImageSelector->SetPredicate(CreatePredicate(1));
        m_pImageSelector->SetDataStorage(m_pMitkDataManager->GetDataStorage());
        m_pMaskSelector->SetPredicate(CreatePredicate(1));
        m_pMaskSelector->SetDataStorage(m_pMitkDataManager->GetDataStorage());
    }
}

void MaskImageView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "MaskImage.DoMask") == 0)
    {
        QString imageName = QInputDialog::getText(NULL, "Input Result Name", "Image Name:");

        mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_pImageSelector->GetSelectedNode()->GetData());
        Float3DImageType::Pointer itkImage = Float3DImageType::New();
        mitk::CastToItkImage<Float3DImageType>(mitkImage, itkImage);

        mitk::Image* mitkMask = dynamic_cast<mitk::Image*>(m_pMaskSelector->GetSelectedNode()->GetData());
        UChar3DImageType::Pointer itkMask = UChar3DImageType::New();
        mitk::CastToItkImage<UChar3DImageType>(mitkMask, itkMask);


        typedef itk::MaskImageFilter< Float3DImageType, UChar3DImageType > MaskFilterType;
        MaskFilterType::Pointer maskFilter = MaskFilterType::New();
        maskFilter->SetInput(itkImage);
        maskFilter->SetMaskImage(itkMask);
        maskFilter->Update();

        mitk::DataNode::Pointer maskedImageNode = mitk::DataNode::New();
        mitk::Image::Pointer maskedMitkImage = mitk::Image::New();
        mitk::CastToMitkImage(maskFilter->GetOutput(), maskedMitkImage);
        maskedImageNode->SetData(maskedMitkImage);
        maskedImageNode->SetColor(1, 1, 1);
        maskedImageNode->SetName(imageName.toStdString());

        m_pMitkDataManager->GetDataStorage()->Add(maskedImageNode);
    }
}
