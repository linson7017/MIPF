#ifndef DSAReader_h__ 
#define DSAReader_h__ 
 
#include "MitkPluginView.h" 
#include <QWidget>
#include "ui_DSAReader.h"
 

class QTimer;
class DSAReader : public QWidget,public MitkPluginView  
{  
public:   
    DSAReader(); 
    ~DSAReader();
    void CreateView() override;
    WndHandle GetPluginHandle() override;
protected slots:
void Load();
void ConfirmOriginFrame();
void ConfirmTargetFrame();
void Cut();
void Compare();
void RefreshSlice();
void EnableAnimation(bool enable);

private:
    template <class PixelType>
    mitk::Image::Pointer CutImage(mitk::Image* mitkImage);
private:
    Ui::DSAReader m_ui;
    QTimer* m_timer;
};
#endif // DSAReader_h__ 