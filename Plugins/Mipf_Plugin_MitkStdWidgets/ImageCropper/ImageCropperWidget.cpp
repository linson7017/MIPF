#include "ImageCropper/ImageCropperWidget.h"
#include <QtWidgets>
#include <QmitkDataStorageComboBox.h>

#include <vtkCommand.h>
#include <vtkCubeSource.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkPlaneWidget.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>

#include <mitkBoundingShapeInteractor.h>
#include <mitkBoundingShapeCropper.h>
#include <mitkDisplayInteractor.h>
#include <mitkIDataStorageService.h>
#include <mitkImageCast.h> 
#include <mitkImageAccessByItk.h>
#include <mitkImageStatisticsHolder.h>
#include <mitkInteractionConst.h>
#include <mitkITKImageImport.h>
#include <mitkLabelSetImage.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkRenderingManager.h>
#include <mitkProperties.h>

#include <itkBoundingBox.h>
#include <itkCommand.h>

ImageCropperWidget::ImageCropperWidget(QF::IQF_Main* pMain)
{
}


ImageCropperWidget::~ImageCropperWidget()
{
}

void ImageCropperWidget::Init(QWidget* parent)
{
    QVBoxLayout* vLayout = new QVBoxLayout;
    setLayout(vLayout);

    {
        boundingShapeSelector->SetDataStorage(this->GetDataStorage());
        boundingShapeSelector->SetPredicate(mitk::NodePredicateAnd::New(
            mitk::TNodePredicateDataType<mitk::GeometryData>::New(),
            mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))));
        m_CroppingObjectNode = boundingShapeSelector->GetSelectedNode();

        buttonCreateNewBoundingBox = new QPushButton("New");
        QHBoxLayout* layout = new QHBoxLayout;
        layout->addWidget(boundingShapeSelector);
        layout->addWidget(buttonCreateNewBoundingBox);
        vLayout->addLayout(layout);
    }

}

void ImageCropperWidget::InitResource(R* pR)
{

}

void ImageCropperWidget::Update(const char* szMessage, int iValue = 0, void* pValue = 0)
{

}