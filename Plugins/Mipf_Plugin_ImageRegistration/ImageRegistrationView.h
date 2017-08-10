#ifndef ImageRegistrationView_h__
#define ImageRegistrationView_h__

#include "MitkPluginView.h"
#include "mitkPointSet.h"
#include "mitkDataNode.h"
#include "Vector3.h"
#include <QObject>
#include "ITKImageTypeDef.h"
#include <QMatrix4x4>
#include <mitkEventConfig.h>
#include <usServiceReference.h>
#include "IndicateDlg.h"

#include "QfResult.h"

class QPushButton;
class QLineEdit;

//class QmitkPointListWidget;
class QmitkDataStorageComboBox;
class RegistrationWorkStation;

class ImageRegistrationView :public QObject,public MitkPluginView
{
    Q_OBJECT
public:
    ImageRegistrationView(QF::IQF_Main* pMain);
    ~ImageRegistrationView();
    void InitResource(R* pR);
    void ResourceConstructed(R* pR);
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
protected slots:
    void OnFixedImageSelectionChanged(const mitk::DataNode* node);
    void OnMovingImageSelectionChanged(const mitk::DataNode* node);

    void SlotRegistrationIterationEnd(const itk::Matrix<double, 4, 4>& result);
    void SlotRegistrationFinished(const QfResult& result);
    void SlotReslutImageGenerated(const Float3DImagePointerType resultImage);

signals:
    void SignalDoRegistration(const Float3DImagePointerType fixedImage, const Float3DImagePointerType movingImage, QMatrix4x4 initTransformMatrix);
    void SignalStopRegistration();
private:
    void InitRegistration(Float3DImageType* itkFixedImage, Float3DImageType* itkMovingImage);
    void DoRegistration();
    void EndRegistration();
    void Reset();
    void Stop();
    
    void UpdataRegistrationText(const vtkMatrix4x4& matrix);

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

    QMatrix4x4 m_registrationMatrix;

    bool m_bInited;

    mitk::DataInteractor::Pointer m_movingImageInteractor;
    std::map<us::ServiceReferenceU, mitk::EventConfig> m_DisplayInteractorConfigs;
    std::string m_EventConfig;
    bool m_ScrollEnabled;
};


#endif // SliceBySliceTrackingView_h__
