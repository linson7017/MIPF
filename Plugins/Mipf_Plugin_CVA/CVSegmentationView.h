#ifndef CVSegmentationView_h__ 
#define CVSegmentationView_h__ 
 
#include "MitkPluginView.h" 

#include <QWidget>
#include <imageAlgorithm.h>

#include "ui_CVSegmentationView.h"
 
class CVSegmentationView :public QWidget, public MitkPluginView  
{  
    Q_OBJECT
public:   
    CVSegmentationView(); 
    ~CVSegmentationView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;

    protected slots:
    void generateSurface();


    virtual void minThresholdValueChanged(int);
    virtual void maxThresholdValueChanged(int);
    virtual void thresholdValueChanged();
    virtual void autoSelectRange();
    void toggleUIState();

    virtual void pruneVesselTree();
    virtual void generateVesselWallSurface();
    virtual void wallThicknessValueChanged(int);
    virtual void pruneRadiusValueChanged(int);
    virtual void vesselnessThresholdChanged(int);
    virtual void addAneurysmPoint();
    void detectAneurysm();

    void OnImageSelectionChanged(const mitk::DataNode* node);
private:
    void setThresholds();
private:
    Ui::CVSegmentationView m_ui;

    ImageAlgorithm*  algorithm;
    double lowerth;
    double upperth;

};
#endif // CVSegmentationView_h__ 