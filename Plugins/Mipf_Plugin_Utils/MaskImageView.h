#ifndef MaskImageView_h__
#define MaskImageView_h__

#pragma once

#include "MitkPluginView.h"

class QmitkDataStorageComboBox;

class MaskImageView : public MitkPluginView
{
public:
    MaskImageView(QF::IQF_Main* pMain);
    ~MaskImageView();

    void Constructed(R* pR);
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);

private:
    QmitkDataStorageComboBox* m_pImageSelector;
    QmitkDataStorageComboBox* m_pMaskSelector;
};

#endif // MaskImageView_h__


