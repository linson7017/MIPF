#ifndef RSSSegmentationView_h__ 
#define RSSSegmentationView_h__ 
 
#include "MitkPluginView.h" 
#include "ui_RSSSegmentationView.h"

class IQF_RSSSegmentation;
 
class RSSSegmentationView :public QWidget, public MitkPluginView  
{  
    Q_OBJECT
public:   
    RSSSegmentationView(); 
    ~RSSSegmentationView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;
protected slots:
    void Apply();
private:
    Ui::RSSSegmentationView m_ui;

    IQF_RSSSegmentation* m_pSegmentor;
};
#endif // RSSSegmentationView_h__ 