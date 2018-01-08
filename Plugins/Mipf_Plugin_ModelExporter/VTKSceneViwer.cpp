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
//qt
#include <QColorDialog>
#include <QImage>


//ctk
#include <ctkDoubleSlider.h>

#include "iqf_main.h"
#include "mitkMain/IQF_MitkReference.h"

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
    connect(m_actorListWidget, &QListWidget::currentRowChanged, this, &VTKSceneViwer::ActorSelectionChanged);


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


void VTKSceneViwer::ActorSelectionChanged(int row)
{
    if (m_actors.count(row))
    {
        m_currentActor = m_actors[row];
        UpdateCurrentActor();
    }    
}

void VTKSceneViwer::UpdateCurrentActor()
{
    vtkActor* currentActor = CurrentActor();
    if (!currentActor)
    {
        return;
    }
    //opacity
    m_actorOpacitySlider->setValue(currentActor->GetProperty()->GetOpacity());
    //specular power 
    m_actorSpecularPower->setValue(currentActor->GetProperty()->GetSpecularPower());

    //color
    double* color = currentActor->GetProperty()->GetColor();
    QColor wholeColor(color[0] * 255, color[1] * 255, color[2] * 255);
    QString styleSheet = QString("QPushButton {background:%1}").arg(wholeColor.name());
    m_actorColorBtn->setStyleSheet(styleSheet);

    color = currentActor->GetProperty()->GetAmbientColor();
    QColor ambientColor(color[0] * 255, color[1] * 255, color[2] * 255);
    styleSheet = QString("QPushButton {background:%1}").arg(ambientColor.name());
    m_actorAmbientColorBtn->setStyleSheet(styleSheet);

    color = currentActor->GetProperty()->GetDiffuseColor();
    QColor diffuseColor(color[0] * 255, color[1] * 255, color[2] * 255);
    styleSheet = QString("QPushButton {background:%1}").arg(diffuseColor.name());
    m_actorDiffuseColorBtn->setStyleSheet(styleSheet);

    color = currentActor->GetProperty()->GetSpecularColor();
    QColor specularColor(color[0] * 255, color[1] * 255, color[2] * 255);
    styleSheet = QString("QPushButton {background:%1}").arg(specularColor.name());
    m_actorSpecularColorBtn->setStyleSheet(styleSheet);

    m_actorAmbientDSB->setValue(m_currentActor->GetProperty()->GetAmbient());
    m_actorDiffuseDSB->setValue(m_currentActor->GetProperty()->GetDiffuse());
    m_actorSpecularDSB->setValue(m_currentActor->GetProperty()->GetSpecular());


    //representation
    m_actorRepresentation->setCurrentText(currentActor->GetProperty()->GetRepresentationAsString());

    //shading and lighting
    m_actorShadingCB->setChecked(currentActor->GetProperty()->GetShading());
    m_actorLightingCB->setChecked(currentActor->GetProperty()->GetLighting());


    m_vtkWidget->update();
}

void VTKSceneViwer::UpdataLight()
{
    if (!m_light)
    {
        //add light to scene
        m_light = vtkSmartPointer<vtkLight>::New();
        double focalPoint[3];
        GetBoundsCenter(focalPoint);
        m_light->SetFocalPoint(focalPoint);
        m_light->SetPosition(m_vtkRenderer->GetActiveCamera()->GetPosition());
        m_vtkRenderer->AddLight(m_light);
    }
    
}

void VTKSceneViwer::OpacityChanged(double value)
{
    m_currentActor->GetProperty()->SetOpacity(value);
    m_vtkWidget->update();
}

void VTKSceneViwer::SpecularPowerChanged(double value)
{
    m_currentActor->GetProperty()->SetSpecularPower(value);
    m_vtkWidget->update();
}

void VTKSceneViwer::ChangeColor()
{
    QColor c = QColorDialog::getColor();
    m_currentActor->GetProperty()->SetColor(c.redF(), c.greenF(), c.blueF());
    UpdateCurrentActor();
}

void VTKSceneViwer::ChangeAmbientColor()
{
    QColor c = QColorDialog::getColor();
    m_currentActor->GetProperty()->SetAmbientColor(c.redF(), c.greenF(), c.blueF());
    UpdateCurrentActor();
}
void VTKSceneViwer::ChangeDiffuseColor()
{
    QColor c = QColorDialog::getColor();
    m_currentActor->GetProperty()->SetDiffuseColor(c.redF(), c.greenF(), c.blueF());
    UpdateCurrentActor();
}
void VTKSceneViwer::ChangeSpecularColor()
{
    QColor c = QColorDialog::getColor();
    m_currentActor->GetProperty()->SetSpecularColor(c.redF(), c.greenF(), c.blueF());
    UpdateCurrentActor();
}

void VTKSceneViwer::ChangeAmbient(double value)
{
    m_currentActor->GetProperty()->SetAmbient(value);
    m_vtkWidget->update();
}

void VTKSceneViwer::ChangeDiffuse(double value)
{
    m_currentActor->GetProperty()->SetDiffuse(value);
    m_vtkWidget->update();

}

void VTKSceneViwer::ChangeSpecular(double value)
{
    m_currentActor->GetProperty()->SetSpecular(value);
    m_vtkWidget->update();

}

void VTKSceneViwer::AddPolyData(vtkPolyData* polyData, const std::string& name)
{
    vtkSmartPointer<vtkPolyDataMapper> mapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    vtkSmartPointer<vtkActor> actor =
        vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    //set default color as white
    actor->GetProperty()->SetColor(1.0, 1.0, 1.0);
    m_vtkRenderer->AddActor(actor);

    m_actorListWidget->addItem(name.c_str());
    m_actors[m_actors.size()] = actor;

    m_actorListWidget->setCurrentRow(m_actors.size());

    //UpdataLight();
    m_vtkWidget->update();
}

void VTKSceneViwer::GetLargestBounds(double* largestBounds)
{
    for (ActorMapType::iterator it = m_actors.begin(); it != m_actors.end(); it++)
    {
        vtkActor* actor = it->second;
        double* bound = actor->GetBounds();
        largestBounds[0] = largestBounds[0] < bound[0] ? largestBounds[0] : bound[0];
        largestBounds[1] = largestBounds[1] > bound[1] ? largestBounds[1] : bound[1];
        largestBounds[2] = largestBounds[2] < bound[2] ? largestBounds[2] : bound[2];
        largestBounds[3] = largestBounds[3] > bound[3] ? largestBounds[3] : bound[3];
        largestBounds[4] = largestBounds[4] < bound[4] ? largestBounds[4] : bound[4];
        largestBounds[5] = largestBounds[5] > bound[5] ? largestBounds[5] : bound[5];
    }
}

void VTKSceneViwer::GetBoundsCenter(double* center)
{
    double bounds[6];
    GetLargestBounds(bounds);
    center[0] = (bounds[0] + bounds[1]) / 2;
    center[1] = (bounds[2] + bounds[3]) / 2;
    center[2] = (bounds[4] + bounds[5]) / 2;

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
