/********************************************************************
	FileName:    ProgressBarWidget.h
	Author:        Ling Song
	Date:           Month 9 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef ProgressBarWidget_h__
#define ProgressBarWidget_h__

#include "mitkProgressBarImplementation.h"

#include "QRoundProgressBar.h"
#include "QProgressBar"

class ProgressBarWidget :public QProgressBar, public mitk::ProgressBarImplementation
{
    Q_OBJECT
public:
    ProgressBarWidget();
    ~ProgressBarWidget();

    virtual void SetPercentageVisible(bool visible);

    virtual void AddStepsToDo(unsigned int steps);

    virtual void Progress(unsigned int steps);

signals:

    void SignalAddStepsToDo(unsigned int steps);
    void SignalProgress(unsigned int steps);
    void SignalSetPercentageVisible(bool visible);

    protected slots:

    virtual void SlotAddStepsToDo(unsigned int steps);
    virtual void SlotProgress(unsigned int steps);
    virtual void SlotSetPercentageVisible(bool visible);

private:
    //##Documentation
    //## @brief Reset the progress bar. The progress bar "rewinds" and shows no progress.
    void Reset() override;

   int m_TotalSteps;

    int m_Progress;

};

#endif // ProgressBarWidget_h__