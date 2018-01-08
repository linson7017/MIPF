#ifndef ImageNormalizationView_h__ 
#define ImageNormalizationView_h__ 
 
#include "MitkPluginView.h" 
#include <QWidget>
#include "ui_ImageNormalizationView.h"
 
class ImageNormalizationView :public QWidget, public MitkPluginView  
{  
    Q_OBJECT
public:   
    ImageNormalizationView(); 
    ~ImageNormalizationView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;

protected slots:
    void AddFile();
    void RemoveFile();
    void Apply();

private:
    void NormalizeImage(const QString& filename,const QString& outputDir);
private:
    Ui::ImageNormalizationView m_ui;
};
#endif // ImageNormalizationView_h__ 