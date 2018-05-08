#include "ImageNavigatorWidget.h"

#include "QmitkStdMultiWidget.h"
#include "mitkRenderingManager.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkPointSet.h"
#include "mitkPointSetDataInteractor.h"
#include "mitkImageAccessByItk.h"
#include "mitkRenderingManager.h"
#include "mitkDataStorage.h"
#include "mitkTransferFunctionInitializer.h"


#include "QmitkSliderNavigatorWidget.h"
#include "QmitkStepperAdapter.h"

#include <mitkIOUtil.h>
#include <QtWidgets>


#include <itkSpatialOrientationAdapter.h>

#include <MitkMain/IQF_MitkDataManager.h>
#include <mitkMain/IQF_MitkRenderWindow.h>

#include "iqf_main.h"
#include "Utils/variant.h"


ImageNavigatorWidget::ImageNavigatorWidget():MitkPluginView()
,m_AxialStepper(0)
, m_SagittalStepper(0)
, m_FrontalStepper(0)
, m_TimeStepper(0)
{
    
}

void ImageNavigatorWidget::CreateView()
{
    Init(NULL);
}

void ImageNavigatorWidget::Update(const char* szMessage, int iValue, void* pValue)
{

}


void ImageNavigatorWidget::Init(QWidget* parent)
{
    m_pMain->Attach(this);
    QVBoxLayout* vLayout = new QVBoxLayout;
    setLayout(vLayout);

    m_AxialLabel = new QLabel("Axial");
    m_CoronalLabel = new QLabel("Coronal");
    m_LocationLabel = new QLabel("Location (mm)");
    m_SagittalLabel = new QLabel("Sagittal");
    m_TimeLabel = new QLabel("Time");


    m_XWorldCoordinateSpinBox = new QDoubleSpinBox;
    m_YWorldCoordinateSpinBox = new QDoubleSpinBox;
    m_ZWorldCoordinateSpinBox = new QDoubleSpinBox;

    m_SliceNavigatorAxial = new QmitkSliderNavigatorWidget;
    m_SliceNavigatorFrontal = new QmitkSliderNavigatorWidget;
    m_SliceNavigatorSagittal = new QmitkSliderNavigatorWidget;
    m_SliceNavigatorTime = new QmitkSliderNavigatorWidget;

    {
        QHBoxLayout* hLayout = new QHBoxLayout;
        hLayout->addWidget(m_LocationLabel);
        hLayout->addWidget(m_XWorldCoordinateSpinBox);
        hLayout->addWidget(m_YWorldCoordinateSpinBox);
        hLayout->addWidget(m_ZWorldCoordinateSpinBox);
        vLayout->addLayout(hLayout);
    }

    {
        QGridLayout* layout = new QGridLayout;
        layout->addWidget(m_AxialLabel,0,0);
        layout->addWidget(m_SagittalLabel, 1, 0);
        layout->addWidget(m_CoronalLabel, 2, 0);
        layout->addWidget(m_TimeLabel, 3, 0);

        layout->addWidget(m_SliceNavigatorAxial, 0, 1);
        layout->addWidget(m_SliceNavigatorSagittal, 1, 1);
        layout->addWidget(m_SliceNavigatorFrontal, 2, 1);
        layout->addWidget(m_SliceNavigatorTime, 3, 1);
        vLayout->addLayout(layout);
    }

    m_SliceNavigatorAxial->SetInverseDirection(true);
    connect(m_XWorldCoordinateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnMillimetreCoordinateValueChanged()));
    connect(m_YWorldCoordinateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnMillimetreCoordinateValueChanged()));
    connect(m_ZWorldCoordinateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnMillimetreCoordinateValueChanged()));

    setEnabled(true);
    RenderWindowPartActivated();
   
}


void ImageNavigatorWidget::OnMillimetreCoordinateValueChanged()
{
    IQF_MitkRenderWindow* pMitkRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
    if (pMitkRenderWindow&&pMitkRenderWindow->GetMitkStdMultiWidget())
    {
        mitk::TimeGeometry::ConstPointer geometry = pMitkRenderWindow->GetActiveMitkRenderWindow()->GetSliceNavigationController()->GetInputWorldTimeGeometry();

        if (geometry.IsNotNull())
        {
            mitk::Point3D positionInWorldCoordinates;
            positionInWorldCoordinates[0] = m_XWorldCoordinateSpinBox->value();
            positionInWorldCoordinates[1] = m_YWorldCoordinateSpinBox->value();
            positionInWorldCoordinates[2] = m_ZWorldCoordinateSpinBox->value();

          //  m_IRenderWindowPart->SetSelectedPosition(positionInWorldCoordinates);
            pMitkRenderWindow->GetMitkStdMultiWidget()->MoveCrossToPosition(positionInWorldCoordinates);
        }
    }
}

void ImageNavigatorWidget::RenderWindowPartActivated()
{
    IQF_MitkRenderWindow* pMitkRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
    if (pMitkRenderWindow)
    {
        QString multiViewsID = GetAttribute("MultiViewID");
        QmitkRenderWindow* renderWindow = pMitkRenderWindow->GetQmitkRenderWindow(multiViewsID +"-axial");
        if (renderWindow)
        {
            if (m_AxialStepper) m_AxialStepper->deleteLater();
            m_AxialStepper = new QmitkStepperAdapter(m_SliceNavigatorAxial,
                renderWindow->GetSliceNavigationController()->GetSlice(),
                "sliceNavigatorAxialFromSimpleExample");
            m_SliceNavigatorAxial->setEnabled(true);
            m_AxialLabel->setEnabled(true);
            m_ZWorldCoordinateSpinBox->setEnabled(true);
            connect(m_AxialStepper, SIGNAL(Refetch()), this, SLOT(OnRefetch()));
            connect(m_AxialStepper, SIGNAL(Refetch()), this, SLOT(UpdateStatusBar()));
        }
        else
        {
            m_SliceNavigatorAxial->setEnabled(false);
            m_AxialLabel->setEnabled(false);
            m_ZWorldCoordinateSpinBox->setEnabled(false);
        }

        renderWindow = pMitkRenderWindow->GetQmitkRenderWindow(multiViewsID + "-sagittal");
        if (renderWindow)
        {
            if (m_SagittalStepper) m_SagittalStepper->deleteLater();
            m_SagittalStepper = new QmitkStepperAdapter(m_SliceNavigatorSagittal,
                renderWindow->GetSliceNavigationController()->GetSlice(),
                "sliceNavigatorSagittalFromSimpleExample");
            m_SliceNavigatorSagittal->setEnabled(true);
            m_SagittalLabel->setEnabled(true);
            m_YWorldCoordinateSpinBox->setEnabled(true);
            connect(m_SagittalStepper, SIGNAL(Refetch()), this, SLOT(OnRefetch()));
            connect(m_SagittalStepper, SIGNAL(Refetch()), this, SLOT(UpdateStatusBar()));
        }
        else
        {
            m_SliceNavigatorSagittal->setEnabled(false);
            m_SagittalLabel->setEnabled(false);
            m_YWorldCoordinateSpinBox->setEnabled(false);
        }

        renderWindow = pMitkRenderWindow->GetQmitkRenderWindow(multiViewsID + "-coronal");
        if (renderWindow)
        {
            if (m_FrontalStepper) m_FrontalStepper->deleteLater();
            m_FrontalStepper = new QmitkStepperAdapter(m_SliceNavigatorFrontal,
                renderWindow->GetSliceNavigationController()->GetSlice(),
                "sliceNavigatorFrontalFromSimpleExample");
            m_SliceNavigatorFrontal->setEnabled(true);
            m_CoronalLabel->setEnabled(true);
            m_XWorldCoordinateSpinBox->setEnabled(true);
            connect(m_FrontalStepper, SIGNAL(Refetch()), this, SLOT(OnRefetch()));
            connect(m_FrontalStepper, SIGNAL(Refetch()), this, SLOT(UpdateStatusBar()));
        }
        else
        {
            m_SliceNavigatorFrontal->setEnabled(false);
            m_CoronalLabel->setEnabled(false);
            m_XWorldCoordinateSpinBox->setEnabled(false);
        }

        if (renderWindow)
        {
            mitk::SliceNavigationController* timeController = renderWindow->GetRenderer()->GetRenderingManager()->GetTimeNavigationController();
            if (timeController)
            {
                if (m_TimeStepper) m_TimeStepper->deleteLater();
                m_TimeStepper = new QmitkStepperAdapter(m_SliceNavigatorTime,
                    timeController->GetTime(),
                    "sliceNavigatorTimeFromSimpleExample");
                m_SliceNavigatorTime->setEnabled(true);
                m_TimeLabel->setEnabled(true);
                connect(m_TimeStepper, SIGNAL(Refetch()), this, SLOT(UpdateStatusBar()));
            }
            else
            {
                m_SliceNavigatorTime->setEnabled(false);
                m_TimeLabel->setEnabled(false);
            }
        }
        
    }
}

void ImageNavigatorWidget::OnRefetch()
{
    if (GetMitkRenderWindowInterface())
    {
        mitk::BaseGeometry::ConstPointer geometry = GetMitkRenderWindowInterface()->GetActiveMitkRenderWindow()->GetSliceNavigationController()->GetInputWorldGeometry3D();
        mitk::TimeGeometry::ConstPointer timeGeometry = GetMitkRenderWindowInterface()->GetActiveMitkRenderWindow()->GetSliceNavigationController()->GetInputWorldTimeGeometry();

        if (geometry.IsNull() && timeGeometry.IsNotNull())
        {
            mitk::TimeStepType timeStep = GetMitkRenderWindowInterface()->GetActiveMitkRenderWindow()->GetSliceNavigationController()->GetTime()->GetPos();
            geometry = timeGeometry->GetGeometryForTimeStep(timeStep);
        }

        if (geometry.IsNotNull())
        {
            mitk::BoundingBox::BoundsArrayType bounds = geometry->GetBounds();

            mitk::Point3D cornerPoint1InIndexCoordinates;
            cornerPoint1InIndexCoordinates[0] = bounds[0];
            cornerPoint1InIndexCoordinates[1] = bounds[2];
            cornerPoint1InIndexCoordinates[2] = bounds[4];

            mitk::Point3D cornerPoint2InIndexCoordinates;
            cornerPoint2InIndexCoordinates[0] = bounds[1];
            cornerPoint2InIndexCoordinates[1] = bounds[3];
            cornerPoint2InIndexCoordinates[2] = bounds[5];

            if (!geometry->GetImageGeometry())
            {
                cornerPoint1InIndexCoordinates[0] += 0.5;
                cornerPoint1InIndexCoordinates[1] += 0.5;
                cornerPoint1InIndexCoordinates[2] += 0.5;
                cornerPoint2InIndexCoordinates[0] -= 0.5;
                cornerPoint2InIndexCoordinates[1] -= 0.5;
                cornerPoint2InIndexCoordinates[2] -= 0.5;
            }

            mitk::Point3D crossPositionInWorldCoordinates = GetMitkRenderWindowInterface()->GetMitkStdMultiWidget()->GetCrossPosition();

            mitk::Point3D cornerPoint1InWorldCoordinates;
            mitk::Point3D cornerPoint2InWorldCoordinates;

            geometry->IndexToWorld(cornerPoint1InIndexCoordinates, cornerPoint1InWorldCoordinates);
            geometry->IndexToWorld(cornerPoint2InIndexCoordinates, cornerPoint2InWorldCoordinates);

            m_XWorldCoordinateSpinBox->blockSignals(true);
            m_YWorldCoordinateSpinBox->blockSignals(true);
            m_ZWorldCoordinateSpinBox->blockSignals(true);

            m_XWorldCoordinateSpinBox->setMinimum(std::min(cornerPoint1InWorldCoordinates[0], cornerPoint2InWorldCoordinates[0]));
            m_YWorldCoordinateSpinBox->setMinimum(std::min(cornerPoint1InWorldCoordinates[1], cornerPoint2InWorldCoordinates[1]));
            m_ZWorldCoordinateSpinBox->setMinimum(std::min(cornerPoint1InWorldCoordinates[2], cornerPoint2InWorldCoordinates[2]));
            m_XWorldCoordinateSpinBox->setMaximum(std::max(cornerPoint1InWorldCoordinates[0], cornerPoint2InWorldCoordinates[0]));
            m_YWorldCoordinateSpinBox->setMaximum(std::max(cornerPoint1InWorldCoordinates[1], cornerPoint2InWorldCoordinates[1]));
            m_ZWorldCoordinateSpinBox->setMaximum(std::max(cornerPoint1InWorldCoordinates[2], cornerPoint2InWorldCoordinates[2]));

            m_XWorldCoordinateSpinBox->setValue(crossPositionInWorldCoordinates[0]);
            m_YWorldCoordinateSpinBox->setValue(crossPositionInWorldCoordinates[1]);
            m_ZWorldCoordinateSpinBox->setValue(crossPositionInWorldCoordinates[2]);

            m_XWorldCoordinateSpinBox->blockSignals(false);
            m_YWorldCoordinateSpinBox->blockSignals(false);
            m_ZWorldCoordinateSpinBox->blockSignals(false);

            /// Calculating 'inverse direction' property.

            mitk::AffineTransform3D::MatrixType matrix = geometry->GetIndexToWorldTransform()->GetMatrix();
            matrix.GetVnlMatrix().normalize_columns();
            mitk::AffineTransform3D::MatrixType::InternalMatrixType inverseMatrix = matrix.GetInverse();

            for (int worldAxis = 0; worldAxis < 3; ++worldAxis)
            {
                QString multiViewID = GetAttribute("MultiViewID");
                QmitkRenderWindow* renderWindow =
                    worldAxis == 0 ? GetMitkRenderWindowInterface()->GetQmitkRenderWindow(multiViewID+"-sagittal") :
                    worldAxis == 1 ? GetMitkRenderWindowInterface()->GetQmitkRenderWindow(multiViewID + "-coronal") :
                    GetMitkRenderWindowInterface()->GetQmitkRenderWindow(multiViewID + "-axial");

                if (renderWindow)
                {
                    const mitk::BaseGeometry* rendererGeometry = renderWindow->GetRenderer()->GetCurrentWorldGeometry();

                    /// Because of some problems with the current way of event signalling,
                    /// 'Modified' events are sent out from the stepper while the renderer
                    /// does not have a geometry yet. Therefore, we do a nullptr check here.
                    /// See bug T22122. This check can be resolved after T22122 got fixed.
                    if (rendererGeometry)
                    {
                        int dominantAxis = itk::Function::Max3(
                            inverseMatrix[0][worldAxis],
                            inverseMatrix[1][worldAxis],
                            inverseMatrix[2][worldAxis]);

                        bool referenceGeometryAxisInverted = inverseMatrix[dominantAxis][worldAxis] < 0;
                        bool rendererZAxisInverted = rendererGeometry->GetAxisVector(2)[worldAxis] < 0;

                        /// `referenceGeometryAxisInverted` tells if the direction of the corresponding axis
                        /// of the reference geometry is flipped compared to the 'world direction' or not.
                        ///
                        /// `rendererZAxisInverted` tells if direction of the renderer geometry z axis is
                        /// flipped compared to the 'world direction' or not. This is the same as the indexing
                        /// direction in the slice navigation controller and matches the 'top' property when
                        /// initialising the renderer planes. (If 'top' was true then the direction is
                        /// inverted.)
                        ///
                        /// The world direction can be +1 ('up') that means right, anterior or superior, or
                        /// it can be -1 ('down') that means left, posterior or inferior, respectively.
                        ///
                        /// If these two do not match, we have to invert the index between the slice navigation
                        /// controller and the slider navigator widget, so that the user can see and control
                        /// the index according to the reference geometry, rather than the slice navigation
                        /// controller. The index in the slice navigation controller depends on in which way
                        /// the reference geometry has been resliced for the renderer, and it does not necessarily
                        /// match neither the world direction, nor the direction of the corresponding axis of
                        /// the reference geometry. Hence, it is a merely internal information that should not
                        /// be exposed to the GUI.
                        ///
                        /// So that one can navigate in the same world direction by dragging the slider
                        /// right, regardless of the direction of the corresponding axis of the reference
                        /// geometry, we invert the direction of the controls if the reference geometry axis
                        /// is inverted but the direction is not ('inversDirection' is false) or the other
                        /// way around.

                        bool inverseDirection = referenceGeometryAxisInverted != rendererZAxisInverted;

                        QmitkSliderNavigatorWidget* navigatorWidget =
                            worldAxis == 0 ? m_SliceNavigatorSagittal :
                            worldAxis == 1 ? m_SliceNavigatorFrontal :
                            m_SliceNavigatorAxial;

                        navigatorWidget->SetInverseDirection(inverseDirection);

                        // This should be a preference (see T22254)
                        // bool invertedControls = referenceGeometryAxisInverted != inverseDirection;
                        // navigatorWidget->SetInvertedControls(invertedControls);
                    }
                }
            }
        }

       // this->SetBorderColors();

    }
}