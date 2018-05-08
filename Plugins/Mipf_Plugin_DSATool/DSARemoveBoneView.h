#ifndef DSARemoveBoneView_h__ 
#define DSARemoveBoneView_h__ 
 
#include "MitkPluginView.h" 
#include <QWidget>
#include "ui_DSARemoveBoneView.h"
 
#include <QFutureWatcher>
#include <QFuture>
#include <QtConcurrent>
#include "CutThread.h"

class QTimer;
class IQF_DSATool;
class DSARemoveBoneView : public QWidget,public MitkPluginView  
{  
    Q_OBJECT
public:   
    DSARemoveBoneView(); 
    ~DSARemoveBoneView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;
    void Update(const char* szMessage, int iValue /* = 0 */, void* pValue /* = 0 */);
signals:
    void SignalStart(const QFileInfoList& lis, const QString& openDir, const QString& saveDir);
protected slots:
void OpenDir();
void SaveDir();
void StartCut();
void Mark();
void DeleteMark();
void Export();
void SlotEnd();
void SlotLog(const QString& logStr, const QString& currentFileName);
void SlotUpdateCurrentResult(Int2DImageType* img);


private:
    Ui::DSARemoveBoneView m_ui;
    QThread* m_thread;
    CutThread* m_pCut;
    mitk::DataNode::Pointer m_ObserveNode;
    QString m_CurrentDSAFileName;
};
#endif // DSARemoveBoneView_h__ 