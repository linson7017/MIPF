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
    void ResourceContructed(R* pR);
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
protected slots:
    void OnFixedImageSelectionChanged(const mitk::DataNode* node);
    void OnMovingImageSelectionChanged(const mitk::DataNode* node);

    void SlotRegistrationIterationEnd(const itk::Matrix<double, 4, 4>& result);
    void SlotRegistrationFinished();


    void SlotMoveXAdd();
    void SlotMoveYAdd();
    void SlotMoveZAdd();
    void SlotMoveXSub();
    void SlotMoveYSub();
    void SlotMoveZSub();

    void SlotRotateXAdd();
    void SlotRotateYAdd();
    void SlotRotateZAdd();
    void SlotRotateXSub();
    void SlotRotateYSub();
    void SlotRotateZSub();

    void TranslateMovingImage(const QVector3D& translate);
    void RotateMovingImage(double angle, const QVector3D& normal);
signals:
    void SignalDoRegistration(const Float3DImagePointerType fixedImage, const Float3DImagePointerType movingImage, QMatrix4x4 initTransformMatrix);
    void SignalStopRegistration();
private:
    void InitRegistration(Float3DImageType* itkFixedImage, Float3DImageType* itkMovingImage);
    void EndRegistration();
    void Reset();
    void Stop();
    void RefreshMovingImage(/*QMatrix4x4& matrix*/);
    

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

    QLineEdit* m_leMoveStep;
    QPushButton* m_btnMoveXAdd;
    QPushButton* m_btnMoveYAdd;
    QPushButton* m_btnMoveZAdd;
    QPushButton* m_btnMoveXSub;
    QPushButton* m_btnMoveYSub;
    QPushButton* m_btnMoveZSub;

    QLineEdit* m_leRotateStep;
    QPushButton* m_btnRotateXAdd;
    QPushButton* m_btnRotateYAdd;
    QPushButton* m_btnRotateZAdd;
    QPushButton* m_btnRotateXSub;
    QPushButton* m_btnRotateYSub;
    QPushButton* m_btnRotateZSub;

    mitk::DataInteractor::Pointer m_movingImageInteractor;
    std::map<us::ServiceReferenceU, mitk::EventConfig> m_DisplayInteractorConfigs;
    std::string m_EventConfig;
    bool m_ScrollEnabled;
};


#endif // SliceBySliceTrackingView_h__
