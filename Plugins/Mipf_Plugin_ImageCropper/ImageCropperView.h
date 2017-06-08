#ifndef ImageCropperView_h__
#define ImageCropperView_h__

#include "MitkPluginView.h"

#include <QProgressDialog>
#include "QVTKWidget.h"
#include "QmitkRegisterClasses.h"

#include "itkCommand.h"
#include <itkImage.h>
#include <itksys/SystemTools.hxx>

#include <vtkEventQtSlotConnect.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>

#include <mitkBoundingShapeInteractor.h>
#include <mitkDataStorage.h>
#include <mitkEventConfig.h>
#include <mitkGeometryData.h>
#include <mitkPointSet.h>
#include <mitkWeakPointer.h>

#include "ui_ImageCropperControls.h"

class ImageCropperView :public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    ImageCropperView(QF::IQF_Main* pMain);
    ~ImageCropperView();
    void CreateView();
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);

    protected slots:
    /*!
    * @brief Creates a new bounding object
    */
    virtual void DoCreateNewBoundingObject();
    /*!
    * @brief Whenever Crop button is pressed, issue a cropping action
    */
    void DoCropping();
    /*!
    * @brief Whenever Mask button is pressed, issue a masking action
    */
    void DoMasking();
    /*!
    * @brief Dis- or enable the advanced setting section
    */
    void OnAdvancedSettingsButtonToggled();
    /*!
    * @brief Updates current selection of the bounding object
    */
    void OnDataSelectionChanged(const mitk::DataNode* node);
    /*!
    * @brief Sets the scalar value for outside pixels in case of masking
    */
    void OnSliderValueChanged(int slidervalue);

protected:

    virtual void SetFocus();
    /*!
    @brief called by QmitkFunctionality when DataManager's selection has changed
    */
    void OnSelectionChanged(const QList<mitk::DataNode::Pointer>& nodes);
    /*!
    @brief Sets the selected bounding object as current bounding object and set up interactor
    */
    void OnComboBoxSelectionChanged(const mitk::DataNode* node);
    /*!
    * @brief Initializes a new bounding shape using the selected image geometry.
    */
    mitk::Geometry3D::Pointer InitializeWithImageGeometry(mitk::BaseGeometry::Pointer geometry);

    void CreateBoundingShapeInteractor(bool rotationEnabled);

private:
    /*!
    * @brief A pointer to the node of the image to be cropped.
    */
    mitk::WeakPointer<mitk::DataNode> m_ImageNode;
    /*!
    * @brief The cuboid used for cropping.
    */
    mitk::GeometryData::Pointer m_CroppingObject;

    /*!
    * @brief Tree node of the cuboid used for cropping.
    */
    mitk::DataNode::Pointer m_CroppingObjectNode;

    /*!
    * @brief Interactor for moving and scaling the cuboid
    */
    mitk::BoundingShapeInteractor::Pointer m_BoundingShapeInteractor;

    void ProcessImage(bool crop);

    // cropping parameter
    mitk::ScalarType m_CropOutsideValue;
    bool m_Advanced;
    bool m_Active;
    bool m_ScrollEnabled;

    Ui::ImageCropperControls m_Controls;
};

#endif // ImageCropperView_h__