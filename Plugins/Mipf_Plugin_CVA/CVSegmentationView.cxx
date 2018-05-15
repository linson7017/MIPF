#include "CVSegmentationView.h" 
#include "iqf_main.h"  
#include "iqf_properties.h"
#include "qf_log.h"

#include "CVA/IQF_CVAlgorithms.h"
#include "Core/IQF_ObjectFactory.h"
#include "MitkStd/IQF_MitkPointList.h"

#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkImageResample.h"
#include "vtkImageCast.h"
#include "vtkSTLWriter.h"
#include "vtkXMLPolyDataWriter.h"

  
#include "mitkImageCast.h"
#include "QmitkStdMultiWidget.h"


#include <QMessageBox.h>
#include <QFileDialog.h>
CVSegmentationView::CVSegmentationView() :MitkPluginView() 
{
}
 
CVSegmentationView::~CVSegmentationView() 
{
    m_pPointList->Release();
}
 
void CVSegmentationView::CreateView()
{
    m_pMain->Attach(this);
    this->algorithm = new ImageAlgorithm;
    m_ui.setupUi(this);
    m_ui.DataSelector->SetPredicate(CreateImagePredicate());
    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    connect(m_ui.DataSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnImageSelectionChanged(const mitk::DataNode *)));

    

    //connect(m_ui.actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
    connect(m_ui.resetButton, SIGNAL(pressed()), this, SLOT(resetViews()));
   // connect(m_ui.actionOpen_Folder, SIGNAL(triggered()), this, SLOT(openFolder()));
   // connect(m_ui.actionAbout_3DPathFinder, SIGNAL(triggered()), this, SLOT(displayAbout()));

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
    connect(m_ui.setStartButton, SIGNAL(clicked(bool)), this, SLOT(setStartPoint(bool)));
    connect(m_ui.setEndButton, SIGNAL(clicked(bool)), this, SLOT(setEndPoint(bool)));
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

    connect(m_ui.addAneurysmPointButton, SIGNAL(clicked(bool)), this, SLOT(addAneurysmPoint(bool)));
    connect(m_ui.setAneurysmLocationPoint1Button, SIGNAL(clicked(bool)), this, SLOT(setAneurysmLocationPoint1(bool)));
    connect(m_ui.setAneurysmLocationPoint2Button, SIGNAL(clicked(bool)), this, SLOT(setAneurysmLocationPoint2(bool)));
    connect(m_ui.detectAneuButton, SIGNAL(pressed()), this, SLOT(detectAneurysm()));


    // Additional ui setup
    m_ui.pruneRadiusSlider->setMinimum(2);
    m_ui.pruneRadiusSlider->setMaximum(10);
    m_ui.pruneRadiusSlider->setValue(4);

    m_ui.wallThicknessSlider->setMinimum(1);
    m_ui.wallThicknessSlider->setMaximum(10);
    m_ui.wallThicknessSlider->setValue(4);

    m_ui.vesselnessSlider->setMinimum(1);
    m_ui.vesselnessSlider->setMaximum(100);
    m_ui.vesselnessSlider->setValue(30);

    m_ui.processingBox->setEnabled(false);
    m_ui.visibilityBox->setEnabled(false);
    m_ui.surfaceBox->setEnabled(false);
    m_ui.cathBox->setEnabled(false);
    m_ui.anuBox->setEnabled(false);

    m_ui.saveSurfaceButton->setEnabled(false);

    //this->windowInitialized = false;


    m_seedPoints = mitk::PointSet::New();
    m_seedPointsNode = mitk::DataNode::New();
    m_seedPointsNode->SetData(m_seedPoints);
    m_seedPointsNode->SetName("seeds");
    GetDataStorage()->Add(m_seedPointsNode);

    IQF_PointListFactory* pFactory = (IQF_PointListFactory*)m_pMain->GetInterfacePtr(QF_MitkStd_PointListFactory);
    if (pFactory)
    {
        m_pPointList = pFactory->CreatePointList();
        m_pPointList->Initialize();
        m_pPointList->SetSingleMode(true);
        m_pPointList->Attach(this);

        m_startPointsNode = m_pPointList->CreateNewPointSetNode();
        m_aneurysmPointsNode = m_pPointList->CreateNewPointSetNode();
        m_endPointsNode = m_pPointList->CreateNewPointSetNode();
        m_aneurysmLocationPoint1Node = m_pPointList->CreateNewPointSetNode();
        m_aneurysmLocationPoint2Node = m_pPointList->CreateNewPointSetNode();


        GetDataStorage()->Add(m_startPointsNode);
        GetDataStorage()->Add(m_endPointsNode);
        GetDataStorage()->Add(m_aneurysmPointsNode);
        GetDataStorage()->Add(m_aneurysmLocationPoint1Node);
        GetDataStorage()->Add(m_aneurysmLocationPoint2Node);
    }
    

} 

void CVSegmentationView::Update(const char* szMessage, int iValue /* = 0 */, void* pValue /* = 0 */)
{
    if (strcmp(szMessage,"CVA_LOAD_DATA")==0)
    {
        QF::IQF_Properties* properties = QF::QF_CreateProperties();
        m_pMain->ExecuteCommand("MITK_MAIN_COMMAND_LOAD_DATA", properties, properties);
        m_ui.sliceVisibilityBox->setCheckState(Qt::Checked);
       

        if (this->algorithm)
        {
            delete this->algorithm;
            this->algorithm = new ImageAlgorithm();
            this->algorithm->setPruneRadius(m_ui.pruneRadiusSlider->value());
            this->algorithm->setWallThickness(m_ui.wallThicknessSlider->value());
            this->algorithm->setVesselnessThreshold(m_ui.vesselnessSlider->value());

        }
        
        mitk::DataNode* node = (mitk::DataNode*)properties->GetPtrProperty("LastLoadedDataNode", nullptr);
        if (!node)
        {
            return;
        }
        mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(node->GetData());
        if (!mitkImage)
        {
            QF_INFO << "The input is not image!";
            return;
        }
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

        m_ui.processingBox->setEnabled(true);
        m_ui.visibilityBox->setEnabled(true);
        m_ui.surfaceBox->setEnabled(true);
        m_ui.cathBox->setEnabled(true);
        m_ui.anuBox->setEnabled(true);
    }
    else  if(strcmp(szMessage, MITK_MESSAGE_POINTLIST_CHANGED) == 0)
    {
        mitk::PointSet* pointSet = (mitk::PointSet*)pValue;
        if (!pointSet->IndexExists(iValue))
        {
            return;
        }
        mitk::Point3D mp = pointSet->GetPoint(iValue);
        double p[3] = { mp[0],mp[1],mp[2] };
       if (static_cast<mitk::PointSet*>(m_startPointsNode->GetData())== pointSet)
       {
           this->algorithm->setStartEndPoint(0, p);
           toggleUIState();
       }
       else if (static_cast<mitk::PointSet*>(m_endPointsNode->GetData()) == pointSet)
       {
           this->algorithm->setStartEndPoint(1, p);
           toggleUIState();
       }
       else if (static_cast<mitk::PointSet*>(m_aneurysmPointsNode->GetData())==pointSet)
       {
           this->algorithm->addAneurysmPoint(p);
       }
       else if (static_cast<mitk::PointSet*>(m_aneurysmLocationPoint1Node->GetData()) == pointSet)
       {
           this->algorithm->setAneurysmLocationPoint(1,p);
       }
       else if (static_cast<mitk::PointSet*>(m_aneurysmLocationPoint2Node->GetData()) == pointSet)
       {
           this->algorithm->setAneurysmLocationPoint(2, p);
       }
    }
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

    mitk::DataNode::Pointer node = ImportVtkPolyData(surfaceData.Get(),"vessel");
    node->GetData()->GetGeometry()->SetOrigin(m_ui.DataSelector->GetSelectedNode()->GetData()->GetGeometry()->GetOrigin());

    ImportVTKImage(this->algorithm->getSegmentedData(), "segmented data",nullptr, m_ui.DataSelector->GetSelectedNode()->GetData()->GetGeometry());
    RequestRenderWindowUpdate();
}


void CVSegmentationView::OnImageSelectionChanged(const mitk::DataNode* node)
{

  
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
        QF_WARN << "Path not ready";
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

    ImportVtkPolyData(this->algorithm->pruneVesselTree(),"pruneVessel")->SetColor(1, 0, 0);
    this->algorithm->setCurrentSurfaceShowing(this->algorithm->getCurrentSurface());
    m_ui.vesselVisibilityBox->setCheckState(Qt::Checked);
}

void CVSegmentationView::generateVesselWallSurface()
{
    this->algorithm->generateVesselWallSurface();
    //ImportVtkPolyData(this->algorithm->generateVesselWallSurface(), "vesse wall surface");
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
    node->SetColor(1.0, 1.0, 0.0);

    ImportVtkPolyData(aneurysmData.aneurysmDisplayPolyData.Get(), "tumor")->SetColor(0.0,1.0,0.0);
    ImportVtkPolyData(aneurysmData.aneurysmDisplayPolyData2.Get(), "tumor")->SetColor(0.0, 0.0, 1.0);

    m_ui.diameterLabel->setText(QString::number(aneurysmData.diameter) + "mm");
    m_ui.heightLabel->setText(QString::number(aneurysmData.height) + "mm");
    m_ui.neckLabel->setText(QString::number(aneurysmData.neckDiameter) + "mm");
    m_ui.volumeLabel->setText(QString::number(aneurysmData.volume) + "mm3");
    m_ui.angleLabel->setText(QString::number(aneurysmData.inflowAngle));

    //node->GetData()->GetGeometry()->SetOrigin(m_ui.DataSelector->GetSelectedNode()->GetData()->GetGeometry()->GetOrigin());
    //node = ImportVtkPolyData(aneurysmData.aneurysmDisplayPolyData.Get(), "display");
    //node->GetData()->GetGeometry()->SetOrigin(m_ui.DataSelector->GetSelectedNode()->GetData()->GetGeometry()->GetOrigin());

    RequestRenderWindowUpdate();
}

void CVSegmentationView::addAngleMeasurementToView(int i)
{
    /*removeDistanceMeasurementToView(i);
    removeAngleMeasurementToView(i);

    if (i == 0) m_ui.measuredistance0box->setCheckState(Qt::Unchecked);
    if (i == 1) m_ui.measuredistance1box->setCheckState(Qt::Unchecked);
    if (i == 2) m_ui.measuredistance2box->setCheckState(Qt::Unchecked);

    this->angleWidget[i] = vtkSmartPointer<vtkAngleWidget>::New();
    this->angleWidget[i]->SetInteractor(this->riw[i]->GetResliceCursorWidget()->GetInteractor());
    this->angleWidget[i]->SetPriority(this->riw[i]->GetResliceCursorWidget()->GetPriority() + 0.01);
    this->riw[i]->GetMeasurements()->AddItem(this->angleWidget[i]);
    this->angleWidget[i]->CreateDefaultRepresentation();
    this->angleWidget[i]->EnabledOn();*/
}

void CVSegmentationView::addDistanceMeasurementToView(int i)
{
    /*removeAngleMeasurementToView(i);
    removeDistanceMeasurementToView(i);

    this->distanceWidget[i] = vtkSmartPointer<vtkDistanceWidget>::New();
    this->distanceWidget[i]->SetInteractor(this->riw[i]->GetResliceCursorWidget()->GetInteractor());

    this->distanceWidget[i]->SetPriority(this->riw[i]->GetResliceCursorWidget()->GetPriority() + 0.01);

    vtkSmartPointer<vtkPointHandleRepresentation2D> handleRep = vtkSmartPointer<vtkPointHandleRepresentation2D>::New();
    vtkSmartPointer<vtkDistanceRepresentation2D> distanceRep = vtkSmartPointer<vtkDistanceRepresentation2D>::New();
    distanceRep->SetHandleRepresentation(handleRep);
    this->distanceWidget[i]->SetRepresentation(distanceRep);
    distanceRep->InstantiateHandleRepresentation();
    distanceRep->GetPoint1Representation()->SetPointPlacer(riw[i]->GetPointPlacer());
    distanceRep->GetPoint2Representation()->SetPointPlacer(riw[i]->GetPointPlacer());

    this->riw[i]->GetMeasurements()->AddItem(this->distanceWidget[i]);

    this->distanceWidget[i]->CreateDefaultRepresentation();
    this->distanceWidget[i]->EnabledOn();
    if (i == 0) m_ui.measureangle0box->setCheckState(Qt::Unchecked);
    if (i == 1) m_ui.measureangle1box->setCheckState(Qt::Unchecked);
    if (i == 2) m_ui.measureangle2box->setCheckState(Qt::Unchecked);*/
}

void CVSegmentationView::aneurysmVisibilityChanged(int state)
{
    if (m_ui.aneurysmVisibilityBox->isChecked())
    {
        /*this->aneurysmActor->GetProperty()->SetOpacity(VESSELSURFACEOPACITY);
        this->aneurysmActor->VisibilityOn();*/
    }
    else
    {
       // this->aneurysmActor->VisibilityOff();
    }
    //m_ui.view4->GetRenderWindow()->Render();
}

void CVSegmentationView::displayAbout()
{
    QMessageBox msgBox;
    msgBox.setText(QString::fromUtf8("<p align='left'>三维图像处理软件<p align='left'>规格型号 3DPathFinder <p align='left'>完整版本V1.0.0.0<p align='left'>发布版本V1.0<p align='left'>强联智创(北京)科技有限公司版权</p>"));
    //msgBox.setText(QString::fromUtf8("<p align='center'>3DPathFinder (V1.0)<br>强联智创（北京）科技有限公司版权所有</p>"));
    msgBox.exec();
}

void CVSegmentationView::displayVTKLicense()
{
    QMessageBox msgBox;
    msgBox.setText("VTK License: \n\nCopyright (c) 1993-2008 Ken Martin, Will Schroeder, Bill Lorensen All rights reserved.\n\nRedistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met: \n\nRedistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. \n\nRedistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution. \n\nNeither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names of any contributors may be used to endorse or promote products derived from this software without specific prior written permission. \n\nTHIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.");
    msgBox.exec();
}

void CVSegmentationView::addSeedPoint()
{
    m_seedPoints->InsertPoint(GetMitkRenderWindowInterface()->GetMitkStdMultiWidget()->GetCrossPosition());
    m_seedPointsNode->Modified();
}

void CVSegmentationView::generateExtensions()
{
    this->algorithm->generateExtensions();
     //this->surfaceMapper->SetInputData(this->algorithm->generateExtensions());
     //this->algorithm->setCurrentSurfaceShowing(this->algorithm->getCurrentSurface());
     //this->surfaceActor->GetProperty()->SetColor(1, 0, 0);
     //this->surfaceActor->GetProperty()->SetOpacity(VESSELSURFACEOPACITY);
     //planeWidget[0]->GetDefaultRenderer()->AddActor(this->surfaceActor);
     //this->surfaceActor->VisibilityOn();
     //m_ui.view4->GetRenderWindow()->Render();
     //m_ui.vesselVisibilityBox->setCheckState(Qt::Checked);

     /*this->algorithm->setCurrentSurfaceShowing(this->algorithm->getCurrentSurface());
      this->surfaceActor->GetProperty()->SetColor(1, 0, 0);
     m_ui.vesselVisibilityBox->setCheckState(Qt::Checked);*/
}

void CVSegmentationView::generateMould()
{
    /* this->mouldData = this->algorithm->generateMould(this->needleContourRepresentation);

     this->surfaceMapper->SetInputData(this->mouldData);
     this->surfaceActor->GetProperty()->SetColor(1, 0, 0);
     this->surfaceActor->GetProperty()->SetOpacity(VESSELSURFACEOPACITY);
     planeWidget[0]->GetDefaultRenderer()->AddActor(this->surfaceActor);
 */
    m_ui.saveMouldButton->setEnabled(true);
}

void CVSegmentationView::saveMould()
{
    vtkSTLWriter * writeSTL = vtkSTLWriter::New();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Mould As:"), "mould.stl", tr("*.stl"));

    writeSTL->SetFileName(fileName.toStdString().c_str());
    writeSTL->SetInputData(dynamic_cast<mitk::Surface*>(GetDataStorage()->GetNamedNode("mould")->GetData())->GetVtkPolyData());

    writeSTL->Write();
}

void CVSegmentationView::generateNeedle()
{
    vtkSmartPointer<vtkPolyData> needleData = this->algorithm->generateNeedle();

    if (needleData->GetNumberOfPoints() == 0)
    {
        QMessageBox msgBox;
        //msgBox.setText("Unable to calculate needle path.");
        msgBox.setText(QString::fromUtf8("未能找到导管路径"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setButtonText(QMessageBox::Ok, QString::fromUtf8("确定"));
        msgBox.setWindowTitle(QString::fromUtf8("生成导管"));
        msgBox.exec();
    }
    ImportVtkPolyData(needleData, "needle")->SetColor(0.0,0.0,1.0);
    m_ui.generateMouldButton->setEnabled(true);
}

void CVSegmentationView::loadSurface()
{
    //char fileToBeLoad[100];
    //memset(fileToBeLoad, 0x0, sizeof(fileToBeLoad));
    //strcpy(fileToBeLoad, this->fileloadname.toStdString().c_str());
    //char* tmpFile = fileToBeLoad + strlen(fileToBeLoad);
    //while ('/' != *tmpFile)
    //{
    //    tmpFile--;
    //}
    //tmpFile++;
    //*tmpFile = '\0';
    //strcat(fileToBeLoad, "surface.usd");
    ////QString fileName = QFileDialog::getOpenFileName(this, tr("Open Surface:"), "surface.usd", tr("*.usd"));
    //QString fileName = QFileDialog::getOpenFileName(this, tr("Open Surface:"), fileToBeLoad, tr("*.usd"));
    //if (fileName.isEmpty())
    //{
    //    return;
    //}
    //string filepath = fileName.toStdString().c_str();
    //char filePutPath[260];
    //strcpy(filePutPath, filepath.c_str());

    //CMyCab umc;
    //bool isUnPackSuccessful = umc.DoUnCAB(filePutPath);
    //if (!isUnPackSuccessful)
    //{
    //    QMessageBox msgbox;
    //    msgbox.setText(QString::fromUtf8("选择的表面文件为异常表面文件"));
    //    msgbox.setIcon(QMessageBox::Warning);
    //    msgbox.setButtonText(QMessageBox::Ok, QString::fromUtf8("确定"));
    //    msgbox.setWindowTitle(QString::fromUtf8("加载表面文件"));
    //    msgbox.exec();
    //    return;
    //}

    //char tmpPathName0[260];
    //memset(tmpPathName0, 0x0, sizeof(tmpPathName0));
    //strcpy(tmpPathName0, filePutPath);
    //char* tmpC = tmpPathName0 + strlen(tmpPathName0);
    //while ('/' != *tmpC)
    //{
    //    tmpC--;
    //}
    //tmpC++;
    //*tmpC = '\0';
    //char fil[260];
    //strcpy(fil, tmpPathName0);
    //strcat(fil, "/file.txt");
    //std::ifstream fin(fil);
    //boost::archive::text_iarchive ia(fin); // 文本的输入归档类
    //pInfo newobj;
    //ia >> newobj; // 恢复到newobj对象
    //fin.close();

    //char stlpath[260];
    //strcpy(stlpath, tmpPathName0);
    //strcat(stlpath, "/surface.stl");
    //if (newobj.patientID == this->algorithm->patientInfo.patientID && newobj.modality == this->algorithm->patientInfo.modality && newobj.contentTime == this->algorithm->patientInfo.contentTime)
    //{
    //    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    //    reader->SetFileName(stlpath);
    //    reader->Update();
    //    this->algorithm->setCurrentSurfaceShowing(reader->GetOutput());
    //    this->surfaceMapper->SetInputData(reader->GetOutput());
    //    this->surfaceActor->GetProperty()->SetColor(1, 0, 0);
    //    this->surfaceActor->GetProperty()->SetOpacity(VESSELSURFACEOPACITY);
    //    this->surfaceActor->VisibilityOn();
    //    planeWidget[0]->GetDefaultRenderer()->AddActor(this->surfaceActor);
    //    m_ui.view4->GetRenderWindow()->Render();
    //    remove(stlpath);
    //    remove(fil);
    //}
    //else
    //{
    //    QMessageBox msgBox;
    //    //msgBox.setText("The selected surface file doesn't match the current data. Please adjust it.");
    //    msgBox.setText(QString::fromUtf8("选择的表面文件与当前数据不匹配，请重新选择表面文件"));
    //    //msgBox.setIcon(QMessageBox::Warning);
    //    msgBox.setButtonText(QMessageBox::Ok, QString::fromUtf8("确定"));
    //    msgBox.setWindowTitle(QString::fromUtf8("加载表面文件"));
    //    msgBox.exec();
    //    remove(stlpath);
    //    remove(fil);
    //    return;
    //}

}

void CVSegmentationView::vesselVisibilityChanged(int state)
{
    GetDataStorage()->GetNamedNode("vessel")->SetVisibility(m_ui.vesselVisibilityBox->isChecked());
}

void CVSegmentationView::slotExit()
{
    qApp->exit();
}

void CVSegmentationView::sliceVisibilityChanged(int state)
{
    GetMitkRenderWindowInterface()->GetMitkStdMultiWidget()->SetWidgetPlanesVisibility(state);
}

void CVSegmentationView::setStartPoint(bool checked)
{
    m_pPointList->AddPoint(checked);
    if (checked)
    {
        m_pPointList->SetPointSetNode(m_startPointsNode);

        m_ui.setEndButton->setChecked(false);
        m_ui.addAneurysmPointButton->setChecked(false);
        m_ui.setAneurysmLocationPoint1Button->setChecked(false);
        m_ui.setAneurysmLocationPoint2Button->setChecked(false);
    }
}

void CVSegmentationView::setEndPoint(bool checked)
{
    m_pPointList->AddPoint(checked);
    if (checked)
    {
        m_pPointList->SetPointSetNode(m_endPointsNode);

        m_ui.setStartButton->setChecked(false);
        m_ui.addAneurysmPointButton->setChecked(false);
        m_ui.setAneurysmLocationPoint1Button->setChecked(false);
        m_ui.setAneurysmLocationPoint2Button->setChecked(false);

    }
    
}  

void CVSegmentationView::addAneurysmPoint(bool checked)
{
    m_pPointList->AddPoint(checked);
    if (checked)
    {
        m_pPointList->SetPointSetNode(m_aneurysmPointsNode);

        m_ui.setEndButton->setChecked(false);
        m_ui.setStartButton->setChecked(false);
        m_ui.setAneurysmLocationPoint1Button->setChecked(false);
        m_ui.setAneurysmLocationPoint2Button->setChecked(false);
    }
}

void CVSegmentationView::setAneurysmLocationPoint1(bool checked)
{
    m_pPointList->AddPoint(checked);
    if (checked)
    {
        m_pPointList->SetPointSetNode(m_aneurysmLocationPoint1Node);

        m_ui.setEndButton->setChecked(false);
        m_ui.setStartButton->setChecked(false);
        m_ui.addAneurysmPointButton->setChecked(false);
        m_ui.setAneurysmLocationPoint2Button->setChecked(false);
    }
}

void CVSegmentationView::setAneurysmLocationPoint2(bool checked)
{
    m_pPointList->AddPoint(checked);
    if (checked)
    {
        m_pPointList->SetPointSetNode(m_aneurysmLocationPoint2Node);

        m_ui.setEndButton->setChecked(false);
        m_ui.setStartButton->setChecked(false);
        m_ui.addAneurysmPointButton->setChecked(false);
        m_ui.setAneurysmLocationPoint1Button->setChecked(false);
    }
}

void CVSegmentationView::generateRectSurface()
{
    this->algorithm->generateRectSurface();
    //this->surfaceMapper->SetInputData(this->algorithm->generateRectSurface());
    this->algorithm->setCurrentSurfaceShowing(this->algorithm->getCurrentSurface());
    //this->surfaceActor->GetProperty()->SetColor(1, 0, 0);
    //this->surfaceActor->GetProperty()->SetOpacity(VESSELSURFACEOPACITY);
   // planeWidget[0]->GetDefaultRenderer()->AddActor(this->surfaceActor);
   // this->surfaceActor->VisibilityOn();
    //m_ui.view4->GetRenderWindow()->Render();
   // m_ui.vesselVisibilityBox->setCheckState(Qt::Checked);
    GetDataStorage()->GetNamedNode("vessel")->SetVisibility(true);
    m_ui.vesselVisibilityBox->setCheckState(Qt::Checked);
}

void CVSegmentationView::needlePathVisibilityChanged(int state)
{
    /*if (m_ui.needlePathVisibilityBox->isChecked())
    {
        this->needleContourWidget->SetEnabled(true);
    }
    else
    {
        this->needleContourWidget->SetEnabled(false);
    }
    m_ui.view4->GetRenderWindow()->Render();*/
    GetDataStorage()->GetNamedNode("needle")->SetVisibility(state);
}

void CVSegmentationView::removeSeedPoint()
{
    m_seedPoints->Clear();
}

void CVSegmentationView::removeSurface()
{
    GetDataStorage()->Remove(GetDataStorage()->GetNamedNode("vessel"));
}

void CVSegmentationView::resetSeedPoints()
{
    this->algorithm->resetSeed();
    m_seedPoints->Clear();
    toggleUIState();
}

void CVSegmentationView::resetViews()
{
    GetMitkRenderWindowInterface()->ResetCrossHair();
}


void CVSegmentationView::saveSurface()
{
    //vtkSTLWriter * writeSTL = vtkSTLWriter::New();
    //char fileToBeSave[100];
    //memset(fileToBeSave, 0x0, sizeof(fileToBeSave));
    //strcpy(fileToBeSave, this->fileloadname.toStdString().c_str());
    //char* tmpFile = fileToBeSave + strlen(fileToBeSave);
    //while ('/' != *tmpFile)
    //{
    //    tmpFile--;
    //}
    //tmpFile++;
    //*tmpFile = '\0';
    //strcat(fileToBeSave, "surface.usd");
    //QString fileName = QFileDialog::getSaveFileName(this, tr("Save Surface As:"), fileToBeSave, tr("*.usd"));
    //if (fileName.isEmpty())
    //{
    //    return;
    //}
    //QString path;
    //QDir dir;
    //path = dir.currentPath();
    //char stlfilePath[260];
    //strcpy(stlfilePath, path.toStdString().c_str());
    //strcat(stlfilePath, "/surface.stl");
    //writeSTL->SetFileName(stlfilePath);
    //writeSTL->SetInputData(this->algorithm->getCurrentSurface());
    //writeSTL->Write();

    //std::ofstream fout("file.txt");// 把对象写到file.txt文件中
    //boost::archive::text_oarchive oa(fout); // 文本的输出归档类，使用一个ostream来构造
    //pInfo obj;
    //obj.patientID = algorithm->patientInfo.patientID;
    //obj.modality = algorithm->patientInfo.modality;
    //obj.studyID = algorithm->patientInfo.studyID;
    //obj.contentDate = algorithm->patientInfo.contentDate;
    //obj.contentTime = algorithm->patientInfo.contentTime;
    ////obj.institute = algorithm->patientInfo.institute;
    ////obj.medicalRecordLocator = algorithm.patientInfo.medicalRecordLocator;
    ////obj.patientName = algorithm.patientInfo.patientName;
    //obj.seriesNumber = algorithm->patientInfo.seriesNumber;
    ////obj.surface = algorithm.getCurrentSurface();
    //oa << obj; // 保存obj对象
    //fout.close();// 关闭文件

    //string filepath = path.toStdString().c_str();
    //char filePutPath[260];
    //strcpy(filePutPath, fileName.toStdString().c_str());
    ////strcat(filePutPath, "/surface.unionstrong");
    //CMyCab mc;
    //mc.SetOutPutFile(filePutPath);
    //char stl[260];
    //strcpy(stl, filepath.c_str());
    //strcat(stl, "/surface.stl");
    //char fil[260];
    //strcpy(fil, filepath.c_str());
    //strcat(fil, "/file.txt");
    //mc.AddFile(fil);
    //mc.AddFile(stl);
    //mc.DoMakeCAB();
    //remove(fil);
    //remove(stl);
}