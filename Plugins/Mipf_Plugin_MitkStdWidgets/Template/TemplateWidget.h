#ifndef TemplateWidget_h__
#define TemplateWidget_h__

#pragma once

#include "MitkPluginView.h"
#include <QWidget>


class TemplateWidget : public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    TemplateWidget(QF::IQF_Main* pMain);
    ~TemplateWidget();
    void Init(QWidget* parent);
    void InitResource(R* pR);
protected:
	virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
};

#endif // TemplateWidget_h__
