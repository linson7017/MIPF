#include "TestView.h"
#include "iqf_main.h"
#include "Res / R.h"
TestView::TestView(QF::IQF_Main* pMain) :PluginView(pMain)
{
    m_pMain->Attach(this);
}

void TestView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, ) == 0)
    {
        //do what you want for the message
    }
}