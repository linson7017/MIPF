#include "StatusBarWidget.h"
#include <qmainwindow.h>
#include <qstatusbar.h>
#include <qapplication.h>
#include <qdesktopwidget.h>

#include <mitkStatusBar.h>

#include <mitkStatusBar.h>


StatusBarWidget::StatusBarWidget() 
{
    m_GreyValueLabel = new QLabel(this, nullptr);
    int xResolution = QApplication::desktop()->screenGeometry(0).width() - 100;
    m_GreyValueLabel->setMaximumSize(QSize(xResolution, 50));
    m_GreyValueLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    this->addPermanentWidget(m_GreyValueLabel);
}

StatusBarWidget::~StatusBarWidget()
{
}

void StatusBarWidget::Init()
{
    mitk::StatusBar::GetInstance()->SetImplementation(this);
}
/**
* Display the text in the statusbar of the applikation
*/
void StatusBarWidget::DisplayText(const char* t)
{
    this->showMessage(t);
    // TODO bug #1357
    //qApp->processEvents(); // produces crashes!
}

/**
* Display the text in the statusbar of the applikation for ms seconds
*/
void StatusBarWidget::DisplayText(const char* t, int ms)
{
    this->showMessage(t, ms);
    // TODO bug #1357
    //qApp->processEvents(); // produces crashes!
}
/**
* Show the grey value text in the statusbar
*/
void StatusBarWidget::DisplayGreyValueText(const char* t)
{
    QString text(t);
    m_GreyValueLabel->setText(text);
}
/**
* Clear the text in the StatusBar
*/
void StatusBarWidget::Clear()
{
    this->clearMessage();
    // TODO bug #1357
    //qApp->processEvents(); // produces crashes!
}

/**
* enable or disable the QSizeGrip
*/
void StatusBarWidget::SetSizeGripEnabled(bool enable)
{
    this->setSizeGripEnabled(enable);
}
