#include "ShapeDrawerView.h"

 //itk
#include "itkBinaryThresholdImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkAddImageFilter.h"

//vtk
#include "vtkPolyData.h"

//mitk
#include "mitkImageCast.h"


#include "MitkSegmentation/IQF_MitkSurfaceTool.h"

#include "ITKImageTypeDef.h"

ShapeDrawerView::ShapeDrawerView(QF::IQF_Main* pMain, QWidget* parent) :QWidget(parent) ,MitkPluginView(pMain)
{
    m_ui.setupUi(this);

    m_ui.MeanImageSelector->SetDataStorage(GetDataStorage());
    m_ui.PCAImageSelector->SetDataStorage(GetDataStorage());
    m_ui.MeanImageSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Image>::New());
    m_ui.PCAImageSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Image>::New());

    

    m_ui.StandardDeviationSlider->setRange(-0.001, 0.001);
    m_ui.StandardDeviationSlider->setTracking(false);
    m_ui.StandardDeviationSlider->setValue(0.00);
    m_ui.StandardDeviationSlider->setSingleStep(0.00001);
    m_ui.StandardDeviationSlider->setTickInterval(0.00001);

    m_ui.StandardDeviationSlider->setDecimals(8);

    m_ui.ThresholdSlider->setRange(-1000, 1000);
    m_ui.ThresholdSlider->setValues(-1000, 0);
    m_ui.ThresholdSlider->setDecimals(0);
    m_ui.ThresholdSlider->setTracking(false);

    connect(m_ui.ApplyBtn, SIGNAL(clicked()), this, SLOT(Apply()));

    connect(m_ui.StandardDeviationSlider, SIGNAL(valueChanged(double)), this, SLOT(Apply()));
    connect(m_ui.ThresholdSlider, SIGNAL(valuesChanged(double , double )), this, SLOT(Apply()));

}


ShapeDrawerView::~ShapeDrawerView()
{
}

void ShapeDrawerView::Apply()
{
    if (!m_ui.MeanImageSelector->GetSelectedNode()||!m_ui.PCAImageSelector->GetSelectedNode())
    {
        return;
    }
    mitk::Image* mitkMeanImage = dynamic_cast<mitk::Image*>(m_ui.MeanImageSelector->GetSelectedNode()->GetData());
    mitk::Image* mitkPCAImage = dynamic_cast<mitk::Image*>(m_ui.PCAImageSelector->GetSelectedNode()->GetData());
    Float3DImageType::Pointer itkMeanImage, itkPCAImage;
    mitk::CastToItkImage(mitkMeanImage, itkMeanImage);
    mitk::CastToItkImage(mitkPCAImage, itkPCAImage);

    typedef itk::BinaryThresholdImageFilter< Float3DImageType, UChar3DImageType>  ThresholdingFilterType;
    typedef itk::MultiplyImageFilter<Float3DImageType, Float3DImageType, Float3DImageType> MultType;
    typedef itk::AddImageFilter<Float3DImageType, Float3DImageType, Float3DImageType> AddFilterType;

    MultType::Pointer multFilter = MultType::New();
    multFilter->SetInput(itkPCAImage);
    multFilter->SetConstant((float)m_ui.StandardDeviationSlider->value()*m_ui.EigenValueLE->text().toDouble());
    multFilter->Update();

    AddFilterType::Pointer addFilter = AddFilterType::New();
    addFilter->SetInput1(itkMeanImage);
    addFilter->SetInput2(multFilter->GetOutput());
    try
    {
        addFilter->Update();
    }
    catch (itk::ExceptionObject & err)
    {
        std::cout << "ExceptionObject caught !" << std::endl;
        std::cout << err << std::endl;
        return;
    }

    ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();
    thresholder->SetLowerThreshold(m_ui.ThresholdSlider->minimumValue());
    thresholder->SetUpperThreshold(m_ui.ThresholdSlider->maximumValue());
    thresholder->SetOutsideValue(0);
    thresholder->SetInsideValue(255);
    thresholder->SetInput(addFilter->GetOutput());
    try
    {
        thresholder->Update();
    }
    catch (itk::ExceptionObject & err)
    {
        std::cout << "ExceptionObject caught !" << std::endl;
        std::cout << err << std::endl;
        return;
    }

    QString resultName = QString("%1").arg(m_ui.StandardDeviationSlider->value(),4,'g',4);

    mitk::Image::Pointer mitkReslutImage;
    auto resultSurface = vtkSmartPointer<vtkPolyData>::New();
    mitk::CastToMitkImage(thresholder->GetOutput(), mitkReslutImage);
    IQF_MitkSurfaceTool* pSurfaceTool = (IQF_MitkSurfaceTool*)GetInterfacePtr(QF_MitkSurface_Tool);
    pSurfaceTool->ExtractSurface(mitkReslutImage, resultSurface.Get());
    

    //mitk::DataNode::Pointer resultImageNode = mitk::DataNode::New();
    //mitk::DataNode::Pointer resultSurfaceNode = mitk::DataNode::New();

    //resultImageNode->SetData(mitkReslutImage);
    //resultImageNode->SetName(resultName.toStdString().c_str());
   // resultImageNode->SetColor(1.0, 0.0, 0.0);
    //GetDataStorage()->Add(resultImageNode, m_ui.PCAImageSelector->GetSelectedNode());

    if (!m_resultNode)
    {
        m_resultNode = mitk::DataNode::New();
        mitk::Surface::Pointer resultSurfaceData = mitk::Surface::New();
        resultSurfaceData->SetVtkPolyData(resultSurface);
        m_resultNode->SetData(resultSurfaceData);
        m_resultNode->SetName(resultName.toStdString().c_str());
        m_resultNode->SetColor(1.0, 1.0, 0.0);
        GetDataStorage()->Add(m_resultNode, m_ui.PCAImageSelector->GetSelectedNode());
    }
    else
    {
        mitk::Surface*  resultSurfaceData = dynamic_cast<mitk::Surface*>(m_resultNode->GetData());
        resultSurfaceData->SetVtkPolyData(resultSurface);
        RequestRenderWindowUpdate();
    }
    

   // ImportITKImage(thresholder->GetOutput(), resultName.toStdString().c_str(), m_ui.MeanImageSelector->GetSelectedNode());
}

