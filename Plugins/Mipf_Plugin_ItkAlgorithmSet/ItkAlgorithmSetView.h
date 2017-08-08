#ifndef SliceBySliceTrackingView_h__
#define SliceBySliceTrackingView_h__

#include "MitkPluginView.h"
#include <QtWidgets>

#include "ui_ItkAlgorithmSetView.h"

class ItkAlgorithmSetView :public QWidget,public MitkPluginView
{
    Q_OBJECT
public:
    ItkAlgorithmSetView();
    void CreateView();

protected slots:
    void OnAlgorithmChanged(const QString &text);

    void OnVesselEnhance();
private:
    Ui::ItkAlgorithmSetView m_ui;


    QPushButton* m_VEDBtn;
};


#endif // SliceBySliceTrackingView_h__
