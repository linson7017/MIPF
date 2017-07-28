#include "SkinExtractView.h"

#include "mitkImageCast.h"

#include "itkBinaryThresholdImageFilter.h"

#include "vtkPolyData.h"

#include "ITKImageTypeDef.h"

#include "MitkSegmentation/IQF_MitkSurfaceTool.h"

SkinExtractView::SkinExtractView()
{
}


SkinExtractView::~SkinExtractView()
{
}

void SkinExtractView::CreateView()
{
    m_ui.setupUi(this);

    m_ui.ImageSelector->SetDataStorage(GetDataStorage());
    m_ui.ImageSelector->SetPredicate(CreatePredicate(1));


    m_ui.ThresholdSlider->setDecimals(1);
    m_ui.ThresholdSlider->setSpinBoxAlignment(Qt::AlignVCenter);
    m_ui.ThresholdSlider->setMaximum(1000);
    m_ui.ThresholdSlider->setMinimum(0);
    m_ui.ThresholdSlider->setMaximumValue(5000);
    m_ui.ThresholdSlider->setMinimumValue(-200);

    connect(m_ui.ExtractBtn, SIGNAL(clicked()), this, SLOT(Extract()));
    connect(m_ui.ImageSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(SelectionChanged(const mitk::DataNode *)));
}

void SkinExtractView::Extract()
{
    mitk::DataNode* node = m_ui.ImageSelector->GetSelectedNode();
    IQF_MitkSurfaceTool* pSurfaceTool = (IQF_MitkSurfaceTool*)m_pMain->GetInterfacePtr(QF_MitkSurface_Tool);
    if (node|| pSurfaceTool)
    {
        mitk::Image* imageData = dynamic_cast<mitk::Image*>(node->GetData());
        if (imageData)
        {
            Int3DImageType::Pointer itkImage;
            mitk::CastToItkImage(imageData, itkImage);
            typedef itk::BinaryThresholdImageFilter<Int3DImageType, UShort3DImageType> btImageFilterType;
            btImageFilterType::Pointer btFilter = btImageFilterType::New();
            btFilter->SetInput(itkImage);
            btFilter->SetLowerThreshold(m_ui.ThresholdSlider->minimumValue());
            btFilter->SetUpperThreshold(m_ui.ThresholdSlider->maximumValue());
            btFilter->SetInsideValue(1);
            btFilter->SetOutsideValue(0);
            btFilter->Update();

            mitk::Image::Pointer tempImage;
            mitk::CastToMitkImage(btFilter->GetOutput(), tempImage);
            vtkSmartPointer<vtkPolyData> surfaceData = vtkSmartPointer<vtkPolyData>::New();
            pSurfaceTool->ExtractSurface(tempImage, surfaceData,15,1);

            mitk::Surface::Pointer surface = mitk::Surface::New();
            surface->SetVtkPolyData(surfaceData);
            mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
            surfaceNode->SetData(surface);
            surfaceNode->SetName("skin");
            surfaceNode->SetColor(1.0, 1.0, 0.0);
            surfaceNode->SetOpacity(0.8);
            GetDataStorage()->Add(surfaceNode);
        }
    }
}

void SkinExtractView::SelectionChanged(const mitk::DataNode* node)
{
    if (node)
    {
        mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
        if (image)
        {
            m_ui.ThresholdSlider->setMaximum(image->GetScalarValueMax());
            m_ui.ThresholdSlider->setMinimum(image->GetScalarValueMin());
            m_ui.ThresholdSlider->setMaximumValue(500);
            m_ui.ThresholdSlider->setMinimumValue(-200);
        }
    }
}
