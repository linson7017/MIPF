/********************************************************************
	FileName:    VTKSceneViwer.h
	Author:        Ling Song
	Date:           Month 12 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef VTKSceneViwer_h__
#define VTKSceneViwer_h__

#include <QtWidgets>
#include <QVTKWidget.h>
#include <vtkRenderer.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

namespace QF
{
    class IQF_Main;
}

class ctkDoubleSlider;

class VTKSceneViwer : public QDialog
{
    Q_OBJECT
public:
    VTKSceneViwer(QF::IQF_Main* pMain,QWidget* parent=0);
    ~VTKSceneViwer();
    vtkRenderWindow* GetRenderWindow() { return m_vtkWidget->GetRenderWindow(); }
    void AddPolyData(vtkPolyData* polyData,const std::string& name="");
protected slots:
    void Apply();

    void ChangeColor();
    void ChangeAmbientColor();
    void ChangeDiffuseColor();
    void ChangeSpecularColor();
    void ChangeTexture();

    void ChangeAmbient(double value);
    void ChangeDiffuse(double value);
    void ChangeSpecular(double value);


    void SpecularPowerChanged(double value);
    void OpacityChanged(double value);
    void RepresentationChanged(const QString &representation);
    void ShadingChanged(int state);
    void LightingChanged(int state);

    void ActorSelectionChanged(int row);

private:
    void UpdateCurrentActor();
    vtkActor* CurrentActor();
    void GetLargestBounds(double* largestBounds);
    void GetBoundsCenter(double* center);
    void UpdataLight();
private:
    vtkActor* m_currentActor;
    QVTKWidget* m_vtkWidget;
    vtkSmartPointer<vtkRenderer> m_vtkRenderer;
    vtkSmartPointer<vtkLight> m_light;
    QF::IQF_Main* m_pMain;

    typedef  std::map<int, vtkActor*> ActorMapType;
    ActorMapType m_actors;

    //ui
    QComboBox* m_typeSelector;
    QListWidget* m_actorListWidget;
    ctkDoubleSlider* m_actorOpacitySlider;
    ctkDoubleSlider* m_actorSpecularPower;

    QPushButton* m_actorColorBtn;
    QPushButton* m_actorAmbientColorBtn;
    QPushButton* m_actorDiffuseColorBtn;
    QPushButton* m_actorSpecularColorBtn;
    QDoubleSpinBox* m_actorAmbientDSB;
    QDoubleSpinBox* m_actorDiffuseDSB;
    QDoubleSpinBox* m_actorSpecularDSB;

    QCheckBox* m_actorShadingCB;
    QCheckBox* m_actorLightingCB;


    QComboBox* m_actorRepresentation;

    double m_bounds[6];

};

#endif // VTKSceneViwer_h__