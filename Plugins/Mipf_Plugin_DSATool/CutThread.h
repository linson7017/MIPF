#ifndef CutThread_h__
#define CutThread_h__
/********************************************************************
	FileName:    CutThread
	Author:        Ling Song
	Date:           Month 4 ; Year 2018
	Purpose:	     
*********************************************************************/

#include <QObject>
#include <QFileInfoList>
#include "iqf_subject.h"
#include "iqf_main.h"
#include <mitkImage.h>
#include "ITKImageTypeDef.h"

class IQF_DSATool;
class CutThread : public QObject
{
    Q_OBJECT
public:
    CutThread(QF::IQF_Main* pMain);
    ~CutThread();
    QF::IQF_Subject* GetSubject() { return m_pSubject; }
    template <class PixelType>
    mitk::Image::Pointer CutThread::CutImage(mitk::Image* mitkImage);

public slots:
void Start(const QFileInfoList& lis,const QString& openDir, const QString& saveDir);

signals:
void SignalEnd();
void SignalLog(const QString& logStr,const QString& currentFileName);
void SignalCurrentResult(Int2DImageType* img);

private:
    void ProcessAndSaveDSA(QString openDir, QString saveDir);
private:
    IQF_DSATool* m_pDSATool;
    QF::IQF_Main* m_pMain;
    QF::IQF_Subject* m_pSubject;

};

#endif // CutThread_h__
