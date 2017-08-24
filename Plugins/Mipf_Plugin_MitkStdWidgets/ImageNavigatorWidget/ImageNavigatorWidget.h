#ifndef ImageNavigatorWidget_h__
#define ImageNavigatorWidget_h__


#include "MitkPluginView.h"

#include <QWidget>
#include <mitkImage.h>
#include <mitkPointSet.h>
#include <mitkStandaloneDataStorage.h>
#include <itkImage.h>


class QCheckBox;
class QLabel;
class QDoubleSpinBox;

class QmitkSliderNavigatorWidget;
class QmitkStepperAdapter;
class QmitkStdMultiWidget;
class IQF_MitkDataManager;

class ImageNavigatorWidget : public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    ImageNavigatorWidget();
    ~ImageNavigatorWidget() {}
    void CreateView() override;
    virtual void Init(QWidget* parent);
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);

protected slots:
    void OnMillimetreCoordinateValueChanged();
    void OnRefetch();
    void UpdateStatusBar() {}
protected:
    void RenderWindowPartActivated();
    /*void SetFocus() override;
    void RenderWindowPartActivated(mitk::IRenderWindowPart *renderWindowPart) override;
    void RenderWindowPartDeactivated(mitk::IRenderWindowPart *renderWindowPart) override;
    void SetBorderColors();
    void SetBorderColor(QDoubleSpinBox *spinBox, QString colorAsStyleSheetString);
    void SetBorderColor(int axis, QString colorAsStyleSheetString);
    void SetStepSizes();
    void SetStepSize(int axis);
    void SetStepSize(int axis, double stepSize);
    int  GetClosestAxisIndex(mitk::Vector3D normal);*/

private:
    mitk::WeakPointer<mitk::DataNode> m_SelectedNode;
protected:
    IQF_MitkDataManager* m_DataManager;

    QLabel* m_AxialLabel;
    QLabel* m_CoronalLabel;
    QLabel* m_LocationLabel;
    QLabel* m_SagittalLabel;
    QLabel* m_TimeLabel;

    QDoubleSpinBox* m_XWorldCoordinateSpinBox;
    QDoubleSpinBox* m_YWorldCoordinateSpinBox;
    QDoubleSpinBox* m_ZWorldCoordinateSpinBox;

    QmitkSliderNavigatorWidget* m_SliceNavigatorAxial;
    QmitkSliderNavigatorWidget* m_SliceNavigatorFrontal;
    QmitkSliderNavigatorWidget* m_SliceNavigatorSagittal;
    QmitkSliderNavigatorWidget* m_SliceNavigatorTime;


    QmitkStepperAdapter* m_AxialStepper;
    QmitkStepperAdapter* m_SagittalStepper;
    QmitkStepperAdapter* m_FrontalStepper;
    QmitkStepperAdapter* m_TimeStepper;
};
#endif // VolumeVisualizationWidget_h__
