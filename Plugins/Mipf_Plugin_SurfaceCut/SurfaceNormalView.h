#ifndef SurfaceNormalView_h__
#define SurfaceNormalView_h__

#include "MitkPluginView.h"

#include <QWidget>

#include "ui_SurfaceNormalView.h"

class vtkImplicitFunction;

class CutImplementation;

class SurfaceNormalView :public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    SurfaceNormalView();
    ~SurfaceNormalView();
    void CreateView() override;
    WndHandle GetPluginHandle() { return this; }
protected slots:
void Apply();
private:
    void TestPointNormals(vtkPolyData* polydata);
    void TestCellNormals(vtkPolyData* polydata);

    bool GetPointNormals(vtkPolyData* polydata);
    bool GetCellNormals(vtkPolyData* polydata);
    Ui::SurfaceNormalView m_ui;

};

#endif // SurfaceNormalView_h__