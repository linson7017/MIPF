#include "ProgressBarWidget.h"

#include "mitkProgressBar.h"
#include "mitkRenderingManager.h"

#include <QVBoxLayout>

ProgressBarWidget::ProgressBarWidget()
{
    m_TotalSteps = 0;
    m_Progress = 0;
   // this->hide();
    this->SetPercentageVisible(true);
    setValue(0);


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
    //this->reset();
   // this->hide();
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

    mitk::RenderingManager::GetInstance()->ExecutePendingRequests();
}

void ProgressBarWidget::SlotProgress(unsigned int steps)
{
    m_Progress += steps;
    this->setValue(m_Progress);
    if (m_Progress >= m_TotalSteps)
        Reset();

    mitk::RenderingManager::GetInstance()->ExecutePendingRequests();

}
void ProgressBarWidget::SlotSetPercentageVisible(bool visible)
{
    setVisible(visible);
}