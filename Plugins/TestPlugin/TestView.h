#ifndef TestView_h__
#define TestView_h__

#include "PluginView.h"

class TestView : public PluginView
{
public:
    TestView(QF::IQF_Main* pMain);
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
}

#endif // TestView_h__