#include "ItkAlgorithmSetView.h"
#include "iqf_main.h"
#include "Res/R.h"

//qt
#include <QtWidgets>

//mitk
#include "mitkImage.h"
#include "mitkImageCast.h"
#include "mitkSurface.h"

#include "MitkMain/IQF_MitkDataManager.h"
#include "MitkMain/IQF_MitkRenderWindow.h"
#include "Grid/Math/IQF_MathUtil.h"

#include "Tracking/IQF_SliceBySliceTracking.h"

#include "QmitkPointListViewWidget.h"
#include "QmitkPointListWidget.h"

#include "vtkPolyData.h"

#include "itkBinaryThresholdImageFilter.h"

#include "ITKImageTypeDef.h"


ItkAlgorithmSetView::ItkAlgorithmSetView(QF::IQF_Main* pMain):PluginView(pMain)
{
    m_pMain->Attach(this);
}


void ItkAlgorithmSetView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "ITKAlgorithmSet.BinaryThreshold") == 0)
    {
        IQF_MitkRenderWindow* pwindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
        QmitkRenderWindow* window = pwindow->GetActiveMitkRenderWindow();
        IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr("QF_MitkMain_DataManager");
        if (pMitkDataManager)
        {
            IQF_SliceBySliceTracking* pTrakcer = (IQF_SliceBySliceTracking*)m_pMain->GetInterfacePtr(QF_Algorithm_SliceBySliceTracking);
            
            //Set Current Image
            mitk::DataNode::Pointer currentNode = pMitkDataManager->GetCurrentNode();
            mitk::Image::Pointer image = 0;
            if (currentNode.IsNotNull())
            {
                image = dynamic_cast<mitk::Image *>(currentNode->GetData());
            }
            if (!image)
            {
                std::cout << "Image NULL" << std::endl;
                return;
            }

            QSlider* slider = (QSlider*)m_pR->getObjectFromGlobalMap("ITKAlgorithmSet.BinaryThreshold.Slider");
            if (!slider)
            {
                return;
            }
            
            Float3DImagePointerType itkImage = Float3DImageType::New();
            mitk::CastToItkImage<Float3DImageType>(image, itkImage);

            typedef itk::BinaryThresholdImageFilter<Float3DImageType, UChar3DImageType> binaryThresholdImageFilterType;
            binaryThresholdImageFilterType::Pointer filter = binaryThresholdImageFilterType::New();
            filter->SetInput(itkImage);
            filter->SetLowerThreshold(slider->value());
            filter->SetUpperThreshold(image->GetScalarValueMax());
            filter->Update();

            mitk::Image::Pointer outputImage = mitk::Image::New();
            mitk::CastToMitkImage<UChar3DImageType>(filter->GetOutput(), outputImage);
            
            mitk::DataNode::Pointer outputNode = mitk::DataNode::New();
            outputNode->SetData(outputImage);
            outputNode->SetName(currentNode->GetName()+"_BinaryThreshold");
            pMitkDataManager->GetDataStorage()->Add(outputNode);
        }
    }
    else if (strcmp(szMessage, MITK_MESSAGE_SELECTION_CHANGED) == 0)
    {
        IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)pValue;
        if (!pMitkDataManager)
        {
            return;
        }
        mitk::DataNode::Pointer currentNode = pMitkDataManager->GetCurrentNode();
        mitk::Image::Pointer image = 0;
        if (currentNode.IsNotNull())
        {
            image = dynamic_cast<mitk::Image *>(currentNode->GetData());
        }
        if (!image)
        {
            std::cout << "Image NULL" << std::endl;
            return;
        }
        //BinaryThreshold

        QSlider* slider = (QSlider*)m_pR->getObjectFromGlobalMap("ITKAlgorithmSet.BinaryThreshold.Slider");
        QSpinBox* spinnerBox = (QSpinBox*)m_pR->getObjectFromGlobalMap("ITKAlgorithmSet.BinaryThreshold.SpinBox");
        if (slider&&spinnerBox)
        {
            slider->setEnabled(true);
            spinnerBox->setEnabled(true);

            QObject::disconnect(slider, &QSlider::valueChanged, spinnerBox, &QSpinBox::setValue);
            QObject::disconnect(spinnerBox, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));
            slider->setRange(image->GetScalarValueMin(), image->GetScalarValueMax());
            spinnerBox->setRange(image->GetScalarValueMin(), image->GetScalarValueMax());
            QObject::connect(slider, &QSlider::valueChanged, spinnerBox, &QSpinBox::setValue);
            QObject::connect(spinnerBox, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));

            slider->setValue(image->GetScalarValueMin());          
        }
    }
    else if (strcmp(szMessage, MITK_MESSAGE_MULTIWIDGET_INIT) == 0)
    {
        IQF_MitkRenderWindow* pRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
        if (pRenderWindow)
        {
            m_PointListWidget = new QmitkPointListWidget();

            m_PointSet = mitk::PointSet::New();
            mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
            pointSetNode->SetData(m_PointSet);
            pointSetNode->SetName("seed points for tracking");
            pointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true));
            pointSetNode->SetProperty("layer", mitk::IntProperty::New(1024));

            // add the pointset to the data storage (for rendering and access by other modules)
            IQF_MitkDataManager* pMitkDataStorage = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr("QF_MitkMain_DataManager");
            pMitkDataStorage->GetDataStorage()->Add(pointSetNode);
            // tell the GUI widget about the point set
            m_PointListWidget->SetPointSetNode(pointSetNode);  
            m_PointListWidget->SetMultiWidget(pRenderWindow->GetMitkStdMultiWidget());
            m_pR->registerCustomWidget("ItkAlgorithSetViewSeedWidget", m_PointListWidget);
        }     
    }
}


void ItkAlgorithmSetView::ShowResults(std::vector< std::vector<Vector3> > graph)
{

    IQF_MathUtil* pMathUtil = (IQF_MathUtil*)m_pMain->GetInterfacePtr(QF_Algorithm_MathUtil);
    if (!pMathUtil)
    {
        return;
    }
    int color[6][3] = { { 1.0,0.0,1.0 },
    { 1.0,0.0,0.0 },
    { 0.0,1.0,0.0 },
    { 0.0,0.0,1.0 },
    { 1.0,1.0,0.0 },
    { 0.0,1.0,1.0 } };

    for (int i = 0; i < graph.size(); i++)
    {
        std::vector<Vector3> points = graph.at(i);
        Vector3 center, normal;
        pMathUtil->FitLine(points, center,normal);

        vtkSmartPointer<vtkPolyData> linepolydata = vtkSmartPointer<vtkPolyData>::New();
        vtkSmartPointer<vtkPoints> linepoints = vtkSmartPointer<vtkPoints>::New();
        double ext = 150;
        linepoints->InsertNextPoint(center.x()-normal.x()*ext, center.y() - normal.y()*ext, center.z() - normal.z()*ext);
        linepoints->InsertNextPoint(center.x() + normal.x()*ext, center.y() + normal.y()*ext, center.z() + normal.z()*ext);
        linepolydata->Allocate();
        linepolydata->SetPoints(linepoints);
        vtkIdType connectivity[2];
        connectivity[0] = 0;
        connectivity[1] = 1;
        linepolydata->InsertNextCell(VTK_LINE, 2, connectivity);
        
        mitk::Surface::Pointer line = mitk::Surface::New();
        line->SetVtkPolyData(linepolydata);
        line->Update();
        mitk::DataNode::Pointer lineNode = mitk::DataNode::New();
        lineNode->SetData(line);
        std::string name = "line";
        char num[10];
        name.append(itoa(i+1, num,10));
        lineNode->SetProperty("name", mitk::StringProperty::New(name));
        lineNode->SetProperty("color", mitk::ColorProperty::New(1.0, 1.0, 1.0));
        lineNode->Update();


        IQF_MitkDataManager* pMitkDataStorage = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr("QF_MitkMain_DataManager");
        if (pMitkDataStorage)
        {
            pMitkDataStorage->GetDataStorage()->Add(lineNode, pMitkDataStorage->GetCurrentNode());
            mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
    }
}

void ItkAlgorithmSetView::InitResource(R* pR)
{
    m_pR = pR;
    //BinaryThreshold
    QSlider* slider = (QSlider*)m_pR->getObjectFromGlobalMap("ITKAlgorithmSet.BinaryThreshold.Slider");
    QSpinBox* spinnerBox = (QSpinBox*)m_pR->getObjectFromGlobalMap("ITKAlgorithmSet.BinaryThreshold.SpinBox");
    if (slider&&spinnerBox)
    {
        slider->setEnabled(false);
        spinnerBox->setEnabled(false);
    }
}

