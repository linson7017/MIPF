#ifndef TestView_h__ 
#define TestView_h__ 
 
#include "MitkPluginView.h" 
 
#include <QWidget>
#include "ui_TestView.h"

class TestView :public QWidget, public MitkPluginView  
{  
public:   
    TestView(); 
    ~TestView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;
protected slots:
void Apply();
private:
    Ui::TestView m_ui;
};
#endif // TestView_h__ 