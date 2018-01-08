#ifndef ModelExporterView_h__ 
#define ModelExporterView_h__ 
 
#include "MitkPluginView.h" 
#include <QWidget>
#include "ui_ModelExporterView.h"
 

class VTKSceneViwer;
class ModelExporterView :public QWidget, public MitkPluginView  
{  
    Q_OBJECT
public:   
    ModelExporterView(); 
    ~ModelExporterView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;
protected slots:
    void Apply();
    void Import();

private:
    Ui::ModelExporterView m_ui;

    VTKSceneViwer* m_sceneViewer;
};
#endif // ModelExporterView_h__ 