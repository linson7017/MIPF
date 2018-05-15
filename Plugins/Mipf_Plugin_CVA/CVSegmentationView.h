#ifndef CVSegmentationView_h__ 
#define CVSegmentationView_h__ 
 
#include "MitkPluginView.h" 

#include <QWidget>
#include <imageAlgorithm.h>

#include "mitkPointSet.h"

#include "ui_CVSegmentationView.h"
 
class IQF_MitkPointList;
class CVSegmentationView :public QWidget, public MitkPluginView  
{  
    Q_OBJECT
public:   
    CVSegmentationView(); 
    ~CVSegmentationView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;

    protected slots:
    virtual void slotExit();
   // virtual void resliceMode(int);
    virtual void resetViews();
  //  virtual void render();
    virtual void measureDistance0Changed(int) {}
    virtual void measureDistance1Changed(int) {}
    virtual void measureDistance2Changed(int) {}
    virtual void measureAngle0Changed(int) {}
    virtual void measureAngle1Changed(int) {}
    virtual void measureAngle2Changed(int) {}

    virtual void addDistanceMeasurementToView(int);
    virtual void removeDistanceMeasurementToView(int i){}
    virtual void addAngleMeasurementToView(int);
    virtual void removeAngleMeasurementToView(int i){}

    virtual void openFolder(){}
    virtual void displayAbout();
    virtual void displayVTKLicense();
    virtual void minThresholdValueChanged(int);
    virtual void maxThresholdValueChanged(int);
    virtual void thresholdValueChanged();
    virtual void generateSurface();
    virtual void removeSurface();
    virtual void saveSurface();
    virtual void loadSurface();
    virtual void autoSelectRange();

    virtual void sliceVisibilityChanged(int state);
    virtual void vesselVisibilityChanged(int state);
    virtual void needlePathVisibilityChanged(int state);
    virtual void aneurysmVisibilityChanged(int state);

    virtual void addSeedPoint();
    virtual void removeSeedPoint();
    virtual void resetSeedPoints();
    virtual void setStartPoint(bool );
    virtual void setEndPoint(bool);
    virtual void generateNeedle();
    virtual void generateMould();
    virtual void saveMould();
    virtual void pruneVesselTree();
    virtual void generateRectSurface();
    virtual void generateExtensions();
    virtual void generateVesselWallSurface();
    virtual void wallThicknessValueChanged(int);
    virtual void pruneRadiusValueChanged(int);
    virtual void vesselnessThresholdChanged(int);

    virtual void addAneurysmPoint(bool);
    virtual void setAneurysmLocationPoint1(bool);
    virtual void setAneurysmLocationPoint2(bool);
    virtual void detectAneurysm();

    void OnImageSelectionChanged(const mitk::DataNode* node);
private:
    void setThresholds();
    void toggleUIState();
    void Update(const char* szMessage, int iValue /* = 0 */, void* pValue /* = 0 */);
private:
    Ui::CVSegmentationView m_ui;

    ImageAlgorithm*  algorithm;
    double lowerth;
    double upperth;

    mitk::PointSet::Pointer m_seedPoints;
    mitk::DataNode::Pointer m_seedPointsNode;


    IQF_MitkPointList* m_pPointList;
    mitk::DataNode::Pointer m_startPointsNode;
    mitk::DataNode::Pointer m_endPointsNode;
    mitk::DataNode::Pointer m_aneurysmPointsNode;
    mitk::DataNode::Pointer m_aneurysmLocationPoint1Node;
    mitk::DataNode::Pointer m_aneurysmLocationPoint2Node;
};
#endif // CVSegmentationView_h__ 