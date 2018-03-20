#ifndef PluginView_h__
#define PluginView_h__
#include "UIs/QF_Plugin.h"

#include "iqf_main.h"

namespace QF
{
    class IQF_Main;
}
class R;

class PluginView :public QF::QF_Plugin
{
public:
    PluginView() :m_pMain(0), m_bActivated(false)
    {
    }
    PluginView(QF::IQF_Main* pMain) :m_pMain(pMain), m_bActivated(false) 
    {
    }
    //override
    virtual void SetMainPtr(QF::IQF_Main* pMain)
    {
        m_pMain = pMain;
    }
	virtual void InitResource()
	{
		CreateView(); 
		m_bActivated = true;
	}
    virtual void SetupResource() {}
	virtual void Activate() { m_bActivated = true; }
	virtual void Disactivate() { m_bActivated = false; }
    virtual WndHandle GetPluginHandle() { return nullptr; }
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
    const char* GetAttribute(const char* attributeName)
    {
        std::map<std::string, std::string>::iterator it = m_attributes.find(attributeName);
        if (it != m_attributes.end())
        {
            return it->second.c_str();
        }
        else
        {
            return "";
        }
    }
    bool HasAttribute(const char* attributeName)
    {
        if (m_attributes.count(attributeName))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
protected:
    //override
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0) {}
    virtual void CreateView() {};
protected:
    QF::IQF_Main* m_pMain;
	bool m_bActivated;
};

#endif // PluginView_h__
