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
    m_ui.ImageSelector->SetPredicate(CreatePredicate(1));
    m_ui.ImageSelector->SetDataStorage(GetDataStorage());
    m_ui.MaskSelector->SetPredicate(CreatePredicate(1));
    m_ui.MaskSelector->SetDataStorage(GetDataStorage());

    connect(m_ui.MaskBtn, SIGNAL(clicked()), this, SLOT(Mask()));
}


void MaskImageView::Mask()
{
    QString imageName = QInputDialog::getText(NULL, "Input Result Name", "Image Name:");

    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());
    Float3DImageType::Pointer itkImage = Float3DImageType::New();
    mitk::CastToItkImage<Float3DImageType>(mitkImage, itkImage);

    mitk::Image* mitkMask = dynamic_cast<mitk::Image*>(m_ui.MaskSelector->GetSelectedNode()->GetData());
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
