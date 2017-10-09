#ifndef ASMView_h__ 
#define ASMView_h__ 
 
#include "MitkPluginView.h" 

#include "ui_ASMView.h"
#include <QWidget>
 

class ASMView : public  QWidget,public MitkPluginView
{  
    Q_OBJECT
public:   
    ASMView(); 
    ~ASMView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;
protected slots:
    void AlgorithmChanged(int);
private:
    Ui::ASMView m_ui;
};
#endif // ASMView_h__ 