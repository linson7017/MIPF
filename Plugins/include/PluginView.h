#ifndef PluginView_h__
#define PluginView_h__
#include "iqf_observer.h"
#include "MitkMain/mitk_main_msg.h"

namespace QF
{
    class IQF_Main;
}
class R;

class PluginView : public QF::IQF_Observer
{
public:
    PluginView(QF::IQF_Main* pMain) :m_pMain(pMain),m_pR(0){}
    //override
    void InitResource(R* pR) { m_pR = pR; }
protected:
    //override
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0) {}
    virtual void CreateView() {};
protected:
    QF::IQF_Main* m_pMain;
    R* m_pR;
};

#endif // PluginView_h__
