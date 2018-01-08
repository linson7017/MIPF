#ifndef DeepMedicView_h__ 
#define DeepMedicView_h__ 
 
#include "MitkPluginView.h" 
#include <QWidget>

#include "ui_DeepMedicView.h"
 
class DeepMedicView : public QWidget, public MitkPluginView  
{  
    Q_OBJECT
public:   
    DeepMedicView(); 
    ~DeepMedicView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;


protected slots:
    void Apply();

private:
    Ui::DeepMedicView m_ui;
};
#endif // DeepMedicView_h__ 