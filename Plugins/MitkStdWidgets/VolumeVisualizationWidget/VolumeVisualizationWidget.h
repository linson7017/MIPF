#ifndef VolumeVisualizationWidget_h__
#define VolumeVisualizationWidget_h__


#include "MitkPluginView.h"

#include <QWidget>
#include <mitkImage.h>
#include <mitkPointSet.h>
#include <mitkStandaloneDataStorage.h>
#include <itkImage.h>


class QCheckBox;
class QLabel;
class QComboBox;
class QmitkTransferFunctionWidget;
class QmitkTransferFunctionGeneratorWidget;

class QmitkStdMultiWidget;
class IQF_MitkDataManager;

class VolumeVisualizationWidget : public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    VolumeVisualizationWidget(QF::IQF_Main* pMain);
    ~VolumeVisualizationWidget() {}
    virtual void Init(QWidget* parent);
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);

protected slots:
    void OnMitkInternalPreset(int mode);
    void OnEnableRendering(bool state);
    void OnEnableLOD(bool state);
    void OnRenderMode(int mode);

private:
    mitk::WeakPointer<mitk::DataNode> m_SelectedNode;
    void UpdateInterface();
protected:
    IQF_MitkDataManager* m_DataManager;


    QLabel* m_SelectedImageLabel;
    QLabel* m_ErrorImageLabel;
    QLabel* m_NoSelectedImageLabel;
    QCheckBox* m_EnableRenderingCB;
    QCheckBox* m_EnableLOD;
    QComboBox* m_RenderModeComboBox;
    QmitkTransferFunctionWidget* m_TransferFunctionWidget;
    QmitkTransferFunctionGeneratorWidget* m_TransferFunctionGeneratorWidget;
};
#endif // VolumeVisualizationWidget_h__
