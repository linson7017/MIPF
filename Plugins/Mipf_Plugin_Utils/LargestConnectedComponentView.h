#ifndef LargestConnectedComponentView_h__
#define LargestConnectedComponentView_h__

#include "MitkPluginView.h"

class QmitkDataStorageComboBox;

class LargestConnectedComponentView : public MitkPluginView
{
public:
    LargestConnectedComponentView(QF::IQF_Main* pMain);
    ~LargestConnectedComponentView() {}
    void Constructed(R* pR);
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);



private:
    QmitkDataStorageComboBox* m_pImageSelector;
};

#endif // UtilView_h__