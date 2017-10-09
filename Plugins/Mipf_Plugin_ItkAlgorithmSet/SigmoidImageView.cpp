#include "SigmoidImageView.h"


//itk
#include "itkSigmoidImageFilter.h"

#include "mitkImageCast.h"
#include "ITKImageTypeDef.h"

SigmoidImageView::SigmoidImageView()
{
}


SigmoidImageView::~SigmoidImageView()
{
}

void SigmoidImageView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Image>::New());
    connect(m_ui.DataSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnImageSelectionChanged(const mitk::DataNode *)));

    m_ui.AlphaValueSlider->setRange(-100.0, 100.0);
    m_ui.AlphaValueSlider->setValue(10.0);

    connect(m_ui.ApplyBtn, SIGNAL(clicked()), this, SLOT(Apply()));
}

WndHandle SigmoidImageView::GetPluginHandle()
{
    return this;
}

void SigmoidImageView::Apply()
{
    if (!m_ui.DataSelector->GetSelectedNode())
    {
        return;
    }

    mitk::Image* image = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());
    if (!image)
    {
        return;
    }

    Float3DImageType::Pointer itkImage;
    mitk::CastToItkImage(image, itkImage);

    typedef itk::SigmoidImageFilter <Float3DImageType, Float3DImageType>
        SigmoidImageFilterType;

    SigmoidImageFilterType::Pointer sigmoidFilter
        = SigmoidImageFilterType::New();
    sigmoidFilter->SetInput(itkImage);
    sigmoidFilter->SetOutputMinimum(0);
    sigmoidFilter->SetOutputMaximum(255);
    sigmoidFilter->SetAlpha(m_ui.AlphaValueSlider->value());
    sigmoidFilter->SetBeta(m_ui.BetaValueSlider->value());
    sigmoidFilter->Update();

    ImportITKImage(sigmoidFilter->GetOutput(), "Sigmoid", m_ui.DataSelector->GetSelectedNode());

    RequestRenderWindowUpdate();
}

void SigmoidImageView::OnImageSelectionChanged(const mitk::DataNode * node)
{
      if (!node)
      {
          return;
      }
      mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
      if (!image)
      {
          return;
      }
      m_ui.BetaValueSlider->setRange(image->GetScalarValueMin(), image->GetScalarValueMax());
      m_ui.BetaValueSlider->setValue(image->GetScalarValueMax());
}