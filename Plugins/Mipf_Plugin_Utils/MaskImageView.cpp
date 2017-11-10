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
#include "mitkImageVtkMapper2D.h"
#include "QmitkStdMultiWidget.h"
#include "mitkExtractSliceFilter.h"

//itk
#include "itkMaskImageFilter.h"
#include "itkExtractImageFilter.h"



//vtk
#include <vtkJPEGReader.h>
#include <vtkImageReslice.h>
#include "MitkMain/IQF_MitkRenderWindow.h"

MaskImageView::MaskImageView() :MitkPluginView()
{
}


MaskImageView::~MaskImageView()
{
}

void MaskImageView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.ImageSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Image>::New());
    m_ui.ImageSelector->SetDataStorage(GetDataStorage());
    m_ui.MaskSelector->SetPredicate(mitk::NodePredicateAnd::New(
        mitk::TNodePredicateDataType < mitk::Image>::New(),
        mitk::NodePredicateProperty::New("binary",mitk::BoolProperty::New(true))
    ));
    m_ui.MaskSelector->SetDataStorage(GetDataStorage());
    connect(m_ui.MaskBtn, SIGNAL(clicked()), this, SLOT(Mask()));
}


void MaskImageView::Mask()
{
    QString imageName = QInputDialog::getText(NULL, "Input Result Name", "Image Name:");

    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());
    mitk::Image* mitkMask = dynamic_cast<mitk::Image*>(m_ui.MaskSelector->GetSelectedNode()->GetData());

    vtkImageData* image = mitkImage->GetVtkImageData();
    vtkImageData* mask = mitkMask->GetVtkImageData();

    int outputExtent[6];
    GetMinExtent(image->GetExtent(), mask->GetExtent(),outputExtent);

    auto outputImage = vtkSmartPointer<vtkImageData>::New();
    outputImage->DeepCopy(image);
    float backgroundValue = m_ui.BackgroundValueLE->text().toFloat();
    if (m_ui.UseMinimumValueCB->isChecked())
    {
        backgroundValue = image->GetScalarTypeMin();
    }
    
    for (int i = outputExtent[0]; i <= outputExtent[1]; i++)
    {
        for (int j = outputExtent[2]; j <= outputExtent[3]; j++)
        {
            for (int k = outputExtent[4]; k <= outputExtent[5]; k++)
            {
                if (static_cast<unsigned short>(mask->GetScalarComponentAsFloat(i, j, k, 0))==0)
                {
                    outputImage->SetScalarComponentFromDouble(i, j, k, 0, backgroundValue);
                }             
            }
        }
    }

    mitk::DataNode::Pointer maskedImageNode = mitk::DataNode::New();
    mitk::Image::Pointer maskedMitkImage = mitk::Image::New();
    maskedMitkImage->Initialize(mitkImage);
    maskedMitkImage->SetImportVolume(outputImage->GetScalarPointer());
    maskedImageNode->SetData(maskedMitkImage);
    maskedImageNode->SetColor(1, 1, 1);
    maskedImageNode->SetName(imageName.toStdString());

    m_pMitkDataManager->GetDataStorage()->Add(maskedImageNode);
}


void MaskImageView::GetMinExtent(int* e1, int* e2, int* out)
{
    out[0] = e1[0] > e2[0] ? e1[0] : e2[0];
    out[1] = e1[1] < e2[1] ? e1[1] : e2[1];
    out[2] = e1[2] > e2[2] ? e1[2] : e2[2];
    out[3] = e1[3] < e2[3] ? e1[3] : e2[3];
    out[4] = e1[4] > e2[4] ? e1[4] : e2[4];
    out[5] = e1[5] < e2[5] ? e1[5] : e2[5];
}