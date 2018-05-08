#ifndef WireSimulationPBDView_h__ 
#define WireSimulationPBDView_h__ 
 
#include "MitkPluginView.h"
#include <QWidget>

#include "ui_WireSimulationPBDView.h"

#include "Common/Common.h"
#include "Demos/Visualization/Selection.h"
#include "Demos/Simulation/ParticleData.h"
#include <Eigen/Dense>
#include "PBD/PositionBasedElasticRodsModel.h"
#include "PBD/PositionBasedElasticRodsConstraints.h"
#include "PBD/PositionBasedElasticRodsTSC.h"

#include "Demos/Simulation/DistanceFieldCollisionDetection.h"

#include <QTimer>

class Scene;
class GuideWire;
class IQF_MitkPointList;
 
class WireSimulationPBDView :public QWidget, public MitkPluginView  
{
    Q_OBJECT
public:   
    WireSimulationPBDView(); 
    ~WireSimulationPBDView();
    void CreateView() override;
    void InitializeScene();
    WndHandle GetPluginHandle() override;
    void Update(const char* szMessage, int iValue /* = 0 */, void* pValue /* = 0 */);
protected slots:
    void Apply();
    void Refresh();
    void SelectEntrancePoint(bool select);
    void SelectDirectionPoint(bool select);
    void ClearEntrancePoint();
    void ClearDirectionPoint();
    void EnableDynamic(bool);
private:
    void RefreshData();
    void createRod(const mitk::Point3D& entrancePoint, const mitk::Vector3D& headingDirection);

    Ui::WireSimulationPBDView m_ui;

    QTimer m_timer;

    IQF_MitkPointList* m_pEntrancePointList;
    IQF_MitkPointList* m_pDirectionPointList;

    PBD::PositionBasedElasticRodsModel model;
    PBD::PositionBasedElasticRodsTSC sim;
    PBD::DistanceFieldCollisionDetection cd;

    int numberOfPoints;
    bool doPause;
    std::vector<unsigned int> selectedParticles;

    double m_timeStep;
};
#endif // WireSimulationPBDView_h__ 