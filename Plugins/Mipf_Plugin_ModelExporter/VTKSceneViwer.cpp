#include "VTKSceneViwer.h"

#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkPolyDataMapper.h>  
#include <vtkActor.h>  
#include <vtkRenderWindow.h>
#include <vtkOBJExporter.h>
#include <vtkX3DExporter.h>
#include <vtkActorCollection.h>
#include <vtkProperty.h>
#include <vtkQImageToImageSource.h>
#include <vtkTexture.h>
#include <vtkNeverTranslucentTexture.h>
#include <vtkImageData.h>
#include <vtkLight.h>
#include <vtkCamera.h>
#include <vtkLightActor.h>
#include <vtkPointData.h>
//qt
#include <QColorDialog>
#include <QImage>




//ctk
#include <ctkDoubleSlider.h>

#include "iqf_main.h"
#include "mitkMain/IQF_MitkReference.h"


//mitk
#include "mitkVtkInterpolationProperty.h"
#include "mitkVtkRepresentationProperty.h"
#include <mitkExtractSliceFilter.h>
#include <mitkIPropertyAliases.h>
#include <mitkIPropertyDescriptions.h>
#include <mitkIShaderRepository.h>
#include <mitkImageSliceSelector.h>
#include <mitkLookupTableProperty.h>
#include <mitkProperties.h>
#include <mitkSmartPointerProperty.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkVtkInterpolationProperty.h>
#include <mitkVtkRepresentationProperty.h>
#include <mitkVtkScalarModeProperty.h>


const int VTKSceneViwer::s_DataRole = 10086;

VTKSceneViwer::VTKSceneViwer(QF::IQF_Main* pMain, QWidget* parent) :m_pMain(pMain) , QDialog(parent), m_currentActor(nullptr)
{
    for (int i=0;i<3;i++)
    {
        m_bounds[i*2] = DBL_MAX;
        m_bounds[i*2+1] = DBL_MIN;
    }

    setWindowTitle("Scene Exporter");
    setMinimumSize(800, 500);

    m_vtkWidget = new QVTKWidget;
    m_vtkRenderer = vtkSmartPointer<vtkRenderer>::New();
    m_vtkWidget->GetRenderWindow()->AddRenderer(m_vtkRenderer);
    

    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(m_vtkWidget,5);

    
    QVBoxLayout* controlLayout = new QVBoxLayout;
    m_actorListWidget = new QListWidget;
    m_actorListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    controlLayout->addWidget(m_actorListWidget);
    connect(m_actorListWidget, &QListWidget::currentItemChanged, this, &VTKSceneViwer::ActorSelectionChanged);


    {
        QPushButton* btn = new QPushButton("-");
        connect(btn, &QPushButton::clicked, this, &VTKSceneViwer::Remove);

        QHBoxLayout* layout = new QHBoxLayout;
        layout->addWidget(btn);
        controlLayout->addLayout(layout);
    }

    {
        QPushButton* btn = new QPushButton("...");
        connect(btn, &QPushButton::clicked, this, &VTKSceneViwer::ChangeTexture);

        QHBoxLayout* layout = new QHBoxLayout;
        layout->addWidget(new QLabel("Texture:"));
        layout->addWidget(btn);
        controlLayout->addLayout(layout);
    }

    //color
    {
        m_actorColorBtn = new QPushButton();
        m_actorColorBtn->setFixedWidth(20);
        m_actorColorBtn->setStyleSheet("QPushButton {background:white}");
        connect(m_actorColorBtn, &QPushButton::clicked, this, &VTKSceneViwer::ChangeColor);

        QHBoxLayout* layout = new QHBoxLayout;
        layout->addWidget(new QLabel("Color:"));
        layout->addWidget(m_actorColorBtn);
        controlLayout->addLayout(layout);
    }

    //ambient  color
    {
        m_actorAmbientColorBtn = new QPushButton();
        m_actorAmbientColorBtn->setFixedWidth(20);
        m_actorAmbientColorBtn->setStyleSheet("QPushButton {background:white}");
        connect(m_actorAmbientColorBtn, &QPushButton::clicked, this, &VTKSceneViwer::ChangeAmbientColor);

        m_actorAmbientDSB = new QDoubleSpinBox();
        m_actorAmbientDSB->setRange(0.0, 1.0);
        m_actorAmbientDSB->setSingleStep(0.1);
        m_actorAmbientDSB->setValue(0.0);
        connect(m_actorAmbientDSB, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &VTKSceneViwer::ChangeAmbient);
        
        QHBoxLayout* layout = new QHBoxLayout;
        layout->addWidget(new QLabel("Ambient Color:"));
        layout->addWidget(m_actorAmbientColorBtn);
        layout->addWidget(m_actorAmbientDSB);
        controlLayout->addLayout(layout);
    }

    //diffuse color
    {
        m_actorDiffuseColorBtn = new QPushButton();
        m_actorDiffuseColorBtn->setFixedWidth(20);
        m_actorDiffuseColorBtn->setStyleSheet("QPushButton {background:white}");
        connect(m_actorDiffuseColorBtn, &QPushButton::clicked, this, &VTKSceneViwer::ChangeDiffuseColor);

        m_actorDiffuseDSB = new QDoubleSpinBox();
        m_actorDiffuseDSB->setRange(0.0, 1.0);
        m_actorDiffuseDSB->setSingleStep(0.1);
        m_actorDiffuseDSB->setValue(1.0);
        connect(m_actorDiffuseDSB, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &VTKSceneViwer::ChangeDiffuse);

        QHBoxLayout* layout = new QHBoxLayout;
        layout->addWidget(new QLabel("Diffuse Color:"));
        layout->addWidget(m_actorDiffuseColorBtn);
        layout->addWidget(m_actorDiffuseDSB);
        controlLayout->addLayout(layout);
    }

    //specular  color
    {
        m_actorSpecularColorBtn = new QPushButton();
        m_actorSpecularColorBtn->setFixedWidth(20);
        m_actorSpecularColorBtn->setStyleSheet("QPushButton {background:white}");
        connect(m_actorSpecularColorBtn, &QPushButton::clicked, this, &VTKSceneViwer::ChangeSpecularColor);

        m_actorSpecularDSB = new QDoubleSpinBox();
        m_actorSpecularDSB->setRange(0.0, 1.0);
        m_actorSpecularDSB->setSingleStep(0.1);
        m_actorSpecularDSB->setValue(0.0);
        connect(m_actorSpecularDSB, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &VTKSceneViwer::ChangeSpecular);

        QHBoxLayout* layout = new QHBoxLayout;
        layout->addWidget(new QLabel("Specular Color:"));
        layout->addWidget(m_actorSpecularColorBtn);
        layout->addWidget(m_actorSpecularDSB);
        controlLayout->addLayout(layout);
    }
    //shading and lighting
    {
        //shading
        m_actorShadingCB = new QCheckBox();
        m_actorShadingCB->setChecked(false);
        connect(m_actorShadingCB, &QCheckBox::stateChanged, this, &VTKSceneViwer::ShadingChanged);

        QHBoxLayout* layout = new QHBoxLayout;
        layout->addWidget(new QLabel("Shading:"));
        layout->addWidget(m_actorShadingCB);

        //lighting
        m_actorLightingCB = new QCheckBox();
        m_actorLightingCB->setChecked(false);
        connect(m_actorLightingCB, &QCheckBox::stateChanged, this, &VTKSceneViwer::LightingChanged);

        layout->addWidget(new QLabel("Lighting:"));
        layout->addWidget(m_actorLightingCB);

        controlLayout->addLayout(layout);
    }
    //specular power
    {
        m_actorSpecularPower = new ctkDoubleSlider;
        m_actorSpecularPower->setMaximum(128);
        m_actorSpecularPower->setMinimum(0);
        m_actorSpecularPower->setSingleStep(1);
        m_actorSpecularPower->setValue(1.0);
        m_actorSpecularPower->setOrientation(Qt::Horizontal);
        connect(m_actorSpecularPower, &ctkDoubleSlider::valueChanged, this, &VTKSceneViwer::SpecularPowerChanged);

        QHBoxLayout* layout = new QHBoxLayout;
        layout->addWidget(new QLabel("Specular Power:"));
        layout->addWidget(m_actorSpecularPower);
        controlLayout->addLayout(layout);
    }

    //opcity
    {          
        m_actorOpacitySlider = new ctkDoubleSlider;
        m_actorOpacitySlider->setMaximum(1.0);
        m_actorOpacitySlider->setMinimum(0.0);
        m_actorOpacitySlider->setSingleStep(0.1);
        m_actorOpacitySlider->setValue(1.0);
        m_actorOpacitySlider->setOrientation(Qt::Horizontal);
        connect(m_actorOpacitySlider, &ctkDoubleSlider::valueChanged, this, &VTKSceneViwer::OpacityChanged);

        QHBoxLayout* layout = new QHBoxLayout;
        layout->addWidget(new QLabel("Opacity:"));
        layout->addWidget(m_actorOpacitySlider);
        controlLayout->addLayout(layout);
    }

    {
        m_actorRepresentation = new QComboBox;
        m_actorRepresentation->addItem("Surface");
        m_actorRepresentation->addItem("Points");
        m_actorRepresentation->addItem("WireFrame");
        connect(m_actorRepresentation, &QComboBox::currentTextChanged, this, &VTKSceneViwer::RepresentationChanged);


        QHBoxLayout* layout = new QHBoxLayout;
        layout->addWidget(new QLabel("Representation:"));
        layout->addWidget(m_actorRepresentation);
        controlLayout->addLayout(layout);
    }

    {
        m_typeSelector = new QComboBox();
        m_typeSelector->addItem("OBJ");
        m_typeSelector->addItem("X3D");

        QHBoxLayout* layout = new QHBoxLayout;
        layout->addWidget(new QLabel("Output Type:"));
        layout->addWidget(m_typeSelector);
        controlLayout->addLayout(layout);
    }

    QPushButton* applyButton = new QPushButton("Apply");
    controlLayout->addWidget(applyButton);
    connect(applyButton, &QPushButton::clicked, this, &VTKSceneViwer::Apply);

    layout->addLayout(controlLayout,2);
    setLayout(layout);



}


VTKSceneViwer::~VTKSceneViwer()
{
}

void VTKSceneViwer::Remove()
{
    if (m_actorListWidget->currentItem())
    {
        m_vtkRenderer->RemoveActor(m_actorListWidget->currentItem()->data(s_DataRole).value<vtkActor*>());
        m_currentActor = nullptr;
        m_actorListWidget->takeItem(m_actorListWidget->currentRow());
        UpdateCurrentActor();
    }
    
}

void VTKSceneViwer::ShadingChanged(int state)
{
    if (m_currentActor)
    {
        m_currentActor->GetProperty()->SetShading(state);
        m_vtkWidget->update();
    }
}

void VTKSceneViwer::LightingChanged(int state)
{
    if (m_currentActor)
    {
        m_currentActor->GetProperty()->SetLighting(state);
        m_vtkWidget->update();
    }
}

vtkActor* VTKSceneViwer::CurrentActor()
{
    return m_currentActor;
}


void VTKSceneViwer::RepresentationChanged(const QString &representation)
{
    vtkActor* currentActor = CurrentActor();
    if (!currentActor)
    {
        return;
    }
    if (representation.compare("Surface",Qt::CaseInsensitive)==0)
    {
        currentActor->GetProperty()->SetRepresentationToSurface();
    }
    else if(representation.compare("Points", Qt::CaseInsensitive) == 0)
    {
        currentActor->GetProperty()->SetRepresentationToPoints();
    }
    else if (representation.compare("WireFrame", Qt::CaseInsensitive) == 0)
    {
        currentActor->GetProperty()->SetRepresentationToWireframe();
    }
    m_vtkWidget->update();
}

void VTKSceneViwer::ChangeTexture()
{
    QString filename = QFileDialog::getOpenFileName(this);
    QImage qimage(filename);

    auto qimageToImageSource = vtkSmartPointer<vtkQImageToImageSource>::New();
    qimageToImageSource->SetQImage(&qimage);
    qimageToImageSource->Update();

    auto texture = vtkSmartPointer<vtkTexture>::New();
    texture->SetInputData(qimageToImageSource->GetOutput());
    texture->Update();
    
    m_currentActor->SetTexture(texture);

    m_vtkWidget->update();
}


void VTKSceneViwer::ActorSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (current)
    {
        m_currentActor = current->data(s_DataRole).value<vtkActor*>();
        if (m_currentActor)
        {
            UpdateCurrentActor();
        }
    }
}

void VTKSceneViwer::UpdateCurrentActor()
{
    vtkActor* currentActor = CurrentActor();
    double opacity = 1.0;
    double color[3] = { 1.0, 1.0, 1.0 };
    double ambient[3] = { 0.5, 0.5, 0.0 };
    double diffuse[3] = { 0.5, 0.5, 0.0 };
    double specular[3] = { 1.0, 1.0, 1.0 };

    float coeff_ambient = 0.5f;
    float coeff_diffuse = 0.5f;
    float coeff_specular = 0.5f;
    float power_specular = 10.0f;

    std::string representation = "surface";
    bool shading = false;
    bool lighting = true;


    if (currentActor)
    {
        opacity = currentActor->GetProperty()->GetOpacity();
        power_specular = currentActor->GetProperty()->GetSpecularPower();
        currentActor->GetProperty()->GetColor(color);
        currentActor->GetProperty()->GetAmbientColor(ambient);
        currentActor->GetProperty()->GetDiffuseColor(diffuse);
        currentActor->GetProperty()->GetSpecularColor(specular);
        coeff_ambient = m_currentActor->GetProperty()->GetAmbient();
        coeff_diffuse = m_currentActor->GetProperty()->GetDiffuse();
        coeff_specular = m_currentActor->GetProperty()->GetSpecular();

        representation = currentActor->GetProperty()->GetRepresentationAsString();
        shading = currentActor->GetProperty()->GetShading();
        lighting = currentActor->GetProperty()->GetLighting();

    }
    //opacity
    m_actorOpacitySlider->setValue(opacity);
    //specular power 
    m_actorSpecularPower->setValue(power_specular);

    //color
    QColor wholeColor(color[0] * 255, color[1] * 255, color[2] * 255);
    QString styleSheet = QString("QPushButton {background:%1}").arg(wholeColor.name());
    m_actorColorBtn->setStyleSheet(styleSheet);

    QColor ambientColor(ambient[0] * 255, ambient[1] * 255, ambient[2] * 255);
    styleSheet = QString("QPushButton {background:%1}").arg(ambientColor.name());
    m_actorAmbientColorBtn->setStyleSheet(styleSheet);

    QColor diffuseColor(diffuse[0] * 255, diffuse[1] * 255, diffuse[2] * 255);
    styleSheet = QString("QPushButton {background:%1}").arg(diffuseColor.name());
    m_actorDiffuseColorBtn->setStyleSheet(styleSheet);

    QColor specularColor(specular[0] * 255, specular[1] * 255, specular[2] * 255);
    styleSheet = QString("QPushButton {background:%1}").arg(specularColor.name());
    m_actorSpecularColorBtn->setStyleSheet(styleSheet);

    m_actorAmbientDSB->setValue(coeff_ambient);
    m_actorDiffuseDSB->setValue(coeff_diffuse);
    m_actorSpecularDSB->setValue(coeff_specular);


    //representation
    m_actorRepresentation->setCurrentText(representation.c_str());

    //shading and lighting
    m_actorShadingCB->setChecked(shading);
    m_actorLightingCB->setChecked(lighting);


    m_vtkWidget->update();
}

void VTKSceneViwer::OpacityChanged(double value)
{
    if (!m_currentActor)
    {
        return;
    }
    m_currentActor->GetProperty()->SetOpacity(value);
    m_vtkWidget->update();
}

void VTKSceneViwer::SpecularPowerChanged(double value)
{
    if (!m_currentActor)
    {
        return;
    }
    m_currentActor->GetProperty()->SetSpecularPower(value);
    m_vtkWidget->update();
}

void VTKSceneViwer::ChangeColor()
{
    if (!m_currentActor)
    {
        return;
    }
    if (!m_currentActor)
    {
        return;
    }
    QColor c = QColorDialog::getColor();
    m_currentActor->GetProperty()->SetColor(c.redF(), c.greenF(), c.blueF());
    UpdateCurrentActor();
}

void VTKSceneViwer::ChangeAmbientColor()
{
    if (!m_currentActor)
    {
        return;
    }
    QColor c = QColorDialog::getColor();
    m_currentActor->GetProperty()->SetAmbientColor(c.redF(), c.greenF(), c.blueF());
    UpdateCurrentActor();
}
void VTKSceneViwer::ChangeDiffuseColor()
{
    if (!m_currentActor)
    {
        return;
    }
    QColor c = QColorDialog::getColor();
    m_currentActor->GetProperty()->SetDiffuseColor(c.redF(), c.greenF(), c.blueF());
    UpdateCurrentActor();
}
void VTKSceneViwer::ChangeSpecularColor()
{
    if (!m_currentActor)
    {
        return;
    }
    QColor c = QColorDialog::getColor();
    m_currentActor->GetProperty()->SetSpecularColor(c.redF(), c.greenF(), c.blueF());
    UpdateCurrentActor();
}

void VTKSceneViwer::ChangeAmbient(double value)
{
    if (!m_currentActor)
    {
        return;
    }
    m_currentActor->GetProperty()->SetAmbient(value);
    m_vtkWidget->update();
}

void VTKSceneViwer::ChangeDiffuse(double value)
{
    if (!m_currentActor)
    {
        return;
    }
    m_currentActor->GetProperty()->SetDiffuse(value);
    m_vtkWidget->update();

}

void VTKSceneViwer::ChangeSpecular(double value)
{
    if (!m_currentActor)
    {
        return;
    }
    m_currentActor->GetProperty()->SetSpecular(value);
    m_vtkWidget->update();

}

void VTKSceneViwer::ApplyProperties(vtkActor* actor, vtkPolyDataMapper* mapper, mitk::DataNode* node, mitk::BaseRenderer* renderer)
{
    double ambient[3] = { 0.5, 0.5, 0.0 };
    double diffuse[3] = { 0.5, 0.5, 0.0 };
    double specular[3] = { 1.0, 1.0, 1.0 };

    float coeff_ambient = 0.5f;
    float coeff_diffuse = 0.5f;
    float coeff_specular = 0.5f;
    float power_specular = 10.0f;

    // Color
    {
        mitk::ColorProperty::Pointer p;
        node->GetProperty(p, "color", renderer);
        if (p.IsNotNull())
        {
            mitk::Color c = p->GetColor();
            ambient[0] = c.GetRed();
            ambient[1] = c.GetGreen();
            ambient[2] = c.GetBlue();
            diffuse[0] = c.GetRed();
            diffuse[1] = c.GetGreen();
            diffuse[2] = c.GetBlue();
            // Setting specular color to the same, make physically no real sense, however vtk rendering slows down, if these
            // colors are different.
            specular[0] = c.GetRed();
            specular[1] = c.GetGreen();
            specular[2] = c.GetBlue();
        }
    }

    // Ambient
    {
        mitk::ColorProperty::Pointer p;
        node->GetProperty(p, "material.ambientColor", renderer);
        if (p.IsNotNull())
        {
            mitk::Color c = p->GetColor();
            ambient[0] = c.GetRed();
            ambient[1] = c.GetGreen();
            ambient[2] = c.GetBlue();
        }
    }

    // Diffuse
    {
        mitk::ColorProperty::Pointer p;
        node->GetProperty(p, "material.diffuseColor", renderer);
        if (p.IsNotNull())
        {
            mitk::Color c = p->GetColor();
            diffuse[0] = c.GetRed();
            diffuse[1] = c.GetGreen();
            diffuse[2] = c.GetBlue();
        }
    }

    // Specular
    {
        mitk::ColorProperty::Pointer p;
        node->GetProperty(p, "material.specularColor", renderer);
        if (p.IsNotNull())
        {
            mitk::Color c = p->GetColor();
            specular[0] = c.GetRed();
            specular[1] = c.GetGreen();
            specular[2] = c.GetBlue();
        }
    }

    // Ambient coeff
    {
        node->GetFloatProperty("material.ambientCoefficient", coeff_ambient, renderer);
    }

    // Diffuse coeff
    {
        node->GetFloatProperty("material.diffuseCoefficient", coeff_diffuse, renderer);
    }

    // Specular coeff
    {
        node->GetFloatProperty("material.specularCoefficient", coeff_specular, renderer);
    }

    // Specular power
    {
        node->GetFloatProperty("material.specularPower", power_specular, renderer);
    }

    auto property = actor->GetProperty();
    property->SetAmbient(coeff_ambient);
    property->SetDiffuse(coeff_diffuse);
    property->SetSpecular(coeff_specular);
    property->SetSpecularPower(power_specular);
    property->SetAmbientColor(ambient);
    property->SetDiffuseColor(diffuse);
    property->SetSpecularColor(specular);

    // Opacity
    {
        float opacity = 1.0f;
        if (node->GetOpacity(opacity, renderer))
            property->SetOpacity(opacity);
    }

    // Wireframe line width
    {
        float lineWidth = 1;
        node->GetFloatProperty("material.wireframeLineWidth", lineWidth, renderer);
        property->SetLineWidth(lineWidth);
    }

    // Point size
    {
        float pointSize = 1.0f;
        node->GetFloatProperty("material.pointSize", pointSize, renderer);
        property->SetPointSize(pointSize);
    }

    // Representation
    {
        mitk::VtkRepresentationProperty::Pointer p;
        node->GetProperty(p, "material.representation", renderer);
        if (p.IsNotNull())
            property->SetRepresentation(p->GetVtkRepresentation());
    }

    // Interpolation
    {
        mitk::VtkInterpolationProperty::Pointer p;
        node->GetProperty(p, "material.interpolation", renderer);
        if (p.IsNotNull())
            property->SetInterpolation(p->GetVtkInterpolation());
    }


    //////////////////////////////////////
    mitk::TransferFunctionProperty::Pointer transferFuncProp;
    node->GetProperty(transferFuncProp, "Surface.TransferFunction", renderer);
    if (transferFuncProp.IsNotNull())
    {
        mapper->SetLookupTable(transferFuncProp->GetValue()->GetColorTransferFunction());
    }

    mitk::LookupTableProperty::Pointer lookupTableProp;
    node->GetProperty(lookupTableProp, "LookupTable", renderer);
    if (lookupTableProp.IsNotNull())
    {
        mapper->SetLookupTable(lookupTableProp->GetLookupTable()->GetVtkLookupTable());
    }

    mitk::LevelWindow levelWindow;
    if (node->GetLevelWindow(levelWindow, renderer, "levelWindow"))
    {
        mapper->SetScalarRange(levelWindow.GetLowerWindowBound(), levelWindow.GetUpperWindowBound());
    }
    else if (node->GetLevelWindow(levelWindow, renderer))
    {
        mapper->SetScalarRange(levelWindow.GetLowerWindowBound(), levelWindow.GetUpperWindowBound());
    }

    bool scalarVisibility = false;
    node->GetBoolProperty("scalar visibility", scalarVisibility);
    mapper->SetScalarVisibility((scalarVisibility ? 1 : 0));

    if (scalarVisibility)
    {
        mitk::VtkScalarModeProperty *scalarMode;
        if (node->GetProperty(scalarMode, "scalar mode", renderer))
            mapper->SetScalarMode(scalarMode->GetVtkScalarMode());
        else
            mapper->SetScalarModeToDefault();

        bool colorMode = false;
        node->GetBoolProperty("color mode", colorMode);
        mapper->SetColorMode((colorMode ? 1 : 0));

        double scalarsMin = 0;
        node->GetDoubleProperty("ScalarsRangeMinimum", scalarsMin, renderer);

        double scalarsMax = 1.0;
        node->GetDoubleProperty("ScalarsRangeMaximum", scalarsMax, renderer);

        mapper->SetScalarRange(scalarsMin, scalarsMax);
    }

    mitk::SmartPointerProperty::Pointer imagetextureProp =
        dynamic_cast<mitk::SmartPointerProperty *>(node->GetProperty("Surface.Texture", renderer));

    if (imagetextureProp.IsNotNull())
    {
        mitk::Image *miktTexture = dynamic_cast<mitk::Image *>(imagetextureProp->GetSmartPointer().GetPointer());
        vtkSmartPointer<vtkTexture> vtkTxture = vtkSmartPointer<vtkTexture>::New();
        // Either select the first slice of a volume
        if (miktTexture->GetDimension(2) > 1)
        {
            MITK_WARN << "3D Textures are not supported by VTK and MITK. The first slice of the volume will be used instead!";
            mitk::ImageSliceSelector::Pointer sliceselector = mitk::ImageSliceSelector::New();
            sliceselector->SetSliceNr(0);
            sliceselector->SetChannelNr(0);
            sliceselector->SetTimeNr(0);
            sliceselector->SetInput(miktTexture);
            sliceselector->Update();
            vtkTxture->SetInputData(sliceselector->GetOutput()->GetVtkImageData());
        }
        else // or just use the 2D image
        {
            vtkTxture->SetInputData(miktTexture->GetVtkImageData());
        }
        // pass the texture to the actor
        actor->SetTexture(vtkTxture);
        if (mapper->GetInput()->GetPointData()->GetTCoords() == NULL)
        {
            MITK_ERROR << "Surface.Texture property was set, but there are no texture coordinates. Please provide texture "
                "coordinates for the vtkPolyData via vtkPolyData->GetPointData()->SetTCoords().";
        }
        // if no texture is set, this will also remove a previously used texture
        // and reset the actor to it's default behaviour
    }
    else
    {
        actor->SetTexture(0);
    }

    // deprecated settings
    bool deprecatedUseCellData = false;
    node->GetBoolProperty("deprecated useCellDataForColouring", deprecatedUseCellData);

    bool deprecatedUsePointData = false;
    node->GetBoolProperty("deprecated usePointDataForColouring", deprecatedUsePointData);

    if (deprecatedUseCellData)
    {
        mapper->SetColorModeToDefault();
        mapper->SetScalarRange(0, 255);
        mapper->ScalarVisibilityOn();
        mapper->SetScalarModeToUseCellData();
        actor->GetProperty()->SetSpecular(1);
        actor->GetProperty()->SetSpecularPower(50);
        actor->GetProperty()->SetInterpolationToPhong();
    }
    else if (deprecatedUsePointData)
    {
        float scalarsMin = 0;
        if (dynamic_cast<mitk::FloatProperty *>(node->GetProperty("ScalarsRangeMinimum")) != NULL)
            scalarsMin =
            dynamic_cast<mitk::FloatProperty *>(node->GetProperty("ScalarsRangeMinimum"))->GetValue();

        float scalarsMax = 0.1;
        if (dynamic_cast<mitk::FloatProperty *>(node->GetProperty("ScalarsRangeMaximum")) != NULL)
            scalarsMax =
            dynamic_cast<mitk::FloatProperty *>(node->GetProperty("ScalarsRangeMaximum"))->GetValue();

        mapper->SetScalarRange(scalarsMin, scalarsMax);
        mapper->SetColorModeToMapScalars();
        mapper->ScalarVisibilityOn();
        actor->GetProperty()->SetSpecular(1);
        actor->GetProperty()->SetSpecularPower(50);
        actor->GetProperty()->SetInterpolationToPhong();
    }

    int deprecatedScalarMode = VTK_COLOR_MODE_DEFAULT;
    if (node->GetIntProperty("deprecated scalar mode", deprecatedScalarMode, renderer))
    {
        mapper->SetScalarMode(deprecatedScalarMode);
        mapper->ScalarVisibilityOn();
        actor->GetProperty()->SetSpecular(1);
        actor->GetProperty()->SetSpecularPower(50);
    }

}


void VTKSceneViwer::AddPolyData(vtkPolyData* polyData, const std::string& name, mitk::DataNode* propertyNode,mitk::BaseRenderer* renderer)
{
    auto copyData = vtkSmartPointer<vtkPolyData>::New();
    copyData->DeepCopy(polyData);

    vtkSmartPointer<vtkPolyDataMapper> mapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(copyData);

    vtkSmartPointer<vtkActor> actor =
        vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    //set default color as white
    actor->GetProperty()->SetColor(1.0, 1.0, 1.0);
    m_vtkRenderer->AddActor(actor);

    if (propertyNode)
    {
        ApplyProperties(actor, mapper,propertyNode, renderer);
    }
    

    //add item add set selected
    QListWidgetItem* item = new QListWidgetItem(name.c_str(), m_actorListWidget);
    item->setData(s_DataRole, QVariant::fromValue(actor.Get()));
    m_actorListWidget->addItem(item);
    m_actorListWidget->setCurrentItem(item);
    m_actorListWidget->setItemSelected(item,true);

    //UpdataLight();
    m_vtkWidget->update();
    m_vtkRenderer->ResetCamera();
}



void VTKSceneViwer::Apply()
{
    IQF_MitkReference* pMitkReference = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);
    QString lastSaveFilePath = pMitkReference->GetString("LastFileSavePath", "");
    QString filename = QFileDialog::getSaveFileName(this, "Save Path", lastSaveFilePath);
    if (filename.isEmpty())
    {
        return;
    }
    pMitkReference->SetString("LastFileSavePath", QFileInfo(filename).absolutePath().toStdString().c_str());


    //exprot scene
    if (m_typeSelector->currentText().compare("OBJ", Qt::CaseInsensitive) == 0)
    {
        auto exporter = vtkSmartPointer<vtkOBJExporter>::New();
        exporter->SetRenderWindow(GetRenderWindow());
        exporter->SetFilePrefix(filename.toLocal8Bit().constData());
        exporter->Write();
    }
    else if (m_typeSelector->currentText().compare("X3D", Qt::CaseInsensitive) == 0)
    {
        auto exporter = vtkSmartPointer<vtkX3DExporter>::New();
        exporter->SetInput(GetRenderWindow());
        QString outputName = filename + ".x3d";
        exporter->SetFileName(outputName.toLocal8Bit().constData());
        exporter->Write();
    }
}
