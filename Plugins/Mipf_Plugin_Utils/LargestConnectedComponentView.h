#ifndef LargestConnectedComponentView_h__
#define LargestConnectedComponentView_h__

#include "MitkPluginView.h"
#include <QWidget>

#include "ui_LargestConnectedComponentExtractView.h"

class QmitkDataStorageComboBox;

class LargestConnectedComponentView :public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    LargestConnectedComponentView();
    ~LargestConnectedComponentView() {}
    void CreateView();
protected slots:
    void Extract();


private:
    Ui::LargestConnectedComponentExtractView m_ui;
    
};

#endif // UtilView_h__