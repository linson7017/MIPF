#include "ItkAlgorithmSetView.h"
#include "iqf_main.h"
#include "Res/R.h"

#include "ITKImageTypeDef.h"

//mitk
#include "mitkImageCast.h"

#include "vtkNrrdReader.h"
#include "vtkJPEGReader.h"

//extern
//#include "extern/itk/itkMultiScaleHessianSmoothed3DToVesselnessMeasureImageFilter.h"
#include "itkAnisotropicDiffusionVesselEnhancementImageFilter.h"

//itk
#include "itkRGBAPixel.h"
#include "itkConvertPixelBuffer.h"


ItkAlgorithmSetView::ItkAlgorithmSetView():MitkPluginView()
{
}


void ItkAlgorithmSetView::CreateView()
{


    vtkSmartPointer<vtkJPEGReader> reader = vtkSmartPointer<vtkJPEGReader>::New();
    reader->SetFileName("D:/temp/UR.jpg");
    reader->Update();

    vtkImageData* image = reader->GetOutput();

    int extent[6];
    image->GetExtent(extent);

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->Initialize(image);
    mitkImage->GetVtkImageData()->DeepCopy(image);


    mitk::CastToMitkImage(itkImage, mitkImage);

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(mitkImage);
    node->SetName("UR");
    GetDataStorage()->Add(node);


    m_ui.setupUi(this);

    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(CreatePredicate(1));

    //VDE
    {
        m_VEDBtn = new QPushButton("Enhance");
        QWidget*   widget = new QWidget;
        QVBoxLayout* layout = new QVBoxLayout;
        widget->setLayout(layout);
        layout->addWidget(m_VEDBtn);

        connect(m_VEDBtn, SIGNAL(clicked()), this, SLOT(OnVesselEnhance()));
        m_ui.AlgorithmContainers->insertWidget(0, widget);
    }
    





    m_ui.AlgorithmContainers->setCurrentWidget(m_VEDBtn);
    connect(m_ui.AlgorithmSelector, SIGNAL(currentIndexChanged(const QString &)),this,SLOT(OnAlgorithmChanged(const QString&)));
}

void ItkAlgorithmSetView::OnAlgorithmChanged(const QString &text)
{
     if (text.compare("Multiscale Vessel Enhance")==0)
     {
         m_ui.AlgorithmContainers->setCurrentWidget(0);
     }
}


void ItkAlgorithmSetView::OnVesselEnhance()
{
    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());
    if (!mitkImage)
    {
        return;
    }

    Float3DImageType::Pointer itkImage;
    mitk::CastToItkImage(mitkImage, itkImage);

    typedef itk::AnisotropicDiffusionVesselEnhancementImageFilter< Float3DImageType,
        Float3DImageType> VesselnessFilterType;

    // Create a vesselness Filter
    VesselnessFilterType::Pointer filter =
        VesselnessFilterType::New();

    filter->SetInput(itkImage);
    filter->SetNumberOfIterations(20);

    filter->SetSensitivity(4.0);
    filter->SetWStrength(24.0);
    filter->SetEpsilon(0.01);

    if (fabs(filter->GetSensitivity() - 4.0) > 0.01)
    {
        std::cerr << "Error Set/Get Sensitivity" << std::endl;
        return ;
    }

    if (fabs(filter->GetWStrength() - 24.0) > 0.01)
    {
        std::cerr << "Error Set/Get Sensitivity" << std::endl;
        return ;
    }

    if (fabs(filter->GetEpsilon() - 0.01) > 0.01)
    {
        std::cerr << "Error Set/Get Sensitivity" << std::endl;
        return ;
    }

    filter->SetSensitivity(5.0);
    filter->SetWStrength(25.0);
    filter->SetEpsilon(10e-2);


    try
    {
        filter->Update();
    }
    catch (itk::ExceptionObject & err)
    {
        std::cerr << "Exception caught: " << err << std::endl;
        return ;
    }

    mitk::Image::Pointer resultMitkImage;
    mitk::CastToMitkImage(filter->GetOutput(),resultMitkImage);

    mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
    resultNode->SetData(resultMitkImage);
    resultNode->SetName("result");
    GetDataStorage()->Add(resultNode);    
}

