#include "ProgressBarWidget.h"

#include "mitkProgressBar.h"
#include "mitkRenderingManager.h"

#include <QVBoxLayout>
#include <QApplication>
#include <QDesktopWidget>

ProgressBarWidget::ProgressBarWidget()
{
    m_TotalSteps = 0;
    m_Progress = 0;
    this->hide();
    this->SetPercentageVisible(true);
    setValue(0);

    setAttribute(Qt::WA_TranslucentBackground, false);

    connect(this, SIGNAL(SignalAddStepsToDo(unsigned int)), this, SLOT(SlotAddStepsToDo(unsigned int)));
    connect(this, SIGNAL(SignalProgress(unsigned int)), this, SLOT(SlotProgress(unsigned int)));
    connect(this, SIGNAL(SignalSetPercentageVisible(bool)), this, SLOT(SlotSetPercentageVisible(bool)));

    mitk::ProgressBar::GetInstance()->RegisterImplementationInstance(this);
}


ProgressBarWidget::~ProgressBarWidget()
{
    mitk::ProgressBar::GetInstance()->UnregisterImplementationInstance(this);
}

void ProgressBarWidget::SetPercentageVisible(bool visible)
{
    emit SignalSetPercentageVisible(visible);
}

void ProgressBarWidget::Reset()
{
    this->hide();
    this->reset();
    m_TotalSteps = 0;
    m_Progress = 0;
}

void ProgressBarWidget::AddStepsToDo(unsigned int steps)
{
    emit SignalAddStepsToDo(steps);
}

void ProgressBarWidget::Progress(unsigned int steps)
{
    emit SignalProgress(steps);
}


void ProgressBarWidget::SlotAddStepsToDo(unsigned int steps)
{
    m_TotalSteps += steps;
    this->setMaximum(m_TotalSteps);
    this->setValue(m_Progress);
   /* if (m_TotalSteps > 0)
    {
        this->showInCenterOfParent();
    }*/

    mitk::RenderingManager::GetInstance()->ExecutePendingRequests();
}

void ProgressBarWidget::SlotProgress(unsigned int steps)
{
    m_Progress += steps;
    this->setValue(m_Progress);
    if (m_Progress >= m_TotalSteps)
        Reset();
    else
    {
        this->showInCenterOfParent();
    }

    mitk::RenderingManager::GetInstance()->ExecutePendingRequests();

}
void ProgressBarWidget::SlotSetPercentageVisible(bool visible)
{
    setTextVisible(visible);
}

void ProgressBarWidget::showInCenterOfParent()
{
    if (parentWidget())
    {
        setGeometry((parentWidget()->width() - width()) / 2, (parentWidget()->height() - height()) / 2, width(), height());
        show();
    }
    else
    {
        move((QApplication::desktop()->width() - width()) / 2, (QApplication::desktop()->height() - height()) / 2);
        show();
    }
}