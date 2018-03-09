#ifndef GrowcutView_h__
#define GrowcutView_h__

#include "MitkPluginView.h"
#include "ITKImageTypeDef.h"

#include <QtConcurrent>
#include <QFutureWatcher>
#include <QFuture>
#include <QObject>

class QmitkDataStorageComboBox;
class IQF_FastGrowCutSegmentation;

namespace mitk
{
    class  PaintbrushTool;
}

class IQF_MitkSegmentationTool;

class GrowcutView : public QObject, public MitkPluginView
{
	Q_OBJECT
public:
    GrowcutView(QF::IQF_Main* pMain);
    ~GrowcutView();
	void Constructed();
	
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
    void Init();
    void ChangeTool(const char* toolName);
public slots:
	void DoSomeThing();
	void GrowcutFinished();
    void SwitchToForeground();
    void SwitchToBackground();
private:
	QmitkDataStorageComboBox* m_pSourceImageSelector;

	IQF_FastGrowCutSegmentation *pFGC;

	bool m_bInitializationFlag;
	bool PaintFlag;
	void DoSegmentation(vtkImageData* SrcImage, vtkImageData* LabImage);

	QFutureWatcher<void> m_watcher;
	QFuture<void> m_future;

	//mitk::DataNode::Pointer m_result;
	//mitk::Image::Pointer m_labelimage;
    mitk::DataNode::Pointer m_seedImageNode;

    mitk::PaintbrushTool* m_tool;
    bool m_bPaintForeground;
    IQF_MitkSegmentationTool* m_pMitkSegTool;
    bool m_bInited;
};
#endif // GrowcutView_h__
