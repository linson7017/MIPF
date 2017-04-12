#ifndef NDIView_h__
#define NDIView_h__

#include "PluginView.h"
#include <QtWidgets>

#include "WxNDIPolarisVega.h"
#include "WxLandMarkRegistration.h"
#include "WxAutoExtractLandMarkFromMR.h"
#include "WxPointSetAutomaticPairing.h"
#include "SphereFit.h"

#include <vtkMatrix4x4.h>

//mitk
#include <mitkDataStorage.h>
#include <mitkImage.h>
#include <mitkProperties.h>
#include <mitkNodePredicateDataType.h>
#include <mitkDataNodeFactory.h>
//#include <mitkGeometry2DDataMapper2D.h>
#include <mitkPointSet.h>

#include "QmitkPointListWidget.h"
#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"

#include <QList>

class NDIView :public QWidget, public PluginView
{
    Q_OBJECT
public:
    NDIView(QF::IQF_Main* pMain);
    void InitResource(R* pR);
    virtual void SetFocus();
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);

private:
    QmitkStdMultiWidget* m_MultiView;

    WxNDIPolarisVega*							 m_NDIComm;
    WxLandMarkRegistration*					 m_LandmarkRegistration;
    WxAutoExtractLandMarkFromMR*		 m_AutoExtractLandMark;
    QMatrix4x4           m_PunctureNeedleMatrix;
    QMatrix4x4           m_PunctureNeedleAxisMatrix;
    QMatrix4x4           m_PunctureNeedleRegistrationOffsetMatrix;

    QMatrix4x4           m_ReferenceMatrix;
    QMatrix4x4           m_RegistrationMatrix;
    mitk::DataNode*				                  m_PlanNode;
    mitk::DataNode::Pointer                   m_PunctureNeedleNode;
    mitk::DataNode::Pointer                   m_AuroraProbeNode;
    mitk::DataNode::Pointer                   m_CalculatedNode;

    mitk::Point3D									  m_NDIAuroraPointerPosition;

    QTimer*                                            m_Timer;

    mitk::PointSet::Pointer					    m_MMIPointSet;
    mitk::DataNode::Pointer						m_MMIPointSetNode;

    std::vector<mitk::Point3D>                  m_SubjectPointSet;
    QList<QVector3D>                            m_ImageLandmarkList;
    QList<QVector3D>                            m_SubjectLandmarkList;


    QQueue<QVector3D> m_TipsPositionSet;

    SphereFit m_fit;

private:

    bool Update6DOFModelQMatrix4x4(const mitk::BaseData* datanode, const QMatrix4x4 &transform);
    mitk::DataStorage::Pointer GetDataStorage();
    //¼ÆËã±ê×¼²î
    void CaculateVariance(const QQueue<QVector3D>& positionSet,double& variance);
private slots:

    void OnStartTracker(bool start);
    void OnPauseTracker(bool pause);
    void OnUpdateNavigationData();
    void OnSelectSubjectPoint();
    void OnClearSubjectPoint();
    void OnAutoGetImagePoint();
    void OnTemplateMatch();
    void LoadNavigationToolsModel();
    void RemoveNavigationToolsModel();

    void OnFitCenter(bool start);

private:
    QPushButton* pbtnAutoGetImagePoint;
    QPushButton* pbtnTemplateMatch;
    QPushButton* pbtnClearSubjectPoint;
    QPushButton* pbtnSelectSubjectPoint;
    QPushButton* pbtnPauseTracker;
    QPushButton* pbtnStartTracker;

    QmitkPointListWidget* wdgtMMIPointSet;
    QListWidget* lstwgtSubjectPointSet;
    QmitkDataStorageComboBox* m_SelecetedImage;

    QTextEdit* txtedtRegistrationMatrix;

    QDoubleSpinBox* dspbxPixelUnit;
    QDoubleSpinBox* dspbxThresholdMax;
    QDoubleSpinBox* dspbxThresholdMin;



    QLineEdit* tipOffsetX;
    QLineEdit* tipOffsetY;
    QLineEdit* tipOffsetZ;
    QLineEdit* varianceValue;

    QPushButton* fitCenterBtn;

    QLineEdit* currentRobotPointX;
    QLineEdit* currentRobotPointY;
    QLineEdit* currentRobotPointZ;

    QLineEdit* currentNdiPointX;
    QLineEdit* currentNdiPointY;
    QLineEdit* currentNdiPointZ;

    bool m_robotArrived;

    QList<QVector3D> m_robotPoints;
    QList<QVector3D> m_ndiPoints;

    QVector3D m_currentNdiPosition;
    QMatrix4x4 m_registrationMatrix;
    QVector3D m_targetPosition;

};



#endif // NDIView_h__