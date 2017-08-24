#ifndef PluginView_h__
#define PluginView_h__
#include "iqf_observer.h"
#include "MitkMain/mitk_main_msg.h"
#include "UIs/QF_Plugin.h"

#include "iqf_main.h"

namespace QF
{
    class IQF_Main;
}
class R;

class PluginView :public QF::QF_Plugin,  public QF::IQF_Observer
{
public:
    PluginView() :m_pMain(0), m_pR(0), m_bActivated(false)
    {
    }
    PluginView(QF::IQF_Main* pMain) :m_pMain(pMain),m_pR(0), m_bActivated(false) 
    {
    }
    //override
    virtual void SetMainPtr(QF::IQF_Main* pMain)
    {
        m_pMain = pMain;
    }
	virtual void InitResource(R* pR)
	{
		m_pR = pR; 
		CreateView(); 
		m_bActivated = true;
	}
	virtual void Activate() { m_bActivated = true; }
	virtual void Disactivate() { m_bActivated = false; }
	void SetActivated(bool bActivated) { m_bActivated = bActivated; }
	bool IsActivated() { return m_bActivated; }
    void* GetInterfacePtr(const char* szInterfaceID)
    {
        if (m_pMain)
        {
            return m_pMain->GetInterfacePtr(szInterfaceID);
        }
        else
        {
            return nullptr;
        }       
    }
protected:
    //override
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0) {}
    virtual void CreateView() {};
protected:
    QF::IQF_Main* m_pMain;
    R* m_pR;
	bool m_bActivated;
};

#endif // PluginView_h__
