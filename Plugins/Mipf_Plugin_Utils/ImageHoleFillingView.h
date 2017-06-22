#ifndef ImageHoleFillingView_h__
#define ImageHoleFillingView_h__

#pragma once
#include "MitkPluginView.h"

class QmitkDataStorageComboBox;
class ImageHoleFillingView : public MitkPluginView
{
public:
    ImageHoleFillingView(QF::IQF_Main* pMain);
    ~ImageHoleFillingView();
    void Constructed(R* pR);
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);

private:
    QmitkDataStorageComboBox* m_pImageSelector;
};

#endif // ImageHoleFillingView_h__
