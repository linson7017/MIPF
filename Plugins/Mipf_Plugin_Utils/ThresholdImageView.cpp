#include "ThresholdImageView.h"


#include "ITKImageTypeDef.h"

#include "itkBinaryThresholdImageFilter.h"

#include "mitkImageCast.h"



ThresholdImageView::ThresholdImageView()
{
    m_pResultNode = mitk::DataNode::New();
    m_pResultNode->SetBoolProperty("helper object", true);
    m_pResultNode->SetVisibility(false);

}


ThresholdImageView::~ThresholdImageView()
{

}

void ThresholdImageView::CreateView()
{
    m_ui.setupUi(this);
    GetDataStorage()->Add(m_pResultNode);
    m_pMain->Attach(this);

    m_ui.ImageSelector->SetDataStorage(GetDataStorage());
    m_ui.ImageSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Image>::New());
    connect(m_ui.ImageSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(SelectionChanged(const mitk::DataNode*)));

    m_ui.ThresholdSlider->setTracking(false);
    m_ui.ThresholdSlider->setDecimals(1);
    m_ui.ThresholdSlider->setSpinBoxAlignment(Qt::AlignVCenter);
    m_ui.ThresholdSlider->setEnabled(false);
    connect(m_ui.ThresholdSlider, SIGNAL(valuesChanged(double, double)), this, SLOT(ThresholdChanged(double,double)));
    
    connect(m_ui.ExportBtn, SIGNAL(clicked()), this, SLOT(Export()));
    
}

void ThresholdImageView::SelectionChanged(const mitk::DataNode* node)
{
      if (node)
      {
          mitk::Image* imageData = dynamic_cast<mitk::Image*>(node->GetData());
          if (imageData)
          {
              m_ui.ThresholdSlider->setRange(imageData->GetScalarValueMin(), imageData->GetScalarValueMax());
              m_ui.ThresholdSlider->setValues(imageData->GetScalarValueMin(), imageData->GetScalarValueMax());
              m_ui.ThresholdSlider->setEnabled(true);
              int layer(10);
              node->GetIntProperty("layer", layer);
              layer++;
              m_pResultNode->SetProperty("layer", mitk::IntProperty::New(layer));
          }
          else
          {
              m_ui.ThresholdSlider->setEnabled(false);
          }
      }
      else
      {
          m_ui.ThresholdSlider->setEnabled(false);

      }
}

void ThresholdImageView::Export()
{
    if (m_pResultNode&&m_ui.ImageSelector->GetSelectedNode())
    {

        mitk::DataNode::Pointer newNode = mitk::DataNode::New();
        mitk::Image::Pointer image = mitk::Image::New();
        image->Initialize(static_cast<mitk::Image*>(m_pResultNode->GetData()));
        auto data = vtkSmartPointer<vtkImageData>::New();
        data->DeepCopy(static_cast<mitk::Image*>(m_pResultNode->GetData())->GetVtkImageData());
        image->SetVolume(data->GetScalarPointer());
        newNode->SetData(image);
        newNode->SetColor(1.0, 0.0, 0.0);
        QString name = QString("Threshold(%1,%2)").arg(m_ui.ThresholdSlider->minimumValue()).arg(m_ui.ThresholdSlider->maximumValue());

        newNode->SetName(name.toLocal8Bit().constData());
        GetDataStorage()->Add(newNode, m_ui.ImageSelector->GetSelectedNode());
        RequestRenderWindowUpdate();
    }
}

void ThresholdImageView::ThresholdChanged(double minValue, double maxValue)
{
    if (m_ui.ImageSelector->GetSelectedNode()&& m_ui.ThresholdSlider->isEnabled())
    {
        mitk::Image* imageData = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());
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
            mitk::Image::Pointer resultImage;
            mitk::CastToMitkImage(btFilter->GetOutput(), resultImage);
            m_pResultNode->SetData(resultImage);
            m_pResultNode->SetColor(1.0, 0.0, 0.0);
            m_pResultNode->SetVisibility(true);
            m_pResultNode->Modified();
            RequestRenderWindowUpdate();
        } 
        else
        {
            m_pResultNode->SetVisibility(false);
        }
    }
}


WndHandle ThresholdImageView::GetPluginHandle()
{
    return this;
}