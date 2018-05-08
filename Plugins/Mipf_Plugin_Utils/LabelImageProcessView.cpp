#include "LabelImageProcessView.h"
#include "iqf_main.h"
#include "Res/R.h"
#include "ITKImageTypeDef.h"
//Qt
#include <QInputDialog>
#include <QCheckBox>

//qmitk
#include "QmitkDataStorageComboBox.h"

//mitk
#include "mitkImageCast.h"

#include "ITK_Helpers.h"
#include "itkGaussianInterpolateImageFunction.h"

#include <vtkProperty.h>

#include "MitkSegmentation/IQF_MitkSurfaceTool.h"

using namespace mitk;


LabelImageProcessView::LabelImageProcessView() : MitkPluginView()
{
}


LabelImageProcessView::~LabelImageProcessView()
{
}

void LabelImageProcessView::CreateView()
{
    m_ui.setupUi(this);

    m_ui.ImageSelector->SetPredicate(CreatePredicate(Image));
    m_ui.ImageSelector->SetDataStorage(GetDataStorage());

    m_ui.ConnectedInformation->setHeaderLabels(QStringList() << "Name" << "Value");

    connect(m_ui.ResampleBtn, SIGNAL(clicked()), this, SLOT(Resample()));
    connect(m_ui.ImageSelector, &QmitkDataStorageComboBox::OnSelectionChanged, this, &LabelImageProcessView::UpdateLabelConnectedComponentInformation);

}

void LabelImageProcessView::Resample()
{
    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());

    UChar3DImageType::Pointer itkImage;
    mitk::CastToItkImage<UChar3DImageType>(mitkImage, itkImage);

    UChar3DImageType::Pointer resultItkImage = UChar3DImageType::New();
    ITKHelpers::ResampleLabelImage<UChar3DImageType, UChar3DImageType, itk::GaussianInterpolateImageFunction>(
        itkImage.GetPointer(), resultItkImage.GetPointer(), 1.0/m_ui.ResampleRateLE->text().toDouble());

    ImportITKImage(resultItkImage.GetPointer(), "Resampled Image", m_ui.ImageSelector->GetSelectedNode());
}

void LabelImageProcessView::UpdateLabelConnectedComponentInformation(const mitk::DataNode * node)
{
    if (!node)
    {
        return;
    }
    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(node->GetData());
    UChar3DImageType::Pointer itkImage;
    mitk::CastToItkImage<UChar3DImageType>(mitkImage, itkImage);

    ITKHelpers::ExtractLargestConnected(itkImage.GetPointer(), itkImage.GetPointer());
    typedef itk::BinaryImageToShapeLabelMapFilter<UChar3DImageType> BinaryImageToShapeLabelMapFilterType;
    BinaryImageToShapeLabelMapFilterType::Pointer binaryImageToShapeLabelMapFilter = BinaryImageToShapeLabelMapFilterType::New();
    binaryImageToShapeLabelMapFilter->SetFullyConnected(true);
    binaryImageToShapeLabelMapFilter->SetInputForegroundValue(1);
    binaryImageToShapeLabelMapFilter->SetInput(itkImage);
    binaryImageToShapeLabelMapFilter->Update();


    BinaryImageToShapeLabelMapFilterType::OutputImageType::LabelObjectType* labelObject =
        binaryImageToShapeLabelMapFilter->GetOutput()->GetNthLabelObject(0);


    QList<QTreeWidgetItem*> items;
    QTreeWidgetItem* item;

    item = new QTreeWidgetItem(QStringList() << "Number of pixel" << QString("%1").arg(labelObject->GetNumberOfPixels()));
    items.append(item);

    item = new QTreeWidgetItem(QStringList() << "PhysicalSize" << QString("%1").arg(labelObject->GetPhysicalSize()));
    items.append(item);

    itk::Point<double> centroid = labelObject->GetCentroid();
    item = new QTreeWidgetItem(QStringList() << "Centroid" << QString("%1,%2,%3").arg(centroid.GetElement(0)).arg(centroid.GetElement(1)).arg(centroid.GetElement(2)));
    items.append(item);

    item = new QTreeWidgetItem(QStringList() << "Number Of Pixels On Border" << QString("%1").arg(labelObject->GetNumberOfPixelsOnBorder()));
    items.append(item);

    item = new QTreeWidgetItem(QStringList() << "Perimeter On Border" << QString("%1").arg(labelObject->GetPerimeterOnBorder()));
    items.append(item);

    item = new QTreeWidgetItem(QStringList() << "Feret Diameter" << QString("%1").arg(labelObject->GetFeretDiameter()));
    items.append(item);

    item = new QTreeWidgetItem(QStringList() << "Elongation" << QString("%1").arg(labelObject->GetElongation()));
    items.append(item);

    item = new QTreeWidgetItem(QStringList() << "Perimeter" << QString("%1").arg(labelObject->GetPerimeter()));
    items.append(item);

    item = new QTreeWidgetItem(QStringList() << "Roundness" << QString("%1").arg(labelObject->GetRoundness()));
    items.append(item);

    item = new QTreeWidgetItem(QStringList() << "Equivalent Spherical Radius" << QString("%1").arg(labelObject->GetEquivalentSphericalRadius()));
    items.append(item);

    item = new QTreeWidgetItem(QStringList() << "Equivalent Spherical Perimeter" << QString("%1").arg(labelObject->GetEquivalentSphericalPerimeter()));
    items.append(item);

    item = new QTreeWidgetItem(QStringList() << "Flatness" << QString("%1").arg(labelObject->GetFlatness()));
    items.append(item);

    item = new QTreeWidgetItem(QStringList() << "Perimeter On Border Ratio" << QString("%1").arg(labelObject->GetPerimeterOnBorderRatio()));
    items.append(item);

    m_ui.ConnectedInformation->addTopLevelItems(items);
}
