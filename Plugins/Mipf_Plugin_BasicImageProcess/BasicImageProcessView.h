#ifndef BasicImageProcessView_h__
#define BasicImageProcessView_h__

#include "MitkPluginView.h"
#include <QWidget>

#include "ui_QmitkBasicImageProcessingViewControls.h"

#include "QmitkStepperAdapter.h"

#include <mitkDataStorageSelection.h>

class BasicImageProcessView : public QWidget,public MitkPluginView
{
    Q_OBJECT
public:
    BasicImageProcessView();
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);

    void CreateView();

    virtual void SetFocus() ;

    /*!
    \brief method for creating the connections of main and control widget
    */
    virtual void CreateConnections();

    /*!
    \brief Invoked when the DataManager selection changed
    */
    void OnSelectionChanged(const QList<mitk::DataNode::Pointer>& nodes) ;


    protected slots:

    /*
    * When an action is selected in the "one image ops" list box
    */
    void SelectAction(int action);

    /*
    * When an action is selected in the "two image ops" list box
    */
    void SelectAction2(int operation);

    /*
    * The "Execute" button in the "one image ops" box was triggered
    */
    void StartButtonClicked();

    /*
    * The "Execute" button in the "two image ops" box was triggered
    */
    void StartButton2Clicked();

    /*
    *  Switch between the one and the two image operations GUI
    */
    void ChangeGUI();

    void SelectInterpolator(int interpolator);

private:

    /*
    * After a one image operation, reset the "one image ops" panel
    */
    void ResetOneImageOpPanel();

    /*
    * Helper method to reset the parameter set panel
    */
    void ResetParameterPanel();

    /*
    * After a two image operation, reset the "two image ops" panel
    */
    void ResetTwoImageOpPanel();

    /** retrieve the tnc from renderwindow part */
    void InternalGetTimeNavigationController();

    /*!
    * controls containing sliders for scrolling through the slices
    */
    Ui::QmitkBasicImageProcessingViewControls *m_Controls;

    //mitk::DataNode*       m_SelectedImageNode;
    mitk::DataStorageSelection::Pointer m_SelectedImageNode;
    QmitkStepperAdapter*      m_TimeStepperAdapter;

    enum ActionType {
        NOACTIONSELECTED,
        CATEGORY_DENOISING,
        GAUSSIAN,
        MEDIAN,
        TOTALVARIATION,
        CATEGORY_MORPHOLOGICAL,
        DILATION,
        EROSION,
        OPENING,
        CLOSING,
        CATEGORY_EDGE_DETECTION,
        GRADIENT,
        LAPLACIAN,
        SOBEL,
        CATEGORY_MISC,
        THRESHOLD,
        INVERSION,
        DOWNSAMPLING,
        FLIPPING,
        RESAMPLING,
        RESCALE,
        RESCALE2
    } m_SelectedAction;

    enum OperationType {
        TWOIMAGESNOACTIONSELECTED,
        CATEGORY_ARITHMETIC,
        ADD,
        SUBTRACT,
        MULTIPLY,
        DIVIDE,
        RESAMPLE_TO,
        CATEGORY_BOOLEAN,
        AND,
        OR,
        XOR
    } m_SelectedOperation;

    enum InterpolationType {
        LINEAR,
        NEAREST
    } m_SelectedInterpolation;
};

#endif // BasicImageProcessView_h__