#ifndef StatusBarWidget_h__
#define StatusBarWidget_h__
#include <mitkStatusBarImplementation.h>
#include <mitkCommon.h>

#include <QStatusBar>
#include <QLabel>

#pragma once
class StatusBarWidget : public QStatusBar, public mitk::StatusBarImplementation
{
public:
    mitkClassMacro(StatusBarWidget, mitk::StatusBarImplementation)
    StatusBarWidget();
    ~StatusBarWidget();

    virtual void DisplayText(const char* t) override;
    virtual void DisplayText(const char* t, int ms) override;

    //##Documentation
    //## @brief Send a string as an error message to StatusBar.
    //## The implementation calls DisplayText()
    virtual void DisplayErrorText(const char *t) override { this->DisplayText(t); };
    virtual void DisplayWarningText(const char *t) override { this->DisplayText(t); };
    virtual void DisplayWarningText(const char *t, int ms) override { this->DisplayText(t, ms); };
    virtual void DisplayGenericOutputText(const char *t) override { this->DisplayText(t); }
    virtual void DisplayDebugText(const char *t) override { this->DisplayText(t); };
    virtual void DisplayGreyValueText(const char *t) override;

    virtual void Clear() override;

    //##Documentation
    //## @brief Set the QSizeGrip of the window
    //## (the triangle in the lower right Windowcorner for changing the size)
    //## to enabled or disabled
    virtual void SetSizeGripEnabled(bool enable) override;

private:
    //static Pointer m_Instance;
    QLabel* m_GreyValueLabel;
};

#endif // StatusBarWidget_h__
