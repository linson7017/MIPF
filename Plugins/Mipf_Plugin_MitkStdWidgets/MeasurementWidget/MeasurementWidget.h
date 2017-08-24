#ifndef MeasurementWidget_h__
#define MeasurementWidget_h__

#pragma once

#include "MitkPluginView.h"
#include <QWidget>
#include <mitkDataStorage.h>

#include "usServiceRegistration.h"
struct QmitkMeasurementViewData;
namespace mitk
{
    class PlanarFigure;
}

class MeasurementWidget : public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    MeasurementWidget();
    ~MeasurementWidget();
    void CreateView() override;
    void Init(QWidget* parent);
    void SetupWidget(R* pR);
public:
    void NodeAdded(const mitk::DataNode* node);
    void NodeChanged(const mitk::DataNode* node);
    void NodeRemoved(const mitk::DataNode* node);
    void SelectionChanged(const QList<mitk::DataNode::Pointer>& nodes);

    void PlanarFigureSelected(itk::Object* object, const itk::EventObject&);
protected slots:
    void OnDrawLineTriggered(bool checked = false);
    void OnDrawPathTriggered(bool checked = false);
    void OnDrawAngleTriggered(bool checked = false);
    void OnDrawFourPointAngleTriggered(bool checked = false);
    void OnDrawCircleTriggered(bool checked = false);
    void OnDrawEllipseTriggered(bool checked = false);
    void OnDrawDoubleEllipseTriggered(bool checked = false);
    void OnDrawRectangleTriggered(bool checked = false);
    void OnDrawPolygonTriggered(bool checked = false);
    void OnDrawBezierCurveTriggered(bool checked = false);
    void OnDrawSubdivisionPolygonTriggered(bool checked = false);
    void OnCopyToClipboard(bool checked = false);

private:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
    void CreateConnections();
    mitk::DataNode::Pointer AddFigureToDataStorage(mitk::PlanarFigure* figure, const QString& name);
    void UpdateMeasurementText();
    void AddAllInteractors();
    mitk::DataNode::Pointer DetectTopMostVisibleImage();
    void EnableCrosshairNavigation();
    void DisableCrosshairNavigation();
    void PlanarFigureInitialized();
    void CheckForTopMostVisibleImage(mitk::DataNode* nodeToNeglect = nullptr);
    mitk::DataStorage::SetOfObjects::ConstPointer GetAllPlanarFigures() const;

private:
    QmitkMeasurementViewData* d;
    std::map<us::ServiceReferenceU, mitk::EventConfig> m_DisplayInteractorConfigs;
};

#endif // MeasurementWidget_h__
