#ifndef ImageCropperWidget_h__
#define ImageCropperWidget_h__

#include "MitkPluginView.h"
#include <QWidget>

class QmitkDataStorageComboBox;
class QPushButton;


#pragma once
class ImageCropperWidget : public QWidget, public MitkPluginView
{
public:
    ImageCropperWidget(QF::IQF_Main* pMain);
    ~ImageCropperWidget();
    void Init(QWidget* parent);
    void InitResource(R* pR);
private:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);

    QmitkDataStorageComboBox* boundingShapeSelector;
    QPushButton* buttonCreateNewBoundingBox;
    QPushButton* buttonCropping;
    QPushButton* buttonMasking;
};

#endif // ImageCropperWidget_h__
