#ifndef ImageRegistrationView_h__
#define ImageRegistrationView_h__

#include "MitkPluginView.h"
#include "mitkPointSet.h"
#include "mitkDataNode.h"
#include <QObject>
#include "ITKImageTypeDef.h"
#include <QMatrix4x4>
#include <mitkEventConfig.h>
#include <usServiceReference.h>
#include "IndicateDlg.h"

#include "QfResult.h"

#include "ui_ImageRegistrationView.h"

class QPushButton;
class QLineEdit;

//class QmitkPointListWidget;
class QmitkDataStorageComboBox;
class RegistrationWorkStation;

class ImageRegistrationView :public QWidget,public MitkPluginView
{
    Q_OBJECT
public:
    ImageRegistrationView();
    ~ImageRegistrationView();
    void CreateView() override;
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
protected slots:
    void OnFixedImageSelectionChanged(const mitk::DataNode* node);
    void OnMovingImageSelectionChanged(const mitk::DataNode* node);

    void SlotRegistrationIterationEnd(const itk::Matrix<double, 4, 4>& result);
    void SlotRegistrationFinished(const QfResult& result);
    void SlotReslutImageGenerated(const Float3DImagePointerType resultImage);

    void InitRegistration();
    void DoRegistration();
    void EndRegistration();
    void Reset();
    void Stop();

    void UpdataRegistrationText(const vtkMatrix4x4& matrix);

signals:
    void SignalDoRegistration(const Float3DImagePointerType fixedImage, const Float3DImagePointerType movingImage, vtkMatrix4x4* initTransformMatrix);
    void SignalStopRegistration();
private:
    

    void DisableDefaultInteraction();
    void EnableDefaultInteraction();

    mitk::PointSet::Pointer m_PointSet;
    QmitkDataStorageComboBox* m_FixedDataStorageComboBox;
    QmitkDataStorageComboBox* m_MovingDataStorageComboBox;
    mitk::DataNode* m_FixedImageNode;
    mitk::DataNode* m_MovingImageNode;

    RegistrationWorkStation* m_RegistrationWorkStation;
    QThread* m_RegistrationThread;

    vtkMatrix4x4* m_originMatrix;

    vtkMatrix4x4* m_registrationMatrix;

    bool m_bInited;

    mitk::DataInteractor::Pointer m_movingImageInteractor;
    std::map<us::ServiceReferenceU, mitk::EventConfig> m_DisplayInteractorConfigs;
    std::string m_EventConfig;
    bool m_ScrollEnabled;



    Ui::ImageRegistrationView  m_ui;
};


#endif // SliceBySliceTrackingView_h__
