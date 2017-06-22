#include "ImageHoleFillingView.h"
#include "iqf_main.h"
#include "Res/R.h"
#include "ITKImageTypeDef.h"

//Qt
#include <QInputDialog>
#include <QLineEdit>

//qmitk
#include "QmitkDataStorageComboBox.h"

//mitk
#include "mitkImageCast.h"

//itk
#include "itkVotingBinaryIterativeHoleFillingImageFilter.h"


ImageHoleFillingView::ImageHoleFillingView(QF::IQF_Main* pMain) :MitkPluginView(pMain)
{
    m_pMain->Attach(this);
}


ImageHoleFillingView::~ImageHoleFillingView()
{
}

void ImageHoleFillingView::Constructed(R* pR)
{
    m_pImageSelector = (QmitkDataStorageComboBox*)pR->getObjectFromGlobalMap("ImageHoleFilling.ImageSelector");
    if (m_pImageSelector)
    {
        m_pImageSelector->SetPredicate(CreatePredicate(1));
        m_pImageSelector->SetDataStorage(m_pMitkDataManager->GetDataStorage());
    }
    

}

void ImageHoleFillingView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "ImageHoleFilling.FillHoles") == 0)
    {
        QString imageName = QInputDialog::getText(NULL, "Input Result Name", "Image Name:");
        double radiusX,radiusY,radiusZ;
        int numberOfIterations;
        QLineEdit* le = (QLineEdit*)m_pR->getObjectFromGlobalMap("ImageHoleFilling.RadiusX");
        radiusX = le ? le->text().toDouble() : 1.0;
        le = (QLineEdit*)m_pR->getObjectFromGlobalMap("ImageHoleFilling.RadiusY");
        radiusY = le ? le->text().toDouble() : 1.0;
        le = (QLineEdit*)m_pR->getObjectFromGlobalMap("ImageHoleFilling.RadiusZ");
        radiusZ = le ? le->text().toDouble() : 1.0;
        le = (QLineEdit*)m_pR->getObjectFromGlobalMap("ImageHoleFilling.NumberOfIterations");
        numberOfIterations = le ? le->text().toUInt() : 5;

        mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_pImageSelector->GetSelectedNode()->GetData());
        UChar3DImageType::Pointer itkImage = UChar3DImageType::New();
        mitk::CastToItkImage<UChar3DImageType>(mitkImage, itkImage);


        typedef itk::VotingBinaryIterativeHoleFillingImageFilter <UChar3DImageType>
            binaryFillholeImageFilterType;
        binaryFillholeImageFilterType::Pointer fillhole =
            binaryFillholeImageFilterType::New();
        fillhole->SetInput(itkImage);
        UChar3DImageType::SizeType indexRadius;
        indexRadius[0] = radiusX; // radius along x
        indexRadius[1] = radiusY; // radius along y
        indexRadius[2] = radiusZ; // radius along z
        fillhole->SetRadius(indexRadius);
        fillhole->SetBackgroundValue(0);
        fillhole->SetForegroundValue(itk::NumericTraits<UChar3DImageType::PixelType>::max());
        fillhole->SetMajorityThreshold(2);
        fillhole->SetMaximumNumberOfIterations(numberOfIterations);
        try
        {
            fillhole->Update();
        }
        catch (itk::ExceptionObject& ex)
        {
            std::cout << "Filled Small Holes Failed! Exception code: " << std::endl;
            std::cout << ex.what() << std::endl;
            return;
        }

        mitk::DataNode::Pointer filledHoleImageNode = mitk::DataNode::New();
        mitk::Image::Pointer lcMitkImage = mitk::Image::New();
        mitk::CastToMitkImage(fillhole->GetOutput(), lcMitkImage);
        filledHoleImageNode->SetData(lcMitkImage);
        filledHoleImageNode->SetColor(1, 0, 0);
        filledHoleImageNode->SetProperty("binary", mitk::BoolProperty::New(true));
        filledHoleImageNode->SetName(imageName.toStdString());

        m_pMitkDataManager->GetDataStorage()->Add(filledHoleImageNode);

    }
}
