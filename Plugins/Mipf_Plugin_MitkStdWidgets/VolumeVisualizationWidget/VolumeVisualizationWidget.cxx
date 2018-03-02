#include "VolumeVisualizationWidget.h"

#include "QmitkStdMultiWidget.h"
#include "mitkRenderingManager.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkPointSet.h"
#include "mitkPointSetDataInteractor.h"
#include "mitkImageAccessByItk.h"
#include "mitkRenderingManager.h"
#include "mitkDataStorage.h"
#include "mitkTransferFunctionInitializer.h"
#include "QmitkTransferFunctionWidget.h"
#include "QmitkTransferFunctionGeneratorWidget.h"

#include <mitkIOUtil.h>
#include <QtWidgets>


#include <MitkMain/IQF_MitkDataManager.h>
#include <mitkMain/IQF_MitkRenderWindow.h>

#include "iqf_main.h"
#include "Utils/variant.h"

//##Documentation
//## @brief As MultiViews, but with QmitkStdMultiWidget as widget


enum RenderMode
{
    RM_CPU_COMPOSITE_RAYCAST = 0,
    RM_CPU_MIP_RAYCAST = 1,
    RM_GPU_COMPOSITE_SLICING = 2,
    RM_GPU_COMPOSITE_RAYCAST = 3,
    RM_GPU_MIP_RAYCAST = 4
};


VolumeVisualizationWidget::VolumeVisualizationWidget():MitkPluginView()
{
}

void VolumeVisualizationWidget::CreateView()
{
    Init(NULL);
}

void VolumeVisualizationWidget::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, MITK_MESSAGE_SELECTION_CHANGED) == 0)
    {
        IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        if(!pMitkDataManager)
        {
            return;
        }
        std::vector<mitk::DataNode::Pointer> nodes = pMitkDataManager->GetSelectedNodes();
        bool weHadAnImageButItsNotThreeDeeOrFourDee = false;
        mitk::DataNode::Pointer node;
        for(int i = 0; i < nodes.size();  i++)
        {
            mitk::DataNode::Pointer currentNode = nodes.at(i);
            if (currentNode.IsNotNull() && dynamic_cast<mitk::Image*>(currentNode->GetData()))
            {
                if (dynamic_cast<mitk::Image*>(currentNode->GetData())->GetDimension() >= 3)
                {
                    if (node.IsNull())
                    {
                        node = currentNode;
                    }
                }
                else
                {
                    weHadAnImageButItsNotThreeDeeOrFourDee = true;
                }
            }
        }

        if (node.IsNotNull())
        {
            m_NoSelectedImageLabel->hide();
            m_ErrorImageLabel->hide();
            m_SelectedImageLabel->show();

            std::string  infoText;

            if (node->GetName().empty())
                infoText = std::string("Selected Image: [currently selected image has no name]");
            else
                infoText = std::string("Selected Image: ") + node->GetName();

            m_SelectedImageLabel->setText(QString(infoText.c_str()));

            m_SelectedNode = node;
        }
        else
        {
            if (weHadAnImageButItsNotThreeDeeOrFourDee)
            {
                m_NoSelectedImageLabel->hide();
                m_ErrorImageLabel->show();
                std::string  infoText;
                infoText = std::string("only 3D or 4D images are supported");
                m_ErrorImageLabel->setText(QString(infoText.c_str()));
            }
            else
            {
                m_SelectedImageLabel->hide();
                m_ErrorImageLabel->hide();
                m_NoSelectedImageLabel->show();
            }

            m_SelectedNode = 0;
        }

        UpdateInterface();
    }
    else if (strcmp(szMessage, MITK_MESSAGE_NODE_REMOVED) == 0)
    {
        mitk::DataNode* node = (mitk::DataNode*)pValue;
        if (m_SelectedNode.GetPointer() == node)
        {
            m_SelectedNode = 0;
            m_SelectedImageLabel->hide();
            m_ErrorImageLabel->hide();
            m_NoSelectedImageLabel->show();
            UpdateInterface();
        }
    }
    else if (strcmp(szMessage, MITK_MESSAGE_NODE_CHANGED) == 0)
    {
        mitk::DataNode* node = (mitk::DataNode*)pValue;
        if (m_SelectedNode.GetPointer() == node)
        {
            UpdateInterface();
        }
    }
}


void VolumeVisualizationWidget::Init(QWidget* parent)
{
    m_pMain->Attach(this);
    QVBoxLayout* vLayout = new QVBoxLayout;
    setLayout(vLayout);

    m_SelectedImageLabel = new QLabel("");
    m_NoSelectedImageLabel = new QLabel("Please select a volume image!");
    m_ErrorImageLabel = new QLabel("");

    m_EnableRenderingCB = new QCheckBox("Volume Rendering");
    m_EnableLOD = new QCheckBox("LOD");
    m_RenderModeComboBox = new QComboBox(this);
    m_TransferFunctionWidget = new QmitkTransferFunctionWidget(this);
    m_TransferFunctionGeneratorWidget = new QmitkTransferFunctionGeneratorWidget(this);

    vLayout->addWidget(m_ErrorImageLabel);
    vLayout->addWidget(m_NoSelectedImageLabel);
    vLayout->addWidget(m_SelectedImageLabel);
    {
        QHBoxLayout* hlayout = new QHBoxLayout;
        hlayout->addWidget(m_EnableRenderingCB);
        hlayout->addWidget(m_EnableLOD);
        hlayout->addWidget(m_RenderModeComboBox);
        vLayout->addLayout(hlayout);
    }
    vLayout->addWidget(m_TransferFunctionGeneratorWidget,1);
    vLayout->addWidget(m_TransferFunctionWidget,5);


    // Fill the tf presets in the generator widget
    std::vector<std::string> names;
    mitk::TransferFunctionInitializer::GetPresetNames(names);
    for (std::vector<std::string>::const_iterator it = names.begin();
        it != names.end(); ++it)
    {
        m_TransferFunctionGeneratorWidget->AddPreset(QString::fromStdString(*it));
    }

    m_RenderModeComboBox->addItem("CPU raycast");
    m_RenderModeComboBox->addItem("CPU MIP raycast");
    m_RenderModeComboBox->addItem("GPU slicing");
    // Only with VTK 5.6 or above
#if ((VTK_MAJOR_VERSION > 5) || ((VTK_MAJOR_VERSION==5) && (VTK_MINOR_VERSION>=6) ))
    m_Controls->m_RenderMode->addItem("GPU raycast");
    m_Controls->m_RenderMode->addItem("GPU MIP raycast");
#endif

    connect(m_EnableRenderingCB, SIGNAL(toggled(bool)), this, SLOT(OnEnableRendering(bool)));
    connect(m_EnableLOD, SIGNAL(toggled(bool)), this, SLOT(OnEnableLOD(bool)));
    connect(m_RenderModeComboBox, SIGNAL(activated(int)), this, SLOT(OnRenderMode(int)));

    connect(m_TransferFunctionGeneratorWidget, SIGNAL(SignalUpdateCanvas()), m_TransferFunctionWidget, SLOT(OnUpdateCanvas()));
    connect(m_TransferFunctionGeneratorWidget, SIGNAL(SignalTransferFunctionModeChanged(int)), SLOT(OnMitkInternalPreset(int)));

    m_EnableRenderingCB->setEnabled(false);
    m_EnableLOD->setEnabled(false);
    m_RenderModeComboBox->setEnabled(false);
    m_TransferFunctionWidget->setEnabled(false);
    m_TransferFunctionGeneratorWidget->setEnabled(false);

    m_SelectedImageLabel->hide();
    m_ErrorImageLabel->hide();
}


void VolumeVisualizationWidget::UpdateInterface()
{
    if (m_SelectedNode.IsNull())
    {
        // turnoff all
        m_EnableRenderingCB->setChecked(false);
        m_EnableRenderingCB->setEnabled(false);

        m_EnableLOD->setChecked(false);
        m_EnableLOD->setEnabled(false);

        m_RenderModeComboBox->setCurrentIndex(0);
        m_RenderModeComboBox->setEnabled(false);

        m_TransferFunctionWidget->SetDataNode(0);
        m_TransferFunctionWidget->setEnabled(false);

        m_TransferFunctionGeneratorWidget->SetDataNode(0);
        m_TransferFunctionGeneratorWidget->setEnabled(false);
        return;
    }

    bool enabled = false;

    m_SelectedNode->GetBoolProperty("volumerendering", enabled);
    m_EnableRenderingCB->setEnabled(true);
    m_EnableRenderingCB->setChecked(enabled);

    if (!enabled)
    {
        // turnoff all except volumerendering checkbox
        m_EnableLOD->setChecked(false);
        m_EnableLOD->setEnabled(false);

        m_RenderModeComboBox->setCurrentIndex(0);
        m_RenderModeComboBox->setEnabled(false);

        m_TransferFunctionWidget->SetDataNode(0);
        m_TransferFunctionWidget->setEnabled(false);

        m_TransferFunctionGeneratorWidget->SetDataNode(0);
        m_TransferFunctionGeneratorWidget->setEnabled(false);
        return;
    }

    // otherwise we can activate em all
    enabled = false;
    m_SelectedNode->GetBoolProperty("volumerendering.uselod", enabled);
    m_EnableLOD->setEnabled(true);
    m_EnableLOD->setChecked(enabled);

    m_RenderModeComboBox->setEnabled(true);

    // Determine Combo Box mode
    {
        bool usegpu = false;
        bool useray = false;
        bool usemip = false;
        m_SelectedNode->GetBoolProperty("volumerendering.usegpu", usegpu);
        // Only with VTK 5.6 or above
#if ((VTK_MAJOR_VERSION > 5) || ((VTK_MAJOR_VERSION==5) && (VTK_MINOR_VERSION>=6) ))
        m_SelectedNode->GetBoolProperty("volumerendering.useray", useray);
#endif
        m_SelectedNode->GetBoolProperty("volumerendering.usemip", usemip);

        int mode = 0;

        if (useray)
        {
            if (usemip)
                mode = RM_GPU_MIP_RAYCAST;
            else
                mode = RM_GPU_COMPOSITE_RAYCAST;
        }
        else if (usegpu)
            mode = RM_GPU_COMPOSITE_SLICING;
        else
        {
            if (usemip)
                mode = RM_CPU_MIP_RAYCAST;
            else
                mode = RM_CPU_COMPOSITE_RAYCAST;
        }

        m_RenderModeComboBox->setCurrentIndex(mode);
    }

    m_TransferFunctionWidget->SetDataNode(m_SelectedNode);
    m_TransferFunctionWidget->setEnabled(true);
    m_TransferFunctionGeneratorWidget->SetDataNode(m_SelectedNode);
    m_TransferFunctionGeneratorWidget->setEnabled(true);
}


void VolumeVisualizationWidget::OnEnableRendering(bool state)
{
    if (m_SelectedNode.IsNull())
        return;

    m_SelectedNode->SetProperty("volumerendering", mitk::BoolProperty::New(state));
    UpdateInterface();
    RequestRenderWindowUpdate();
}

void VolumeVisualizationWidget::OnEnableLOD(bool state)
{
    if (m_SelectedNode.IsNull())
        return;

    m_SelectedNode->SetProperty("volumerendering.uselod", mitk::BoolProperty::New(state));
    RequestRenderWindowUpdate();
}

void VolumeVisualizationWidget::OnRenderMode(int mode)
{
    if (m_SelectedNode.IsNull())
        return;

    bool usegpu = mode == RM_GPU_COMPOSITE_SLICING;
    // Only with VTK 5.6 or above
#if ((VTK_MAJOR_VERSION > 5) || ((VTK_MAJOR_VERSION==5) && (VTK_MINOR_VERSION>=6) ))
    bool useray = (mode == RM_GPU_COMPOSITE_RAYCAST) || (mode == RM_GPU_MIP_RAYCAST);
#endif
    bool usemip = (mode == RM_GPU_MIP_RAYCAST) || (mode == RM_CPU_MIP_RAYCAST);

    m_SelectedNode->SetProperty("volumerendering.usegpu", mitk::BoolProperty::New(usegpu));
    // Only with VTK 5.6 or above
#if ((VTK_MAJOR_VERSION > 5) || ((VTK_MAJOR_VERSION==5) && (VTK_MINOR_VERSION>=6) ))
    m_SelectedNode->SetProperty("volumerendering.useray", mitk::BoolProperty::New(useray));
#endif
    m_SelectedNode->SetProperty("volumerendering.usemip", mitk::BoolProperty::New(usemip));

    RequestRenderWindowUpdate();
}


void VolumeVisualizationWidget::OnMitkInternalPreset(int mode)
{
    if (m_SelectedNode.IsNull()) return;

    mitk::DataNode::Pointer node(m_SelectedNode.GetPointer());
    mitk::TransferFunctionProperty::Pointer transferFuncProp;
    if (node->GetProperty(transferFuncProp, "TransferFunction"))
    {
        //first item is only information
        if (--mode == -1)
            return;

        // -- Creat new TransferFunction
        mitk::TransferFunctionInitializer::Pointer tfInit = mitk::TransferFunctionInitializer::New(transferFuncProp->GetValue());
        tfInit->SetTransferFunctionMode(mode);
        RequestRenderWindowUpdate();
        m_TransferFunctionWidget->OnUpdateCanvas();
    }
}