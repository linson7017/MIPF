#ifndef PluginView_h__
#define PluginView_h__
#include "UIs/QF_Plugin.h"

#include "iqf_main.h"

namespace QF
{
    class IQF_Main;
}
class R;

class QF_API PluginView :public QF::QF_Plugin
{
public:
    PluginView() :m_pMain(0), m_bActivated(false)
    {
    }
    PluginView(QF::IQF_Main* pMain) :m_pMain(pMain), m_bActivated(false) 
    {
    }
    //override
    virtual void SetMainPtr(QF::IQF_Main* pMain);
    virtual void InitResource();
    virtual void SetupResource() {}
	virtual void Activate() { m_bActivated = true; }
	virtual void Disactivate() { m_bActivated = false; }
    virtual WndHandle GetPluginHandle() { return nullptr; }
	void SetActivated(bool bActivated) { m_bActivated = bActivated; }
	bool IsActivated() { return m_bActivated; }
    void* GetInterfacePtr(const char* szInterfaceID);
    const char* GetAttribute(const char* attributeName);
    bool HasAttribute(const char* attributeName);
protected:
    //override
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0) {}
    virtual void CreateView() {};
protected:
    QF::IQF_Main* m_pMain;
	bool m_bActivated;
};

#endif // PluginView_h__
