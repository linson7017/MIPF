#include "NDIView.h"
#include "iqf_main.h"
#include "Res/R.h"
#include "Utils/variant.h"


//mitk
#include "mitkRenderingManager.h"
#include "mitkDataNode.h"
#include "mitkSurface.h"

//vtk
#include "vtkSphereSource.h"

//mitkMain
#include "MitkMain/IQF_MitkDataManager.h"
#include "MitkMain/IQF_MitkRenderWindow.h"

#include "newmatap.h"

NDIView::NDIView(QF::IQF_Main* pMain) :PluginView(pMain), m_robotArrived(false)
{
    m_pMain->Attach(this);

    SphereFit sf;
    sf.InsertPoint(QVector3D(14.2651,  -296.672, 588.538));
    sf.InsertPoint(QVector3D(52.7645,  -268.218, 574.207));
    sf.InsertPoint(QVector3D(-16.2902, -295.397, 588.781));
    sf.InsertPoint(QVector3D(17.1993,  -246.208, 574.911));
    sf.InsertPoint(QVector3D(75.6449,  -249.325, 552.671));

    /*sf.InsertPoint(QVector3D(-14.0844,-9.96628,-137.891));
    sf.InsertPoint(QVector3D(-52.5838,-38.4207,-123.561));
    sf.InsertPoint(QVector3D(16.4709,-11.2415,-138.135));
    sf.InsertPoint(QVector3D(-17.0186,-60.4308,-124.265));
    sf.InsertPoint(QVector3D(-75.4642,-57.3139,-102.024));*/
    QVector3D center;
    double radius;
    sf.Fit(center, radius);


    qDebug() << "Center:" << center;
    qDebug() << "Radius:" << radius;
}

void NDIView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "MITK_MESSAGE_MULTIWIDGET_INIT") == 0)
    {
        //do what you want for the message
        IQF_MitkRenderWindow* pRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
        if (pRenderWindow)
        {
            wdgtMMIPointSet = new QmitkPointListWidget;
            m_MultiView = pRenderWindow->GetMitkStdMultiWidget();
            m_MMIPointSet = mitk::PointSet::New();
            m_MMIPointSetNode = mitk::DataNode::New();
            m_MMIPointSetNode->SetData(m_MMIPointSet);
            m_MMIPointSetNode->SetName("MMI Point Set");
            m_MMIPointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true));
            m_MMIPointSetNode->SetProperty("layer", mitk::IntProperty::New(1024));

            // add the pointset to the data storage (for rendering and access by other modules)
            IQF_MitkDataManager* pMitkDataStorage = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr("QF_MitkMain_DataManager");
            pMitkDataStorage->GetDataStorage()->Add(m_MMIPointSetNode);
            // tell the GUI widget about the point set
            wdgtMMIPointSet->SetPointSetNode(m_MMIPointSetNode);
            wdgtMMIPointSet->SetMultiWidget(m_MultiView);
        }
         
    }
    else if (strcmp(szMessage, "main.FindX") == 0)
    {
        double maxStep, minStep;
        maxStep = 10;
        minStep = 0.1;
        while (true)
        {
            VarientMap vm;
            QString step;
            step.setNum(maxStep);
            m_pMain->SendMessageQf("main.MoveX", 1, &vm);
            
        }
    }
    else if (strcmp(szMessage, "Robot.Move")==0)
    {
        QVector3D* positionValue = (QVector3D*)(pValue);
        QString temp;
        currentRobotPointX->setText(temp.setNum(positionValue->x()));
        currentRobotPointY->setText(temp.setNum(positionValue->y()));
        currentRobotPointZ->setText(temp.setNum(positionValue->z()));
        m_robotArrived = true;

        currentNdiPointX->setText(temp.setNum(m_currentNdiPosition.x()));
        currentNdiPointY->setText(temp.setNum(m_currentNdiPosition.y()));
        currentNdiPointZ->setText(temp.setNum(m_currentNdiPosition.z()));

    }
    else if (strcmp(szMessage, "main.GetNeedlePosition") == 0)
    {
        m_robotPoints.push_back(QVector3D(currentRobotPointX->text().toDouble(), currentRobotPointY->text().toDouble(), currentRobotPointZ->text().toDouble()));
        m_ndiPoints.push_back(m_currentNdiPosition);

        QString text;
        int  index = m_robotPoints.size();
        text = QString("%0: (%1, %2, %3);(%4, %5, %6)").arg(index, 3)
            .arg(currentRobotPointX->text())
            .arg(currentRobotPointY->text())
            .arg(currentRobotPointZ->text())
            .arg(m_currentNdiPosition.x())
            .arg(m_currentNdiPosition.y())
            .arg(m_currentNdiPosition.z());
        lstwgtSubjectPointSet->addItem(text);
    }
    else if (strcmp(szMessage, "main.Registrate") == 0)
    {
        WxLandMarkRegistration registration;
        registration.ComputeRegistration(m_ndiPoints, m_robotPoints);
        QTextEdit* textEdit = (QTextEdit*)m_pR->getObjectFromGlobalMap("main.RegistrationMatrix");
        if (textEdit)
        {
            
            QMatrix4x4 m = registration.GetRegistrationQtMatrix();
            m_registrationMatrix = m;
            m_pMain->SendMessageQf("main.RegistrationMatrixCaculated", 0, &m_registrationMatrix);
            //m.setToIdentity();
            QString registrationMatrixText = QString("%1, %2, %3, %4 \n%5, %6, %7, %8 \n%9, %10, %11, %12 \n%13, %14, %15, %16 \n")
                .arg(m(0, 0)).arg(m(0, 1)).arg(m(0, 2)).arg(m(0, 3))
                .arg(m(1, 0)).arg(m(1, 1)).arg(m(1, 2)).arg(m(1, 3))
                .arg(m(2, 0)).arg(m(2, 1)).arg(m(2, 2)).arg(m(2, 3))
                .arg(m(3, 0)).arg(m(3, 1)).arg(m(3, 2)).arg(m(3, 3));
            textEdit->setText(registrationMatrixText);
        }
    }
    else if (strcmp(szMessage, "main.RecordTargetPosition") == 0)
    {
        m_targetPosition.setX(currentRobotPointX->text().toDouble());
        m_targetPosition.setY(currentRobotPointY->text().toDouble());
        m_targetPosition.setZ(currentRobotPointZ->text().toDouble());

        qDebug() << "Target NDI Position: " << m_targetPosition;
        QLineEdit* lineEdit = (QLineEdit*)m_pR->getObjectFromGlobalMap("main.TargetPosition");
        if (lineEdit)
        {
            QString targetPositionText = QString("%1, %2, %3").arg(m_currentNdiPosition.x()).arg(m_currentNdiPosition.y()).arg(m_currentNdiPosition.z());
            lineEdit->setText(targetPositionText);
        }
    }
    else if (strcmp(szMessage, "main.BackToTargetPosition") == 0)
    {
        m_pMain->SendMessageQf("main.MoveToPosition", 0, &m_targetPosition);
    }
    
}

mitk::DataStorage::Pointer NDIView::GetDataStorage()
{
    IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    if (pDataManager)
    {
        return pDataManager->GetDataStorage();
    }
    else
    {
        return NULL;
    }
}


void NDIView::InitResource(R* pR)
{
    m_pR = pR;
    //init
    m_Timer = NULL;
    m_NDIComm = new WxNDIPolarisVega;
    m_LandmarkRegistration = new WxLandMarkRegistration;
    m_AutoExtractLandMark = new WxAutoExtractLandMarkFromMR;
    m_PunctureNeedleMatrix.setToIdentity();
    m_PunctureNeedleAxisMatrix.setToIdentity();
   // m_PunctureNeedleAxisMatrix.rotate(-180, QVector3D(0, 1, 0));
   // m_PunctureNeedleAxisMatrix.rotate(180, QVector3D(0, 0, 1));
    m_PunctureNeedleRegistrationOffsetMatrix.setToIdentity();
    
    m_ReferenceMatrix.setToIdentity();
    m_RegistrationMatrix.setToIdentity();
    m_SubjectPointSet.clear();
    m_ImageLandmarkList.clear();
    m_SubjectLandmarkList.clear();

    pbtnAutoGetImagePoint = new QPushButton("Threshold Matching Recognize Mark Point");
    pbtnTemplateMatch = new QPushButton("Template Matching Recognize Mark Point");
    pbtnClearSubjectPoint = new QPushButton("Clear");
    pbtnSelectSubjectPoint = new QPushButton("Collect");
    pbtnPauseTracker = new QPushButton("Pause");
    pbtnPauseTracker->setCheckable(true);
    pbtnStartTracker = new QPushButton("Start");
    pbtnStartTracker->setCheckable(true);


    //
    variant* v = (variant*)m_pR->getConfigResource("TipOffsetX");
    QString tempTxt;
    if (v)
    {
        tipOffsetX = new QLineEdit(tempTxt.setNum(v->getDouble()));
    }
    

    v = (variant*)m_pR->getConfigResource("TipOffsetY");
    if (v)
    {
        tipOffsetY = new QLineEdit(tempTxt.setNum(v->getDouble()));
    }

    v = (variant*)m_pR->getConfigResource("TipOffsetZ");
    if (v)
    {
        tipOffsetZ = new QLineEdit(tempTxt.setNum(v->getDouble()));
    }
    fitCenterBtn = new QPushButton("Fit Center");
    fitCenterBtn->setCheckable(true);
    connect(fitCenterBtn, SIGNAL(clicked(bool)), this, SLOT(OnFitCenter(bool)));
    varianceValue = new QLineEdit;
    varianceValue->setReadOnly(true);

    if (!wdgtMMIPointSet)
    {
        wdgtMMIPointSet = new QmitkPointListWidget;
    }
    wdgtMMIPointSet->setMinimumHeight(200);
    
    lstwgtSubjectPointSet = new QListWidget;
    m_SelecetedImage = new QmitkDataStorageComboBox;
    txtedtRegistrationMatrix = new QTextEdit;
    dspbxPixelUnit = new QDoubleSpinBox;
    dspbxThresholdMax = new QDoubleSpinBox;
    dspbxThresholdMin = new QDoubleSpinBox;

    QVBoxLayout* mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    mainLayout->addWidget(m_SelecetedImage);

    {
        QHBoxLayout* layout = new QHBoxLayout;
        layout->addWidget(pbtnStartTracker);
        layout->addWidget(pbtnPauseTracker);
        mainLayout->addLayout(layout);
    }

    {
        QHBoxLayout* layout = new QHBoxLayout;
        layout->addWidget(tipOffsetX);
        layout->addWidget(tipOffsetY);
        layout->addWidget(tipOffsetZ);

        QVBoxLayout* vLayout = new QVBoxLayout;
        vLayout->addLayout(layout);
        vLayout->addWidget(varianceValue);
        vLayout->addWidget(fitCenterBtn);
        mainLayout->addLayout(vLayout);
    }

    {
        currentRobotPointX = new QLineEdit;
        currentRobotPointY = new QLineEdit;
        currentRobotPointZ = new QLineEdit;

        QHBoxLayout* layout = new QHBoxLayout;
        layout->addWidget(currentRobotPointX);
        layout->addWidget(currentRobotPointY);
        layout->addWidget(currentRobotPointZ);
        mainLayout->addLayout(layout);
    }

    {
        currentNdiPointX = new QLineEdit;
        currentNdiPointY = new QLineEdit;
        currentNdiPointZ = new QLineEdit;

        QHBoxLayout* layout = new QHBoxLayout;
        layout->addWidget(currentNdiPointX);
        layout->addWidget(currentNdiPointY);
        layout->addWidget(currentNdiPointZ);
        mainLayout->addLayout(layout);
    }

   /* {
        QGroupBox* box = new QGroupBox("Image Mark Point");
        QVBoxLayout* vlayout = new QVBoxLayout;
        vlayout->addWidget(wdgtMMIPointSet);

        QHBoxLayout* hlayout = new QHBoxLayout;
        hlayout->addWidget(dspbxPixelUnit);
        hlayout->addWidget(dspbxThresholdMin);
        hlayout->addWidget(dspbxThresholdMax);
        vlayout->addLayout(hlayout);

        vlayout->addWidget(pbtnAutoGetImagePoint);
        vlayout->addWidget(pbtnTemplateMatch);

        box->setLayout(vlayout);
        mainLayout->addWidget(box);
    }*/

    {
        QGroupBox* box = new QGroupBox("Patient Mark Point");
        QVBoxLayout* vlayout = new QVBoxLayout;
        vlayout->addWidget(lstwgtSubjectPointSet);

        //QHBoxLayout* hlayout = new QHBoxLayout;
        //hlayout->addWidget(pbtnSelectSubjectPoint);
        //hlayout->addWidget(pbtnClearSubjectPoint);

        box->setLayout(vlayout);
        mainLayout->addWidget(box);
    }

    /*{
        QGroupBox* box = new QGroupBox("Registration Results");
        QVBoxLayout* vlayout = new QVBoxLayout;
        vlayout->addWidget(txtedtRegistrationMatrix);

        box->setLayout(vlayout);
        mainLayout->addWidget(box);
    }*/
    mainLayout->addStretch(100);

    connect(pbtnStartTracker, SIGNAL(clicked(bool)), this, SLOT(OnStartTracker(bool)));
    connect(pbtnPauseTracker, SIGNAL(clicked(bool)), this, SLOT(OnPauseTracker(bool)));
   // connect(pbtnSelectSubjectPoint, SIGNAL(clicked()), this, SLOT(OnSelectSubjectPoint()));
   // connect(pbtnClearSubjectPoint, SIGNAL(clicked()), this, SLOT(OnClearSubjectPoint()));
   // connect(pbtnAutoGetImagePoint, SIGNAL(clicked()), this, SLOT(OnAutoGetImagePoint()));
  
    //connect(pbtnTemplateMatch, SIGNAL(clicked()), this, SLOT(OnTemplateMatch()));

    m_SelecetedImage->SetPredicate(mitk::NodePredicateDataType::New("Image"));
    m_SelecetedImage->SetDataStorage(this->GetDataStorage());
}

void NDIView::SetFocus()
{
    if (1)
    {
        mitk::DataNode* tempNode;
        if (m_MMIPointSet.IsNull())  m_MMIPointSet = mitk::PointSet::New();
        if (m_MMIPointSetNode.IsNull())
        {
            m_MMIPointSetNode = mitk::DataNode::New();
            m_MMIPointSetNode->SetData(m_MMIPointSet);
            m_MMIPointSetNode->SetName("DIPSNode");
            m_MMIPointSetNode->SetProperty("label", mitk::StringProperty::New("A"));
            m_MMIPointSetNode->SetProperty("color", mitk::ColorProperty::New(0.0f, 1.0f, 0.0f));
            m_MMIPointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true));
        }
        tempNode = NULL;
        tempNode = this->GetDataStorage()->GetNamedNode("DIPSNode");
        if (tempNode == NULL)
        {
            this->GetDataStorage()->Add(m_MMIPointSetNode);
            wdgtMMIPointSet->SetPointSetNode(m_MMIPointSetNode);
        }
    }
}

void NDIView::LoadNavigationToolsModel()
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    mitk::DataNode* m_TempNode;
    mitk::DataNodeFactory::Pointer nodeReader = mitk::DataNodeFactory::New();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    nodeReader->SetFileName("VegaProbe.stl");
    nodeReader->Update();

    m_PunctureNeedleNode = nodeReader->GetOutput();
    m_PunctureNeedleNode->SetName("PunctureNeedle");
    //m_PunctureNeedleNode->SetProperty("visible", mitk::BoolProperty::New(false));
    //m_PunctureNeedleNode->SetProperty("helper object", mitk::BoolProperty::New(true) );
    m_PunctureNeedleNode->SetProperty("color", mitk::ColorProperty::New(0.0f, 1.0f, 0.0f));
    m_PunctureNeedleNode->SetVisibility(true);

    m_TempNode = NULL;
    m_TempNode = this->GetDataStorage()->GetNamedNode("PunctureNeedle");
    if (m_TempNode == NULL)
    {
        this->GetDataStorage()->Add(m_PunctureNeedleNode);
    }
    else
    {
        this->GetDataStorage()->Remove(m_TempNode);
        this->GetDataStorage()->Add(m_PunctureNeedleNode);
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void NDIView::RemoveNavigationToolsModel()
{

    mitk::DataNode* m_TempNode;

    m_TempNode = NULL;
    m_TempNode = this->GetDataStorage()->GetNamedNode("PunctureNeedle");
    if (m_TempNode != NULL)
    {
        m_PunctureNeedleNode->SetMapper(mitk::BaseRenderer::Standard2D, NULL);
        this->GetDataStorage()->Remove(m_TempNode);
    }

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void NDIView::OnStartTracker(bool start)
{
    if (start)
    {
        m_MultiView->SetWidgetPlanesVisibility(false);
        //Get Config IpAddress
        const variant* v = m_pR->getConfigResource("IPAddress"); 
        QString ipAddress = "169.254.1.3";
        if (v)
        {
            ipAddress = v->getString();        
        }
        //Get Config Port
        v = m_pR->getConfigResource("Port");
        int port = 8765;
        if (v)
        {
            port = v->getInt();
        }

        if (m_NDIComm->InitalizePolarisVega(ipAddress, port))
        {
            m_NDIComm->AddTool("VegaProbe.rom", QString("S"));
            m_NDIComm->AddTool("8700449.rom", QString("D"));
            m_NDIComm->StartTrack();

            LoadNavigationToolsModel();
            if (m_Timer == NULL)
            {
                m_Timer = new QTimer(this);
                m_Timer->start(50);
                connect(m_Timer, SIGNAL(timeout()), this, SLOT(OnUpdateNavigationData()));
                pbtnPauseTracker->setEnabled(true);
                pbtnSelectSubjectPoint->setEnabled(true);
            }
        }
        else
        {
            pbtnStartTracker->setChecked(false);
            pbtnSelectSubjectPoint->setEnabled(false);
        }
    }
    else
    {
        if (m_Timer != NULL)
        {
            disconnect(m_Timer, SIGNAL(timeout()), this, SLOT(OnUpdateNavigationData()));
            m_Timer->stop();
            m_Timer = NULL;
        }

        m_NDIComm->StopTrack();
        RemoveNavigationToolsModel();

        pbtnPauseTracker->setEnabled(false);
        pbtnSelectSubjectPoint->setEnabled(false);
    }
}

void NDIView::OnPauseTracker(bool pause)
{
    if (pause)
    {
        if (m_Timer != NULL)
        {
            disconnect(m_Timer, SIGNAL(timeout()), this, SLOT(OnUpdateNavigationData()));
            m_Timer->stop();
            m_Timer = NULL;
            pbtnStartTracker->setEnabled(false);
        }
    }
    else
    {
        if (m_Timer == NULL)
        {
            m_Timer = new QTimer(this);
            m_Timer->start(50);
            connect(m_Timer, SIGNAL(timeout()), this, SLOT(OnUpdateNavigationData()));
            pbtnStartTracker->setEnabled(true);
        }
    }
}

void NDIView::OnUpdateNavigationData()
{
    //update needle Matrix
    m_PunctureNeedleRegistrationOffsetMatrix.setToIdentity();
    m_PunctureNeedleRegistrationOffsetMatrix.translate(tipOffsetX->text().toDouble(), tipOffsetY->text().toDouble(), tipOffsetZ->text().toDouble());
    m_NDIComm->GetQMatrix4x4(0, m_ReferenceMatrix);
    m_NDIComm->GetQMatrix4x4(1, m_PunctureNeedleMatrix);    

    m_PunctureNeedleMatrix = m_PunctureNeedleMatrix*m_PunctureNeedleRegistrationOffsetMatrix;
    m_PunctureNeedleMatrix = m_PunctureNeedleMatrix*m_PunctureNeedleAxisMatrix;
    m_PunctureNeedleMatrix = m_ReferenceMatrix.inverted()*m_PunctureNeedleMatrix;
    m_NDIAuroraPointerPosition[0] = m_PunctureNeedleMatrix(0, 3);
    m_NDIAuroraPointerPosition[1] = m_PunctureNeedleMatrix(1, 3);
    m_NDIAuroraPointerPosition[2] = m_PunctureNeedleMatrix(2, 3);
    m_PunctureNeedleMatrix = m_RegistrationMatrix*m_PunctureNeedleMatrix;

    Update6DOFModelQMatrix4x4(m_PunctureNeedleNode->GetData(), m_PunctureNeedleMatrix);

    //Calculate Fit Point
    if (fitCenterBtn->isChecked())
    {
        QMatrix4x4 m = m_PunctureNeedleMatrix.inverted();
        QVector3D  tip(m(0, 3),
            m(1, 3),
            m(2, 3));
        m_fit.InsertPoint(tip);
    }

    //update calculated center position  
    if (m_TipsPositionSet.size()>20)
    {
        m_TipsPositionSet.pop_front();
    }
    else
    {
        double tipX, tipY, tipZ;
      //  m_NDIComm->GetCurrentTips(1, tipX, tipY, tipZ);
        QVector3D  tip(m_PunctureNeedleMatrix(0,3),
            m_PunctureNeedleMatrix(1, 3),
            m_PunctureNeedleMatrix(2, 3));
        //tip = m_PunctureNeedleMatrix*tip;
        m_TipsPositionSet.push_back(tip);
      //  qDebug() << "Current Tip: " << tip;
     //   qDebug() << "Tip Without Rotate And Translate: " << m_PunctureNeedleMatrix.inverted()*tip;
    }

   // m_pMain->SendMessageQf("Robot.Move", 0, &m_PunctureNeedleMatrix.column(3).toVector3D());
    if (!qFuzzyCompare(m_currentNdiPosition, m_PunctureNeedleMatrix.column(3).toVector3D()))
    {
        m_currentNdiPosition = m_PunctureNeedleMatrix.column(3).toVector3D();
        m_pMain->SendMessageQf("main.NDIPositionChanged", 0, &m_currentNdiPosition);
    }
    

    double variance;
    CaculateVariance(m_TipsPositionSet, variance);
    QString varianceText;
    varianceValue->setText(varianceText.setNum(variance));

    if (!m_RegistrationMatrix.isIdentity())
    {
        this->m_MultiView->GetRenderWindow1()->GetSliceNavigationController()->ReorientSlices(m_PunctureNeedleNode->GetData()->GetGeometry()->GetOrigin(), m_PunctureNeedleNode->GetData()->GetGeometry()->GetAxisVector(1));
        this->m_MultiView->GetRenderWindow2()->GetSliceNavigationController()->ReorientSlices(m_PunctureNeedleNode->GetData()->GetGeometry()->GetOrigin(), m_PunctureNeedleNode->GetData()->GetGeometry()->GetAxisVector(0));
        this->m_MultiView->GetRenderWindow3()->GetSliceNavigationController()->ReorientSlices(m_PunctureNeedleNode->GetData()->GetGeometry()->GetOrigin(), -m_PunctureNeedleNode->GetData()->GetGeometry()->GetAxisVector(2));
    }

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void NDIView::CaculateVariance(const QQueue<QVector3D>& positionSet, double& variance)
{
    int N = positionSet.size();
    if (N <=0)
    {
        return;
    }
    double sumX, sumY, sumZ = 0;
    for (int i = 0; i < N; i++)
    {
        sumX += positionSet[i].x();
        sumY += positionSet[i].y();
        sumZ += positionSet[i].z();
    }
    QVector3D center(sumX / N, sumY / N, sumZ / N);

    double sum = 0;
    for (int i = 0; i < N; i++)
    {
        double dis = center.distanceToPoint(positionSet[i]);
        sum += dis*dis;
    }
    variance = qSqrt(sum / N);
}

bool NDIView::Update6DOFModelQMatrix4x4(const mitk::BaseData* datanode, const QMatrix4x4 &transform)
{
    if (datanode != NULL)
    {
        vtkMatrix4x4*  m_Matrix4x4;
        m_Matrix4x4 = vtkMatrix4x4::New();
        m_Matrix4x4->Identity();

        for (int row = 0; row < 4; row++)
        {
            for (int column = 0; column < 4; column++)
            {
                m_Matrix4x4->SetElement(row, column, transform(row, column));
            }
        }

        datanode->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(m_Matrix4x4);
        datanode->Modified();
        return true;
    }
    else
    {
        return false;
    }
}

void NDIView::OnSelectSubjectPoint()
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    QString text;
    int  index = m_SubjectPointSet.size();
    text = QString("%0: (%1, %2, %3)").arg(index, 3)
        .arg(m_NDIAuroraPointerPosition[0], 0, 'f', 3)
        .arg(m_NDIAuroraPointerPosition[1], 0, 'f', 3)
        .arg(m_NDIAuroraPointerPosition[2], 0, 'f', 3);
    lstwgtSubjectPointSet->addItem(text);
    m_SubjectPointSet.push_back(m_NDIAuroraPointerPosition);

    if (m_SubjectPointSet.size() == 4)
    {
        m_ImageLandmarkList.clear();
        m_SubjectLandmarkList.clear();

        mitk::PointSet* tempps = dynamic_cast<mitk::PointSet*>(m_MMIPointSetNode->GetData());
        for (mitk::PointSet::PointsConstIterator pointsIterator = tempps->GetPointSet()->GetPoints()->Begin();
            pointsIterator != tempps->GetPointSet()->GetPoints()->End();
            ++pointsIterator)
        {
            m_ImageLandmarkList.push_back(QVector3D(pointsIterator.Value()[0], pointsIterator.Value()[1], pointsIterator.Value()[2]));
            MITK_INFO << pointsIterator.Value()[0] << pointsIterator.Value()[1] << pointsIterator.Value()[2];
        }

        for (int i = 0; i < m_SubjectPointSet.size(); i++)
        {
            m_SubjectLandmarkList.push_back(QVector3D(m_SubjectPointSet.at(i)[0], m_SubjectPointSet.at(i)[1], m_SubjectPointSet.at(i)[2]));
            MITK_INFO << m_SubjectPointSet.at(i)[0] << m_SubjectPointSet.at(i)[1] << m_SubjectPointSet.at(i)[2];
        }

        WxPointSetAutomaticPairing* autopair = new WxPointSetAutomaticPairing;
        autopair->PointSetAutomaticPariring(m_ImageLandmarkList, m_SubjectLandmarkList);
        QList<QVector3D> tmplist1 = autopair->GetPointSet1();
        QList<QVector3D> tmplist2 = autopair->GetPointSet2();

        WxLandMarkRegistration* landmark = new WxLandMarkRegistration;
        landmark->ComputeRegistration(tmplist1, tmplist2);
        vtkMatrix4x4* matrix = landmark->GetRegistrationVtkMatrix();
        double rmserror = landmark->GetRegistrationRMSError();

        m_RegistrationMatrix.setToIdentity();
        m_RegistrationMatrix(0, 0) = matrix->GetElement(0, 0);
        m_RegistrationMatrix(1, 0) = matrix->GetElement(1, 0);
        m_RegistrationMatrix(2, 0) = matrix->GetElement(2, 0);
        m_RegistrationMatrix(3, 0) = matrix->GetElement(3, 0);

        m_RegistrationMatrix(0, 1) = matrix->GetElement(0, 1);
        m_RegistrationMatrix(1, 1) = matrix->GetElement(1, 1);
        m_RegistrationMatrix(2, 1) = matrix->GetElement(2, 1);
        m_RegistrationMatrix(3, 1) = matrix->GetElement(3, 1);

        m_RegistrationMatrix(0, 2) = matrix->GetElement(0, 2);
        m_RegistrationMatrix(1, 2) = matrix->GetElement(1, 2);
        m_RegistrationMatrix(2, 2) = matrix->GetElement(2, 2);
        m_RegistrationMatrix(3, 2) = matrix->GetElement(3, 2);

        m_RegistrationMatrix(0, 3) = matrix->GetElement(0, 3);
        m_RegistrationMatrix(1, 3) = matrix->GetElement(1, 3);
        m_RegistrationMatrix(2, 3) = matrix->GetElement(2, 3);
        m_RegistrationMatrix(3, 3) = matrix->GetElement(3, 3);

        QString strRegMatrix = QString("Registration Matrix:\n"
            "%1 ,%2 ,%3 ,%4\n"
            "%5 ,%6 ,%7 ,%8\n"
            "%9 ,%10 ,%11 ,%12\n"
            "%13 ,%14 ,%15 ,%16\n"
            "%17")
            .arg(m_RegistrationMatrix(0, 0)).arg(m_RegistrationMatrix(0, 1)).arg(m_RegistrationMatrix(0, 2)).arg(m_RegistrationMatrix(0, 3))
            .arg(m_RegistrationMatrix(1, 0)).arg(m_RegistrationMatrix(1, 1)).arg(m_RegistrationMatrix(1, 2)).arg(m_RegistrationMatrix(1, 3))
            .arg(m_RegistrationMatrix(2, 0)).arg(m_RegistrationMatrix(2, 1)).arg(m_RegistrationMatrix(2, 2)).arg(m_RegistrationMatrix(2, 3))
            .arg(m_RegistrationMatrix(3, 0)).arg(m_RegistrationMatrix(3, 1)).arg(m_RegistrationMatrix(3, 2)).arg(m_RegistrationMatrix(3, 3))
            .arg(rmserror);
        txtedtRegistrationMatrix->setText(strRegMatrix);
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void NDIView::OnClearSubjectPoint()
{
    m_SubjectPointSet.clear();
    m_ImageLandmarkList.clear();
    m_SubjectLandmarkList.clear();
    lstwgtSubjectPointSet->clear();
}

void NDIView::OnAutoGetImagePoint()
{
    m_MMIPointSet->Clear();

    if (m_SelecetedImage->GetSelectedNode())
    {

        m_AutoExtractLandMark->FindModelPoints(m_SelecetedImage->GetSelectedNode(), dspbxPixelUnit->value(),
            dspbxThresholdMax->value(), dspbxThresholdMin->value(),
            0.0, 1.0, 0.0);

        std::vector<WxPoint3D> vecResult;
        vecResult = m_AutoExtractLandMark->GetResult();
        std::vector<WxPoint3D>::iterator itrOfOutputVector = vecResult.begin();

        int iPointIndex = 0;
        for (; itrOfOutputVector != vecResult.end(); itrOfOutputVector++)
        {
            mitk::Point3D pInsertPoint;
            pInsertPoint[0] = (*itrOfOutputVector)[0];
            pInsertPoint[1] = (*itrOfOutputVector)[1];
            pInsertPoint[2] = (*itrOfOutputVector)[2];

            m_MMIPointSet->InsertPoint(iPointIndex, pInsertPoint);
            iPointIndex++;
        }
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void NDIView::OnTemplateMatch()
{
    if (m_SelecetedImage->GetSelectedNode())
    {
        mitk::Image::Pointer originalmage = dynamic_cast<mitk::Image*>(m_SelecetedImage->GetSelectedNode()->GetData());

        mitk::Image::Pointer roiimage = mitk::Image::New();
        mitk::Point3D tempworld = m_MMIPointSet->GetPoint(0);
        mitk::Point3D tempindex;
        originalmage->GetGeometry()->WorldToIndex(tempworld, tempindex);
        QVector3D roiStart(tempindex.GetElement(0) - 10, tempindex.GetElement(1) - 10, tempindex.GetElement(2) - 10);
        QVector3D roiSize(20, 20, 20);
        m_AutoExtractLandMark->RegionOfInterestImageFilter(originalmage, roiimage, roiSize, roiStart);

        mitk::DataNode::Pointer roinode = mitk::DataNode::New();
        roinode->SetData(roiimage);
        roinode->SetName("ROI");
        roinode->SetProperty("color", mitk::ColorProperty::New(1.0f, 0.0f, 0.0f));
        this->GetDataStorage()->Add(roinode);

        int index = 0;
        mitk::PointSet* tempps = dynamic_cast<mitk::PointSet*>(m_MMIPointSetNode->GetData());
        for (mitk::PointSet::PointsConstIterator pointsIterator = tempps->GetPointSet()->GetPoints()->Begin();
            pointsIterator != tempps->GetPointSet()->GetPoints()->End();
            ++pointsIterator)
        {
            mitk::Point3D tempworld = pointsIterator.Value();
            mitk::Point3D tempindex;
            originalmage->GetGeometry()->WorldToIndex(tempworld, tempindex);
            roiStart = QVector3D(tempindex.GetElement(0) - 10, tempindex.GetElement(1) - 10, tempindex.GetElement(2) - 10);

            qDebug() << tempworld.GetElement(0) << "," << tempworld.GetElement(1) << "," << tempworld.GetElement(2) << "\n";
            qDebug() << roiStart.x() << "," << roiStart.y() << "," << roiStart.z() << "\n";

            mitk::Image::Pointer tempimage = mitk::Image::New();
            m_AutoExtractLandMark->RegionOfInterestImageFilter(originalmage, tempimage, roiSize, roiStart);

            qDebug() << m_AutoExtractLandMark->MeanSquaresImageToImage(roiimage, tempimage) << "\n";

            mitk::DataNode::Pointer tempnode = mitk::DataNode::New();
            tempnode->SetData(tempimage);
            tempnode->SetName(QString("ROI%1").arg(index).toStdString().c_str());
            tempnode->SetProperty("color", mitk::ColorProperty::New(1.0f, 1.0f, 0.0f));
            this->GetDataStorage()->Add(tempnode);
            index++;
        }
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void NDIView::OnFitCenter(bool start)
{
    if (start)
    {
        tipOffsetX->setText("0.0");
        tipOffsetY->setText("0.0");
        tipOffsetZ->setText("0.0");
        fitCenterBtn->setText("Collect Points ...");
        m_fit.Clear();
    }
    else
    {
        QVector3D center;
        double radius;
        m_fit.Fit(center, radius);

        QString text;
        text.setNum(center.x());
        tipOffsetX->setText(text);

        text.clear();
        text.setNum(center.y());
        tipOffsetY->setText(text);

        text.setNum(center.z());
        tipOffsetZ->setText(text);
        fitCenterBtn->setText("Fit Center");

        //add center node
        vtkSmartPointer<vtkSphereSource> sphereSource =
            vtkSmartPointer<vtkSphereSource>::New();
        sphereSource->SetCenter(center.x(), center.y(), center.z());
        sphereSource->SetRadius(100.0);
        mitk::Surface::Pointer centerSurface = mitk::Surface::New();
        centerSurface->SetVtkPolyData(sphereSource->GetOutput());
        centerSurface->Update();
        mitk::DataNode::Pointer centerNode = mitk::DataNode::New();
        centerNode->SetData(centerSurface);
        std::string name = "line";
        centerNode->SetProperty("name", mitk::StringProperty::New("center"));
        centerNode->SetProperty("color", mitk::ColorProperty::New(1.0, 1.0, 1.0));
        centerNode->Update();
        IQF_MitkDataManager* pMitkDataStorage = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr("QF_MitkMain_DataManager");
        if (pMitkDataStorage)
        {
            pMitkDataStorage->GetDataStorage()->Add(centerNode);
            mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
    }
}