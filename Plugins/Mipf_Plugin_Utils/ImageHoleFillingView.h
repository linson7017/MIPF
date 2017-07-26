#ifndef ImageHoleFillingView_h__
#define ImageHoleFillingView_h__

#pragma once
#include "MitkPluginView.h"

#include "ui_ImageHoleFillingView.h"

class QmitkDataStorageComboBox;
class ImageHoleFillingView :public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    ImageHoleFillingView();
    ~ImageHoleFillingView();
    void CreateView();
protected slots:
    void FillHoles();

protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
private:
    Ui::Form m_ui;
};

#endif // ImageHoleFillingView_h__
