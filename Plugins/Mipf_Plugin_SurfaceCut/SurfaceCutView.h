#ifndef SurfaceCutView_h__
#define SurfaceCutView_h__

#include "MitkPluginView.h"

#include <QWidget>

#include "ui_SurfaceCutView.h"

class vtkImplicitFunction;

class CutImplementation;

class SurfaceCutView :public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    SurfaceCutView();
    ~SurfaceCutView();
    void CreateView() override;

protected slots:
    void FreehandCut(bool enableCut);
    void Undo();
    void Redo();
    void InsideOut(bool flag);

    void AddBox();
    void RemoveBox();
    void BoxCut();
    void BoxSelected(QListWidgetItem *, QListWidgetItem *);

    void AddGeometry();
    void RemoveGeometry();
    void GeometryCut();
    void OnOriginPointRadioButton(bool);
    void OnCenterPointRadioButton(bool);
    void GeometrySelected(QListWidgetItem *, QListWidgetItem *);
    void GeometryChanged(QListWidgetItem *);

private:
    mitk::Geometry3D::Pointer InitializeWithSurfaceGeometry(mitk::BaseGeometry::Pointer geometry);
    vtkSmartPointer<vtkPolyData> CreateGeometry(mitk::BaseGeometry::Pointer geometry, vtkSmartPointer<vtkImplicitFunction>& geometryClipFunction);
protected:

    Ui::SurfaceCutView m_ui;

    mitk::DataInteractor::Pointer m_freehandCutInteractor;
    CutImplementation* m_pImplementation;

    mitk::DataInteractor::Pointer m_boundingShapeInteractor;
    mitk::DataInteractor::Pointer m_geometryInteractor;

    int m_boxNumber;

    std::map< std::string, vtkSmartPointer<vtkImplicitFunction> > m_geometryClipFunctions;

};

#endif // SurfaceCutView_h__