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

MaskImageView::MaskImageView() :MitkPluginView()
{
}


MaskImageView::~MaskImageView()
{
}

void MaskImageView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.ImageSelector->SetPredicate(CreatePredicate(Image));
    m_ui.ImageSelector->SetDataStorage(GetDataStorage());
    m_ui.MaskSelector->SetPredicate(CreatePredicate(Image));
    m_ui.MaskSelector->SetDataStorage(GetDataStorage());
    connect(m_ui.MaskBtn, SIGNAL(clicked()), this, SLOT(Mask()));
}


void MaskImageView::Mask()
{
    QString imageName = QInputDialog::getText(NULL, "Input Result Name", "Image Name:");

    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());
    Float3DImageType::Pointer itkImage;
    mitk::CastToItkImage<Float3DImageType>(mitkImage, itkImage);

    mitk::Image* mitkMask = dynamic_cast<mitk::Image*>(m_ui.MaskSelector->GetSelectedNode()->GetData());
    UChar3DImageType::Pointer itkMask;
    mitk::CastToItkImage<UChar3DImageType>(mitkMask, itkMask);

    //check if the size is different
    bool useSmallestRegion = true;
    itk::ImageRegion<3> imageRegion = itkImage->GetLargestPossibleRegion();
    itk::ImageRegion<3> maskRegion = itkMask->GetLargestPossibleRegion();
    if (imageRegion!=maskRegion)
    {
        if (useSmallestRegion)
        {
            itk::ImageRegion<3> croppedRegion;
            if (imageRegion.Crop(maskRegion))
            {
                croppedRegion = imageRegion;
            }
            else if(maskRegion.Crop(imageRegion))
            {
                croppedRegion = maskRegion;
            }
            else
            {
                return;
            }
        }
        else
        {
            std::cerr << "Image and mask have different size! " << std::endl;
            return;
        }
    }


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
