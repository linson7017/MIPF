#ifndef IndicateDlg_H_
#define IndicateDlg_H_

#include <QDialog>
#pragma once

class IndicateDlg : public QDialog
{
public:
    IndicateDlg()
    {
        //setMinimumSize(200, 100);
        setFixedSize(200, 100);
        setWindowFlags(Qt::CustomizeWindowHint | Qt::Dialog);
     //   setWindowOpacity(0.6);
    }
    ~IndicateDlg(){}
};



#endif // !1

