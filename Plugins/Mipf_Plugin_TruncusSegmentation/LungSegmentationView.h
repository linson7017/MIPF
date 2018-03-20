#ifndef LungSegmentationView_h__ 
#define LungSegmentationView_h__ 
 
#include "MitkPluginView.h" 
#include "ui_LungSegmentationView.h"
 
class LungSegmentationView :public QWidget, public MitkPluginView  
{  
    Q_OBJECT
public:   
    LungSegmentationView(); 
    ~LungSegmentationView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;
protected slots:
    void Apply();
private:
    Ui::LungSegmentationView m_ui;
};
#endif // LungSegmentationView_h__ 