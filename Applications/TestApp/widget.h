#ifndef widget_h__
#define widget_h__

#include <QWidget>

namespace QF
{
   class  IQF_Main;
}

#pragma once
class widget  : public QWidget
{
public:
    widget(QF::IQF_Main* pMain);
    ~widget();
};

#endif // widget_h__
