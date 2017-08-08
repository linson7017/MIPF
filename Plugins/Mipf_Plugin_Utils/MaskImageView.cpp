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
#include "mitkTransferFunction.h"

//itk
#include "itkMaskImageFilter.h"


//vtk
#include <vtkJPEGReader.h>

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
        ////Set Opacity
        //mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();
        //tf->GetScalarOpacityFunction()->AddPoint(-643.78106689453125, 0.0);
        //tf->GetScalarOpacityFunction()->AddPoint(-584.65887451171875, 0.26931655406951904);
        //tf->GetScalarOpacityFunction()->AddPoint(-382.65924072265625, 0.46969130635261536);
        //tf->GetScalarOpacityFunction()->AddPoint(-235.1552734375, 0.0);

        ////Set Color
        //tf->GetColorTransferFunction()->AddRGBPoint(-643.78106689453125, 0.0, 0.60537117719650269, 0.70577555894851685);
        //tf->GetColorTransferFunction()->AddRGBPoint(-584.65887451171875, 0.0, 0.60537117719650269, 0.70577555894851685);
        //tf->GetColorTransferFunction()->AddRGBPoint(-382.65924072265625, 0.0, 0.60537117719650269, 0.70577555894851685);
        //tf->GetColorTransferFunction()->AddRGBPoint(-235.1552734375, 0.0, 0.60537117719650269, 0.70577555894851685);

        ////Set Gradient
        //m_transferFunction->GetGradientOpacityFunction()->AddPoint(560.695, 1);
        //break;


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
