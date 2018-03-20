#include "PluginView.h"

void PluginView::SetMainPtr(QF::IQF_Main* pMain)
{
    m_pMain = pMain;
}

void PluginView::InitResource()
{
    CreateView();
    m_bActivated = true;
}

void* PluginView::GetInterfacePtr(const char* szInterfaceID)
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
const char* PluginView::GetAttribute(const char* attributeName)
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
bool PluginView::HasAttribute(const char* attributeName)
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