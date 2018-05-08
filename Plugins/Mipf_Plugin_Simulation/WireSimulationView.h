#ifndef WireSimulationView_h__ 
#define WireSimulationView_h__ 
 
#include "MitkPluginView.h"
#include <QWidget>

#include "ui_WireSimulationView.h"

#include "Physics2.h"

#include <QTimer>

class Scene;
class GuideWire;
class IQF_MitkPointList;
 
class WireSimulationView :public QWidget, public MitkPluginView  
{
    Q_OBJECT
public:   
    WireSimulationView(); 
    ~WireSimulationView();
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
    Ui::WireSimulationView m_ui;

    QTimer m_timer;

    IQF_MitkPointList* m_pEntrancePointList;
    IQF_MitkPointList* m_pDirectionPointList;

    GuideWire* m_scene;

    double m_timeStep;
};
#endif // WireSimulationView_h__ 