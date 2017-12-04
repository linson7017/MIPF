#ifndef PCLView_h__ 
#define PCLView_h__ 
 
#include "MitkPluginView.h" 
#include <QWidget>

#include "ui_PCLView.h"
 
class PCLView :public QWidget, public MitkPluginView  
{  
    Q_OBJECT
public:   
    PCLView(); 
    ~PCLView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;

protected slots:
    void Apply();
private:
    Ui::PCLView m_ui;
};
#endif // PCLView_h__ 