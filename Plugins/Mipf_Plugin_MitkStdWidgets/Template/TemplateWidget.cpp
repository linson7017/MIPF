#include "TemplateWidget.h"

//qf
#include <MitkMain/IQF_MitkDataManager.h>
#include <MitkMain/IQF_MitkRenderWindow.h>
#include <MitkMain/IQF_MitkReference.h>
#include "iqf_main.h"
#include <Res/R.h>


TemplateWidget::TemplateWidget(QF::IQF_Main* pMain) :MitkPluginView(pMain)
{
    m_pMain->Attach(this);
    m_DataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr("QF_MitkMain_DataManager");
    m_DataManager->Init();
}

TemplateWidget::~TemplateWidget()
{
}

void TemplateWidget::InitResource(R* pR)
{
    PluginView::InitResource(pR);
    //create your view
}

void TemplateWidget::Init(QWidget* parent)
{
    m_Parent = parent;
}

void TemplateWidget::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "") == 0)
    {
	}
}