#include "DSARemoveBoneView.h" 
#include "iqf_main.h"  
#include <QFileDialog>

#include "mitkImageCast.h"


#include "ITKImageTypeDef.h"
#include "ITK_Helpers.h"  

//qt
#include <QDebug>

#include "CVA/IQF_DSATool.h"
#include "cva/cva_command_def.h"

DSARemoveBoneView::DSARemoveBoneView() :MitkPluginView() 
{
}
 
DSARemoveBoneView::~DSARemoveBoneView() 
{
}

WndHandle DSARemoveBoneView::GetPluginHandle()
{
    return this;
}

 
void DSARemoveBoneView::CreateView()
{
    m_ui.setupUi(this);

    connect(m_ui.OpenBtn, &QPushButton::clicked, this, &DSARemoveBoneView::OpenDir);
    connect(m_ui.SaveBtn, &QPushButton::clicked, this, &DSARemoveBoneView::SaveDir);
    connect(m_ui.StartBtn, &QPushButton::clicked, this, &DSARemoveBoneView::StartCut);
    connect(m_ui.MarkBtn, &QPushButton::clicked, this, &DSARemoveBoneView::Mark);
    connect(m_ui.DeleteBtn, &QPushButton::clicked, this, &DSARemoveBoneView::DeleteMark);
    connect(m_ui.ExportBtn, &QPushButton::clicked, this, &DSARemoveBoneView::Export);


    m_pCut = new CutThread(m_pMain);
    m_pCut->GetSubject()->Attach(this);
    qRegisterMetaType<QFileInfoList>("QFileInfoList");

} 

QFileInfoList GetDirList(QString path)
{
    QDir dir(path);
    QFileInfoList dir_list = dir.entryInfoList(QStringList()<<"*.DSA",QDir::Dirs);
    QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs);
    folder_list.removeFirst();
    folder_list.removeFirst();
    foreach (QFileInfo info,folder_list)
    {   
        QString name = info.absoluteFilePath();
        QFileInfoList child_dir_list = GetDirList(name);
        dir_list.append(child_dir_list);
    }

    return dir_list;
}

void DSARemoveBoneView::Mark()
{
    if (!QDir().exists(m_CurrentDSAFileName) || m_CurrentDSAFileName.isEmpty())
    {
        return;
    }
    if ((m_ui.MarkedDataList->findItems(m_CurrentDSAFileName, Qt::MatchExactly).size()!=0))
    {
        m_ui.LogTE->setHtml(m_ui.LogTE->toHtml()+QString("<font color=blue weight=bold size=4>%1 is already marked!</font>").arg(m_CurrentDSAFileName));
        return;
    }
    QListWidgetItem* item = new QListWidgetItem(m_CurrentDSAFileName);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setSelected(true);
    m_ui.MarkedDataList->addItem(item);
}

void DSARemoveBoneView::DeleteMark()
{
    m_ui.MarkedDataList->takeItem(m_ui.MarkedDataList->currentRow());
}

void DSARemoveBoneView :: Export()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Log"),
        "",
        tr("log (*.txt)"));

    QFile file(fileName);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        m_ui.LogTE->setHtml(m_ui.LogTE->toHtml() + QString("<font color=red weight=bold size=4>Open file %1 failed !</font>").arg(fileName));
        return;
    }
    QTextStream out(&file);
    for (int i=0;i<m_ui.MarkedDataList->count();i++)
    {
        out << m_ui.MarkedDataList->item(i)->text() << "\n";
    }
    file.close();
    m_ui.LogTE->setHtml(m_ui.LogTE->toHtml() + QString("<font color=green weight=bold size=4>Save log file %1 successfully !</font>").arg(fileName));
}

void DSARemoveBoneView :: OpenDir()
{
    QString defaultOpenFilePath = GetMitkReferenceInterface()->GetString("LastOpenDirectory");
    QString dirName = QFileDialog::getExistingDirectory(NULL, "Open Dicom Dir",
        defaultOpenFilePath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dirName.isEmpty())
        return;
    m_ui.OpenDirLE->setText(dirName);
    GetMitkReferenceInterface()->SetString("LastOpenDirectory", dirName.toLocal8Bit().constData());
}

void DSARemoveBoneView::SaveDir()
{
    QString defaultOpenFilePath = GetMitkReferenceInterface()->GetString("LastOpenDirectory");
    QString dirName = QFileDialog::getExistingDirectory(NULL, "Open Dicom Dir",
        defaultOpenFilePath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dirName.isEmpty())
        return;
    m_ui.SaveDirLE->setText(dirName);
}

void DSARemoveBoneView::StartCut()
{
    if (m_ui.SaveDirLE->text().isEmpty()||m_ui.OpenDirLE->text().isEmpty())
    {
        MITK_ERROR << "Please set the open directory and save directory!";
        return;
    }
    m_ui.LogTE->clear();   
    m_ui.MarkedDataList->clear();
    QFileInfoList list = GetDirList(m_ui.OpenDirLE->text());

    if (list.size()==0)
    {
        m_ui.LogTE->setHtml(m_ui.LogTE->toHtml()+"<font color=blue weight=bold size=4>The subdirectores do not contain *.DSA directory !</font>");
        return;
    }


    m_thread = new QThread;
    m_pCut->moveToThread(m_thread);
    disconnect(m_thread, &QThread::finished,
        m_thread, &QThread::deleteLater);
    disconnect(m_thread, &QThread::finished,
        m_pCut, &QThread::deleteLater);

    connect(m_thread, &QThread::finished,
        m_thread, &QThread::deleteLater);
    connect(m_thread, &QThread::finished,
        m_pCut, &QThread::deleteLater);

    GetDataStorage()->Remove(GetDataStorage()->GetSubset(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))));
    if (!m_ObserveNode)
    {
        m_ObserveNode = mitk::DataNode::New();
        m_ObserveNode->SetName("Observer");
        GetDataStorage()->Add(m_ObserveNode);
    }


    connect(this, &DSARemoveBoneView::SignalStart, m_pCut, &CutThread::Start);
    connect(m_pCut, &CutThread::SignalEnd, this, &DSARemoveBoneView::SlotEnd, Qt::BlockingQueuedConnection);
    connect(m_pCut, &CutThread::SignalLog, this, &DSARemoveBoneView::SlotLog);
    connect(m_pCut, &CutThread::SignalCurrentResult, this, &DSARemoveBoneView::SlotUpdateCurrentResult, Qt::BlockingQueuedConnection);


    // setEnabled(false);
    m_thread->start();
    emit SignalStart(list, m_ui.OpenDirLE->text(), m_ui.SaveDirLE->text());
    m_ui.StartBtn->setEnabled(false);
    m_ui.StartBtn->setText("Cutting...");
}

void DSARemoveBoneView::SlotEnd()
{
    m_ui.StartBtn->setEnabled(true);
    m_ui.StartBtn->setText("Start");
    disconnect(this, &DSARemoveBoneView::SignalStart, m_pCut, &CutThread::Start);
    disconnect(m_pCut, &CutThread::SignalEnd, this, &DSARemoveBoneView::SlotEnd);
    disconnect(m_pCut, &CutThread::SignalLog, this, &DSARemoveBoneView::SlotLog);
    disconnect(m_pCut, &CutThread::SignalCurrentResult, this, &DSARemoveBoneView::SlotUpdateCurrentResult);
    GetDataStorage()->Remove(m_ObserveNode);
    m_ObserveNode = NULL;
    RequestRenderWindowUpdate();
    m_ui.LogTE->setHtml(m_ui.LogTE->toHtml()+"<font color=green weight=bold size=4>Finished !</font>");
}

  
void DSARemoveBoneView::SlotUpdateCurrentResult(Int2DImageType* img)
{
    mitk::Image::Pointer mitkImage;
    mitk::CastToMitkImage<Int2DImageType>(img, mitkImage);
    m_ObserveNode->SetData(mitkImage);
    m_ObserveNode->SetBoolProperty("binary", false);
    m_ObserveNode->SetColor(1.0, 1.0, 1.0);
    m_ObserveNode->SetOpacity(1.0);
    mitk::LevelWindow lw(0, 1000);
    m_ObserveNode->SetLevelWindow(lw);
    GetMitkRenderWindowInterface()->Reinit(m_ObserveNode);
}

void DSARemoveBoneView::SlotLog(const QString& logStr, const QString& currentFileName)
{
    m_ui.LogTE->setHtml(m_ui.LogTE->toHtml()+logStr);
    m_CurrentDSAFileName = currentFileName;
}

void DSARemoveBoneView::Update(const char* szMessage, int iValue /* = 0 */, void* pValue /* = 0 */)
{
      if (strcmp("Message_DSACut_Log", szMessage)==0)
      {
      }
}

