#include "CVSegmentationView.h" 
#include "iqf_main.h"  

#include "CVA/IQF_CVAlgorithms.h"
#include "Core/IQF_ObjectFactory.h"

#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkImageResample.h"
  
#include "mitkImageCast.h"
#include <vtkImageCast.h>

#include "QmitkStdMultiWidget.h"
#include "QMessageBox.h"
CVSegmentationView::CVSegmentationView() :MitkPluginView() 
{
}
 
CVSegmentationView::~CVSegmentationView() 
{
}
 
void CVSegmentationView::CreateView()
{
    this->algorithm = new ImageAlgorithm;
    m_ui.setupUi(this);
    m_ui.DataSelector->SetPredicate(CreateImagePredicate());
    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    connect(m_ui.DataSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnImageSelectionChanged(const mitk::DataNode *)));

    

    connect(m_ui.minThresholdSlider, SIGNAL(valueChanged(int)), this, SLOT(minThresholdValueChanged(int)));
    connect(m_ui.maxThresholdSlider, SIGNAL(valueChanged(int)), this, SLOT(maxThresholdValueChanged(int)));
    connect(m_ui.generateSurfaceButton, SIGNAL(pressed()), this, SLOT(generateSurface()));
    connect(m_ui.removeSurfaceButton, SIGNAL(pressed()), this, SLOT(removeSurface()));
    connect(m_ui.saveSurfaceButton, SIGNAL(pressed()), this, SLOT(saveSurface()));
    connect(m_ui.loadSurfaceButton, SIGNAL(pressed()), this, SLOT(loadSurface()));
    connect(m_ui.sliceVisibilityBox, SIGNAL(stateChanged(int)), this, SLOT(sliceVisibilityChanged(int)));
    connect(m_ui.vesselVisibilityBox, SIGNAL(stateChanged(int)), this, SLOT(vesselVisibilityChanged(int)));
    connect(m_ui.needlePathVisibilityBox, SIGNAL(stateChanged(int)), this,
        SLOT(needlePathVisibilityChanged(int)));
    connect(m_ui.aneurysmVisibilityBox, SIGNAL(stateChanged(int)), this,
        SLOT(aneurysmVisibilityChanged(int)));
    connect(m_ui.measuredistance0box, SIGNAL(stateChanged(int)), this, SLOT(measureDistance0Changed(int)));
    connect(m_ui.measuredistance1box, SIGNAL(stateChanged(int)), this, SLOT(measureDistance1Changed(int)));
    connect(m_ui.measuredistance2box, SIGNAL(stateChanged(int)), this, SLOT(measureDistance2Changed(int)));
    connect(m_ui.measureangle0box, SIGNAL(stateChanged(int)), this, SLOT(measureAngle0Changed(int)));
    connect(m_ui.measureangle1box, SIGNAL(stateChanged(int)), this, SLOT(measureAngle1Changed(int)));
    connect(m_ui.measureangle2box, SIGNAL(stateChanged(int)), this, SLOT(measureAngle2Changed(int)));

    connect(m_ui.addSeedButton, SIGNAL(pressed()), this, SLOT(addSeedPoint()));
    //connect(m_ui.removeSeedButton, SIGNAL(pressed()), this, SLOT(removeSeedPoint()));
    connect(m_ui.resetSeedButton, SIGNAL(pressed()), this, SLOT(resetSeedPoints()));
    connect(m_ui.setStartButton, SIGNAL(pressed()), this, SLOT(setStartPoint()));
    connect(m_ui.setEndButton, SIGNAL(pressed()), this, SLOT(setEndPoint()));
    connect(m_ui.generateNeedleButton, SIGNAL(pressed()), this, SLOT(generateNeedle()));
    connect(m_ui.generateMouldButton, SIGNAL(pressed()), this, SLOT(generateMould()));
    connect(m_ui.saveMouldButton, SIGNAL(pressed()), this, SLOT(saveMould()));
    connect(m_ui.pruneRadiusSlider, SIGNAL(valueChanged(int)), this, SLOT(pruneRadiusValueChanged(int)));
    connect(m_ui.pruneVesselTreeButton, SIGNAL(pressed()), this, SLOT(pruneVesselTree()));
    connect(m_ui.generateRectSurfaceButton, SIGNAL(pressed()), this, SLOT(generateRectSurface()));
    connect(m_ui.generateExtensionsButton, SIGNAL(pressed()), this, SLOT(generateExtensions()));
    connect(m_ui.generateVesselWallSurfaceButton, SIGNAL(pressed()), this, SLOT(generateVesselWallSurface()));
    connect(m_ui.wallThicknessSlider, SIGNAL(valueChanged(int)), this, SLOT(wallThicknessValueChanged(int)));
    connect(m_ui.autoSelectRangeButton, SIGNAL(pressed()), this, SLOT(autoSelectRange()));
    connect(m_ui.vesselnessSlider, SIGNAL(valueChanged(int)), this, SLOT(vesselnessThresholdChanged(int)));

    connect(m_ui.addAneurysmPointButton, SIGNAL(pressed()), this, SLOT(addAneurysmPoint()));
    connect(m_ui.setAneurysmLocationPoint1Button, SIGNAL(pressed()), this, SLOT(setAneurysmLocationPoint1()));
    connect(m_ui.setAneurysmLocationPoint2Button, SIGNAL(pressed()), this, SLOT(setAneurysmLocationPoint2()));
    connect(m_ui.detectAneuButton, SIGNAL(pressed()), this, SLOT(detectAneurysm()));

} 
 
WndHandle CVSegmentationView::GetPluginHandle() 
{
    return this; 
}

void CVSegmentationView::generateSurface()
{
    if (!this->algorithm->getInputData().Get())
    {
        return;
    }
    vtkSmartPointer<vtkPolyData> surfaceData = this->algorithm->generateSurface();
    this->algorithm->setCurrentSurfaceShowing(surfaceData);
    m_ui.saveSurfaceButton->setEnabled(true);

    mitk::DataNode::Pointer node = ImportVtkPolyData(surfaceData.Get(),"surface");
    node->GetData()->GetGeometry()->SetOrigin(m_ui.DataSelector->GetSelectedNode()->GetData()->GetGeometry()->GetOrigin());

    ImportVTKImage(this->algorithm->getSegmentedData(), "segmented data",nullptr, m_ui.DataSelector->GetSelectedNode()->GetData()->GetGeometry());
    RequestRenderWindowUpdate();
}


void CVSegmentationView::OnImageSelectionChanged(const mitk::DataNode* node)
{

    if (this->algorithm)
    {
        delete this->algorithm;
        this->algorithm = new ImageAlgorithm();
    }
    if (!node)
    {
        return;
    }
    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());
    vtkSmartPointer<vtkImageCast> castFilter =
        vtkSmartPointer<vtkImageCast>::New();
    castFilter->SetInputData(mitkImage->GetVtkImageData());
    castFilter->SetOutputScalarTypeToFloat();
    castFilter->Update();
    mitkImage->Initialize(castFilter->GetOutput());
    mitkImage->SetVolume(castFilter->GetOutput()->GetScalarPointer());
    this->algorithm->init(mitkImage->GetVtkImageData());

    toggleUIState();
    setThresholds();
}



void CVSegmentationView::autoSelectRange()
{
    this->algorithm->autoSelectRange();
    setThresholds();
}

void CVSegmentationView::toggleUIState()
{
    if (this->algorithm->isPathReady())
    {
        m_ui.pruneRadiusSlider->setEnabled(true);
        m_ui.pruneVesselTreeButton->setEnabled(true);
        m_ui.generateExtensionsButton->setEnabled(true);
        m_ui.generateRectSurfaceButton->setEnabled(true);
        m_ui.generateVesselWallSurfaceButton->setEnabled(true);
        m_ui.wallThicknessSlider->setEnabled(true);
        m_ui.generateNeedleButton->setEnabled(true);
    }
    else
    {
        m_ui.pruneRadiusSlider->setEnabled(false);
        m_ui.pruneVesselTreeButton->setEnabled(false);
        m_ui.generateExtensionsButton->setEnabled(false);
        m_ui.generateRectSurfaceButton->setEnabled(false);
        m_ui.generateVesselWallSurfaceButton->setEnabled(false);
        m_ui.wallThicknessSlider->setEnabled(false);
        m_ui.generateNeedleButton->setEnabled(false);
        m_ui.generateMouldButton->setEnabled(false);
        m_ui.saveMouldButton->setEnabled(false);
    }
}

void CVSegmentationView::setThresholds()
{
    // To prevent unsuccessful setting of values
    m_ui.minThresholdSlider->setMinimum(-999999);
    m_ui.minThresholdSlider->setMaximum(999999);
    m_ui.maxThresholdSlider->setMinimum(-999999);
    m_ui.maxThresholdSlider->setMaximum(999999);

    m_ui.minThresholdSlider->setValue(this->algorithm->getLowerth());
    m_ui.maxThresholdSlider->setValue(this->algorithm->getUpperth());
    m_ui.minThresholdSlider->setMinimum(this->algorithm->getThreshMinimum());
    m_ui.minThresholdSlider->setMaximum(this->algorithm->getThreshMaximum());
    m_ui.maxThresholdSlider->setMinimum(this->algorithm->getThreshMinimum());
    m_ui.maxThresholdSlider->setMaximum(this->algorithm->getThreshMaximum());
}

void CVSegmentationView::vesselnessThresholdChanged(int value)
{
    this->algorithm->setVesselnessThreshold((double)value / 150.0);
}

void CVSegmentationView::minThresholdValueChanged(int value)
{
    this->lowerth = (double)(m_ui.minThresholdSlider->value());
    this->algorithm->setLowerth(this->lowerth);
    thresholdValueChanged();
}

void CVSegmentationView::maxThresholdValueChanged(int value)
{
    this->upperth = (double)(m_ui.maxThresholdSlider->value());
    this->algorithm->setUpperth(this->upperth);
    thresholdValueChanged();
}

void CVSegmentationView::thresholdValueChanged()
{
    /*std::cout << "current threshold " << this->lowerth << " " << this->upperth << std::endl;
    for (int i = 0; i < 3; i++)
    {
        if (riw[i])
        {
            vtkReslice2CursorLineRepresentation *rep = vtkReslice2CursorLineRepresentation::SafeDownCast(riw[i]->GetResliceCursorWidget()->GetRepresentation());
            rep->SetImageThresholdValues(this->lowerth, this->upperth);
            riw[i]->Render();
        }
    }*/
}

void CVSegmentationView::wallThicknessValueChanged(int thickness)
{
    this->algorithm->setWallThickness(thickness);
}

void CVSegmentationView::pruneRadiusValueChanged(int radius)
{
    this->algorithm->setPruneRadius(radius);
}      

void CVSegmentationView::pruneVesselTree()
{
    vtkSmartPointer<vtkPolyData> prunedVessel = this->algorithm->getCurrentSurface();
}

void CVSegmentationView::generateVesselWallSurface()
{
    //ImportVtkPolyData(this->algorithm->generateVesselWallSurface(), "vesse wall surface");

}


void CVSegmentationView::addAneurysmPoint()
{
    double* center = (double*)GetMitkRenderWindowInterface()->GetMitkStdMultiWidget()->GetCrossPosition().GetDataPointer();
    bool isAneurysmPointUsable = this->algorithm->addAneurysmPoint(center);
    if (isAneurysmPointUsable)
    {
        auto sphere = vtkSmartPointer<vtkSphereSource>::New();
        sphere->SetCenter(center);
        sphere->Update();
        ImportVtkPolyData(sphere->GetOutput(),"seed");
    }
}

void CVSegmentationView::detectAneurysm()
{
    AneurysmData aneurysmData = this->algorithm->detectAneurysm();
    
    vtkSmartPointer<vtkPolyData> aneurysmSurfaceData = aneurysmData.aneurysmSurfaceData;
    if (aneurysmSurfaceData == NULL) {
        QMessageBox msgBox;
        //msgBox.setText("Unable to find aneurysm.");
        msgBox.setText(QString::fromUtf8("未能检测到血管瘤"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setButtonText(QMessageBox::Ok, QString::fromUtf8("确定"));
        msgBox.setWindowTitle(QString::fromUtf8("血管瘤"));
        msgBox.exec();
        return;
    }

    mitk::DataNode* node = ImportVtkPolyData(aneurysmData.aneurysmSurfaceData.Get(), "tumor");
    node->GetData()->GetGeometry()->SetOrigin(m_ui.DataSelector->GetSelectedNode()->GetData()->GetGeometry()->GetOrigin());
    node = ImportVtkPolyData(aneurysmData.aneurysmDisplayPolyData.Get(), "display");
    node->GetData()->GetGeometry()->SetOrigin(m_ui.DataSelector->GetSelectedNode()->GetData()->GetGeometry()->GetOrigin());

    RequestRenderWindowUpdate();
}