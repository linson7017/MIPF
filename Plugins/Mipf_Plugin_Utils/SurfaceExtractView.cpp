#include "SurfaceExtractView.h"
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
#include "mitkLabelSetImage.h"
#include <mitkShowSegmentationAsSmoothedSurface.h>
#include <mitkShowSegmentationAsSurface.h>
#include <mitkStatusBar.h>
#include "mitkImageToItk.h"
#include <itkAddImageFilter.h>
#include <itkBinaryMedianImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkConstantPadImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkMultiplyImageFilter.h>
#include <itkRegionOfInterestImageFilter.h>
#include <itkIntelligentBinaryClosingFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include "itkConnectedComponentImageFilter.h"
#include "itkLabelShapeKeepNObjectsImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include <mitkGeometry3D.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageToSurfaceFilter.h>
#include <mitkProgressBar.h>
#include <mitkStatusBar.h>
#include <mitkUIDGenerator.h>
#include <mitkVtkRepresentationProperty.h>
#include <mitkManualSegmentationToSurfaceFilter.h>
#include <vtkCleanPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkQuadricDecimation.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkPolyDataConnectivityFilter.h>

#include <vtkProperty.h>

#include "MitkSegmentation/IQF_MitkSurfaceTool.h"

using namespace mitk;


SurfaceExtractView::SurfaceExtractView() : MitkPluginView()
{

    connect(&m_Watcher, SIGNAL(finished()), this, SLOT(ShowResult()));
}


SurfaceExtractView::~SurfaceExtractView()
{
}

void SurfaceExtractView::CreateView()
{
    m_ui.setupUi(this);

    m_ui.ImageSelector->SetPredicate(CreatePredicate(Image));
    m_ui.ImageSelector->SetDataStorage(GetDataStorage());

    m_ui.SurfaceSelector->SetPredicate(CreatePredicate(Surface));
    m_ui.SurfaceSelector->SetDataStorage(GetDataStorage());

    m_ui.SimplifySurfaceSelector->SetPredicate(CreatePredicate(Surface));
    m_ui.SimplifySurfaceSelector->SetDataStorage(GetDataStorage());

    connect(m_ui.ImageExtractBtn, SIGNAL(clicked()), this, SLOT(Extract()));
    connect(m_ui.SurfaceSmoothBtn, SIGNAL(clicked()), this, SLOT(Smooth()));
    connect(m_ui.SimplifyBtn, SIGNAL(clicked()), this, SLOT(Simplify()));

}

void SurfaceExtractView::Simplify()
{
    IQF_MitkSurfaceTool* pSurfaceTool = (IQF_MitkSurfaceTool*)m_pMain->GetInterfacePtr(QF_MitkSurface_Tool);
    if (pSurfaceTool)
    {
        
        mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_ui.SurfaceSelector->GetSelectedNode()->GetData());
        vtkSmartPointer<vtkPolyData> simplifideSurface = vtkSmartPointer<vtkPolyData>::New();

        double simplifyRate = m_ui.SimplifyRate->value();
        int simplifyMode = m_ui.SimplifyMode->currentIndex();
        pSurfaceTool->SimplifySurfaceMesh(surface->GetVtkPolyData(), simplifideSurface, simplifyRate, simplifyMode);

        mitk::Surface::Pointer mitkSurface = mitk::Surface::New();
        mitkSurface->SetVtkPolyData(simplifideSurface);

        QString resultName = "Surface_Simplifed_";
        resultName.append(m_ui.SimplifyMode->currentText()+"_");
        resultName.append(m_ui.SimplifyRate->text());
        mitk::DataNode::Pointer result = mitk::DataNode::New();
        result->SetData(mitkSurface);
        result->SetName(resultName.toStdString());
        result->SetColor(1.0, 1.0, 0.0);
        result->SetProperty("material.representation", mitk::VtkRepresentationProperty::New(VTK_WIREFRAME));

        GetDataStorage()->Add(result, m_ui.SurfaceSelector->GetSelectedNode());
    }
}

void SurfaceExtractView::Extract()
{
    m_SurfaceName = QInputDialog::getText(NULL, "Input Result Name", "Image Name:");
    if (m_SurfaceName.isEmpty())
    {
        return;
    }
    int smooth = 0;
    bool largestConnect = false;
    smooth = m_ui.ImageSmoothTimes->text().toInt();
    largestConnect = m_ui.ImageLargestConnected->isChecked();

    mitk::Image* image = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());

    if (m_Watcher.isRunning())
        m_Watcher.waitForFinished();
    m_Future = QtConcurrent::run(this, &SurfaceExtractView::ExtractSurface, image, smooth, largestConnect);
    m_Watcher.setFuture(m_Future);

    //ShowResult();
    return;
}
void SurfaceExtractView::Smooth()
{
    IQF_MitkSurfaceTool* pSurfaceTool = (IQF_MitkSurfaceTool*)m_pMain->GetInterfacePtr(QF_MitkSurface_Tool);
    if (pSurfaceTool)
    {
        int smootTimes = m_ui.SurfaceSmoothTimes->text().toInt();
        mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_ui.SurfaceSelector->GetSelectedNode()->GetData());
        vtkSmartPointer<vtkPolyData> smoothSurface = vtkSmartPointer<vtkPolyData>::New();
        pSurfaceTool->SmoothTubeSurface(surface->GetVtkPolyData(), smoothSurface, smootTimes);

        mitk::Surface::Pointer mitkSurface = mitk::Surface::New();
        mitkSurface->SetVtkPolyData(smoothSurface);

        QString resultName = "Surface_Smoothed_";
        resultName.append(m_ui.SurfaceSmoothTimes->text().toInt());
        mitk::DataNode::Pointer result = mitk::DataNode::New();
        result->SetData(mitkSurface);
        result->SetName(resultName.toStdString());
        result->SetColor(1.0, 1.0, 0.0);
        GetDataStorage()->Add(result, m_ui.SurfaceSelector->GetSelectedNode());
    }
}

void SurfaceExtractView::ExtractSurface(mitk::Image* image, int smooth, bool largestConnect)
{
    IQF_MitkSurfaceTool* pSurfaceTool = (IQF_MitkSurfaceTool*)m_pMain->GetInterfacePtr(QF_MitkSurface_Tool);
    if (pSurfaceTool)
    {
        vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
        pSurfaceTool->ExtractSurface(image,polydata, smooth, largestConnect);
        m_pSurface = mitk::Surface::New();
        m_pSurface->SetVtkPolyData(polydata);
    }
}

void SurfaceExtractView::GetForeground( Float3DImageType* image, UChar3DImageType* outputImage)
{
    typedef itk::BinaryThresholdImageFilter<Float3DImageType, UInt3DImageType>  ThresholdFilterType;
    ThresholdFilterType::Pointer thFilter = ThresholdFilterType::New();
    thFilter->SetLowerThreshold(-300);
    thFilter->SetUpperThreshold(1500);
    thFilter->SetInsideValue(1);
    thFilter->SetOutsideValue(0);
    thFilter->SetInput(image);
    thFilter->UpdateLargestPossibleRegion();

    typedef itk::LabelShapeKeepNObjectsImageFilter< UInt3DImageType > LabelShapeKeepNObjectsImageFilterType;
    LabelShapeKeepNObjectsImageFilterType::Pointer labelShapeKeepNObjectsImageFilter = LabelShapeKeepNObjectsImageFilterType::New();
    labelShapeKeepNObjectsImageFilter->SetInput(thFilter->GetOutput());
    labelShapeKeepNObjectsImageFilter->SetBackgroundValue(0);
    labelShapeKeepNObjectsImageFilter->SetNumberOfObjects(1);
    labelShapeKeepNObjectsImageFilter->SetAttribute(LabelShapeKeepNObjectsImageFilterType::LabelObjectType::NUMBER_OF_PIXELS);

    typedef itk::RescaleIntensityImageFilter< UInt3DImageType, UChar3DImageType > RescaleFilterType;
    RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
    rescaleFilter->SetOutputMinimum(0);
    rescaleFilter->SetOutputMaximum(1);
    rescaleFilter->SetInput(labelShapeKeepNObjectsImageFilter->GetOutput());
    rescaleFilter->Update();

    outputImage->Graft(rescaleFilter->GetOutput());
}

void SurfaceExtractView::ShowResult()
{
    DataNode::Pointer node = DataNode::New();

    bool wireframe = false;

    if (wireframe)
    {
        VtkRepresentationProperty *representation =
            dynamic_cast<VtkRepresentationProperty *>(node->GetProperty("material.representation"));

        if (representation != nullptr)
            representation->SetRepresentationToWireframe();
    }

    node->SetProperty("opacity", FloatProperty::New(0.5));
    node->SetProperty("line width", IntProperty::New(1));
    node->SetProperty("scalar visibility", BoolProperty::New(false));

    std::string groupNodeName = "surface";
    DataNode *groupNode = m_ui.ImageSelector->GetSelectedNode();

    if (groupNode != nullptr)
        groupNode->GetName(groupNodeName);

    node->SetProperty("name", StringProperty::New(m_SurfaceName.toStdString()));
    node->SetData(m_pSurface);

    BaseProperty *colorProperty = groupNode->GetProperty("color");

    node->SetProperty("color", ColorProperty::New(1.0f, 1.0f, 0.0f));

    bool showResult = true;

    bool syncVisibility = false;

    Image::Pointer image = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());

    BaseProperty *organTypeProperty = image->GetProperty("organ type");

    if (organTypeProperty != nullptr)
        m_pSurface->SetProperty("organ type", organTypeProperty);

    BaseProperty *visibleProperty = groupNode->GetProperty("visible");

    if (visibleProperty != nullptr && syncVisibility)
        node->ReplaceProperty("visible", visibleProperty->Clone());
    else
        node->SetProperty("visible", BoolProperty::New(showResult));

    GetDataStorage()->Add(node, groupNode);
}


void SurfaceExtractView::OnSurfaceCalculationDone()
{
    mitk::StatusBar::GetInstance()->Clear();
}
