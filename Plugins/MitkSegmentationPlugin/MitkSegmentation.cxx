#include "MitkSegmentation.h"
#include "iqf_main.h"
#include "Res/R.h"

//qt
#include <QMessageBox>

//qmitk
#include "QmitkStdMultiWidget.h"
#include "QmitkNewSegmentationDialog.h"
//mitk
#include <mitkSurfaceToImageFilter.h>

#include "mitkVtkResliceInterpolationProperty.h"

#include "mitkApplicationCursor.h"
#include "mitkSegmentationObjectFactory.h"
#include "mitkCameraController.h"
#include "mitkLabelSetImage.h"

//#include "QmitkSegmentationOrganNamesHandling.cpp"


MitkSegmentation::MitkSegmentation(QF::IQF_Main* pMain) :MitkPluginView(pMain),
     m_MouseCursorSet(false)
    , m_Parent(NULL)
    , m_MultiWidget(NULL)
    , m_DataSelectionChanged(false)
{
    m_pMain->Attach(this);

    mitk::NodePredicateDataType::Pointer isDwi = mitk::NodePredicateDataType::New("DiffusionImage");
    mitk::NodePredicateDataType::Pointer isDti = mitk::NodePredicateDataType::New("TensorImage");
    mitk::NodePredicateDataType::Pointer isQbi = mitk::NodePredicateDataType::New("QBallImage");
    mitk::NodePredicateOr::Pointer isDiffusionImage = mitk::NodePredicateOr::New(isDwi, isDti);
    isDiffusionImage = mitk::NodePredicateOr::New(isDiffusionImage, isQbi);
    m_IsOfTypeImagePredicate = mitk::NodePredicateOr::New(isDiffusionImage, mitk::TNodePredicateDataType<mitk::Image>::New());

    m_IsBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    m_IsNotBinaryPredicate = mitk::NodePredicateNot::New(m_IsBinaryPredicate);

    m_IsNotABinaryImagePredicate = mitk::NodePredicateAnd::New(m_IsOfTypeImagePredicate, m_IsNotBinaryPredicate);
    m_IsABinaryImagePredicate = mitk::NodePredicateAnd::New(m_IsOfTypeImagePredicate, m_IsBinaryPredicate);

    m_IsASegmentationImagePredicate = mitk::NodePredicateOr::New(m_IsABinaryImagePredicate, mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());
    m_IsAPatientImagePredicate = mitk::NodePredicateAnd::New(m_IsNotABinaryImagePredicate, mitk::NodePredicateNot::New(mitk::TNodePredicateDataType<mitk::LabelSetImage>::New()));
}

void MitkSegmentation::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "") == 0)
    {
        //do what you want for the message
    }
}

void MitkSegmentation::CreateView()
{

}

