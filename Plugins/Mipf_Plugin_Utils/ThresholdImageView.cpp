#include "ThresholdImageView.h"


#include "ITKImageTypeDef.h"

#include "itkBinaryThresholdImageFilter.h"
#include "itkThresholdImageFilter.h"

#include "mitkImageCast.h"



ThresholdImageView::ThresholdImageView()
{
    m_pResultNode = nullptr;

}


ThresholdImageView::~ThresholdImageView()
{

}

void ThresholdImageView::CreateView()
{
    m_ui.setupUi(this);
    m_pMain->Attach(this);

    m_ui.ImageSelector->SetDataStorage(GetDataStorage());
    m_ui.ImageSelector->SetPredicate(CreateImagePredicate());
    connect(m_ui.ImageSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(SelectionChanged(const mitk::DataNode*)));

    m_ui.ThresholdSlider->setTracking(false);
    m_ui.ThresholdSlider->setDecimals(1);
    m_ui.ThresholdSlider->setSpinBoxAlignment(Qt::AlignVCenter);
    m_ui.WorkWidget->setEnabled(false);
    connect(m_ui.ThresholdSlider, SIGNAL(valuesChanged(double, double)), this, SLOT(ThresholdChanged(double,double)));
    
    connect(m_ui.ExportBtn, SIGNAL(clicked()), this, SLOT(Export()));

    m_ui.VisibleCB->setChecked(false);
    connect(m_ui.VisibleCB, SIGNAL(clicked(bool)), this, SLOT(VisibleChanged(bool)));
    connect(m_ui.ExportBinaryCB, SIGNAL(clicked(bool)), this, SLOT(ExportTypeChanged(bool)));

    m_ui.ReplaceValueLE->setVisible(false);
    
}

void ThresholdImageView::ExportTypeChanged(bool checked)
{
     if (checked)
     {
         m_ui.ReplaceValueLE->setVisible(false);
     }
     else
     {
         if (m_ui.ImageSelector->GetSelectedNode())
         {
             mitk::Image* image = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());
             m_ui.ReplaceValueLE->setText(QString("%1").arg(image->GetScalarValueMin()));
         }
         m_ui.ReplaceValueLE->setVisible(true);
     }

}

void ThresholdImageView::showEvent(QShowEvent *event)
{
       if (m_pResultNode.IsNotNull())
       {
           m_pResultNode->SetVisibility(true);
       }
}

void ThresholdImageView::hideEvent(QHideEvent *event)
{
    if (m_pResultNode.IsNotNull())
    {
        m_pResultNode->SetVisibility(false);
    }
}

void ThresholdImageView::VisibleChanged(bool visible)
{
      if (m_pResultNode.IsNotNull())
      {
          m_pResultNode->SetVisibility(visible);
          int layer(10);
          m_ui.ImageSelector->GetSelectedNode()->GetIntProperty("layer", layer); 
          m_pResultNode->SetProperty("layer", mitk::IntProperty::New(visible? layer++:layer--)); 
          m_pResultNode->Modified();
          RequestRenderWindowUpdate();
      }
}

void ThresholdImageView::SelectionChanged(const mitk::DataNode* node)
{
    if (m_pResultNode.IsNotNull())
    {
        GetDataStorage()->Remove(m_pResultNode);
        m_pResultNode = NULL;
    }
    m_ui.VisibleCB->setChecked(false);
    if (node)
    {
        mitk::Image* imageData = dynamic_cast<mitk::Image*>(node->GetData());
        if (imageData)
        {
            m_pResultNode = mitk::DataNode::New();
            m_pResultNode->SetBoolProperty("helper object", true);
            m_pResultNode->SetVisibility(false);
            GetDataStorage()->Add(m_pResultNode,(mitk::DataNode*)node);
            m_ui.ThresholdSlider->setRange(imageData->GetScalarValueMin(), imageData->GetScalarValueMax());
            m_ui.ThresholdSlider->setValues(imageData->GetScalarValueMin(), imageData->GetScalarValueMax());
            m_ui.WorkWidget->setEnabled(true);
        }
        else
        {
            m_ui.WorkWidget->setEnabled(false);
        }
    }
    else
    {
        m_ui.WorkWidget->setEnabled(false);

    }
}

void ThresholdImageView::Export()
{
    if (m_pResultNode&&m_ui.ImageSelector->GetSelectedNode())
    {
        QString name = QString("Threshold(%1,%2)").arg(m_ui.ThresholdSlider->minimumValue()).arg(m_ui.ThresholdSlider->maximumValue());
        if (m_ui.ExportBinaryCB->isChecked())
        {
            auto data = vtkSmartPointer<vtkImageData>::New();
            data->DeepCopy(static_cast<mitk::Image*>(m_pResultNode->GetData())->GetVtkImageData());
            ImportVTKImage(data, ("Binary"+name).toLocal8Bit().constData(), m_ui.ImageSelector->GetSelectedNode());   
        }
        else
        {
            mitk::Image* imageData = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());
            if (imageData->GetDimension() == 3)
            {
                Int3DImageType::Pointer itkImage;
                mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData()), itkImage);
                typedef itk::ThresholdImageFilter<Int3DImageType> thresholdImageType;
                thresholdImageType::Pointer filter = thresholdImageType::New();
                filter->SetInput(itkImage);
                filter->ThresholdOutside(m_ui.ThresholdSlider->minimumValue(), m_ui.ThresholdSlider->maximumValue());
                filter->SetOutsideValue(m_ui.ReplaceValueLE->text().toDouble());
                filter->Update();
                ImportITKImage(filter->GetOutput(), name.toLocal8Bit().constData(), m_ui.ImageSelector->GetSelectedNode());
            }
            else if (imageData->GetDimension() == 2)
            {
                Int2DImageType::Pointer itkImage;
                mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData()), itkImage);
                typedef itk::ThresholdImageFilter<Int2DImageType> thresholdImageType;
                thresholdImageType::Pointer filter = thresholdImageType::New();
                filter->SetInput(itkImage);
                filter->ThresholdOutside(m_ui.ThresholdSlider->minimumValue(), m_ui.ThresholdSlider->maximumValue());
                filter->SetOutsideValue(m_ui.ReplaceValueLE->text().toDouble());
                filter->Update();
                ImportITKImage(filter->GetOutput(), name.toLocal8Bit().constData(), m_ui.ImageSelector->GetSelectedNode());

            }
            
        }
        
    }
}

void ThresholdImageView::ThresholdChanged(double minValue, double maxValue)
{
    if (m_ui.ImageSelector->GetSelectedNode()&& m_ui.ThresholdSlider->isEnabled())
    {
        mitk::Image* imageData = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());
        if (imageData)
        {
           
            mitk::Image::Pointer resultImage;
            if (imageData->GetDimension()==3)
            {
                Int3DImageType::Pointer itkImage;
                mitk::CastToItkImage(imageData, itkImage);
                typedef itk::BinaryThresholdImageFilter<Int3DImageType, UChar3DImageType> btImageFilterType;
                btImageFilterType::Pointer btFilter = btImageFilterType::New();
                btFilter->SetInput(itkImage);
                btFilter->SetLowerThreshold(m_ui.ThresholdSlider->minimumValue());
                btFilter->SetUpperThreshold(m_ui.ThresholdSlider->maximumValue());
                btFilter->SetInsideValue(255);
                btFilter->SetOutsideValue(0);
                btFilter->Update();
                mitk::CastToMitkImage(btFilter->GetOutput(), resultImage);
            }
            else if (imageData->GetDimension() == 2)
            {
                Int2DImageType::Pointer itkImage;
                mitk::CastToItkImage(imageData, itkImage);
                typedef itk::BinaryThresholdImageFilter<Int2DImageType, UChar2DImageType> btImageFilterType;
                btImageFilterType::Pointer btFilter = btImageFilterType::New();
                btFilter->SetInput(itkImage);
                btFilter->SetLowerThreshold(m_ui.ThresholdSlider->minimumValue());
                btFilter->SetUpperThreshold(m_ui.ThresholdSlider->maximumValue());
                btFilter->SetInsideValue(255);
                btFilter->SetOutsideValue(0);
                btFilter->Update();
                mitk::CastToMitkImage(btFilter->GetOutput(), resultImage);
            }      
            m_pResultNode->SetData(resultImage);
            m_pResultNode->SetColor(1.0, 0.0, 0.0);
            m_pResultNode->SetVisibility(true);
            m_ui.VisibleCB->setChecked(true);
            int layer(10);
            m_ui.ImageSelector->GetSelectedNode()->GetIntProperty("layer", layer);
            layer++;
            m_pResultNode->SetProperty("layer", mitk::IntProperty::New(layer));
            m_pResultNode->SetBoolProperty("helper object", true);
            m_pResultNode->Modified();
            RequestRenderWindowUpdate();
        } 
        else
        {
            m_pResultNode->SetVisibility(false);
            m_ui.VisibleCB->setChecked(false);
        }
    }
}


WndHandle ThresholdImageView::GetPluginHandle()
{
    return this;
}