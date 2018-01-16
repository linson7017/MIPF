#ifndef VolumeCutView_h__ 
#define VolumeCutView_h__ 
 

#include <QWidget>
#include "MitkPluginView.h" 

#include "ui_VolumeCutView.h"

#include "mitkDataInteractor.h"
 
class CutImplementation;

class VolumeCutView : public  QWidget, public MitkPluginView  
{  
    Q_OBJECT
public:   
    VolumeCutView(); 
    ~VolumeCutView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;

protected slots:
void AddBox();
void RemoveBox();
void BoxCut();
void BoxSelected(QListWidgetItem *, QListWidgetItem *);
void ModelCut(bool b);
void Undo();
void Redo();

void InsideOutChanged(bool checked);

private:
    mitk::Geometry3D::Pointer InitializeWithSurfaceGeometry(mitk::BaseGeometry::Pointer geometry);


private:
    Ui::VolumeCutView m_ui;

    mitk::DataInteractor::Pointer m_freehandCutInteractor;
    CutImplementation* m_pImplementation;
    mitk::DataInteractor::Pointer m_boundingShapeInteractor;
    int m_boxNumber;
};
#endif // VolumeCutView_h__ 