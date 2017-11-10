#ifndef SFLSSegmentationView_h__
#define SFLSSegmentationView_h__

#include "MitkPluginView.h"
#include <QWidget>
#include "SFLSRobustStatSegmentor3DLabelMap_single.h"

#include "ui_WxAutoSegmentationViewControls.h"

#include "ITKImageTypeDef.h"

#include <QThread>

namespace mitk
{
    class  PaintbrushTool;
}
class IQF_MitkSegmentationTool;

class SFLSSegmentationView : public QWidget,public MitkPluginView
{
    Q_OBJECT
public:
    SFLSSegmentationView();
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
    virtual void SetFocus();

    void CreateView();
    WndHandle GetPluginHandle() { return this; }
public slots:
    void ApplySegment();
    void StopSegment();

    void BeginPaint();
    void BeginWipe();
    void EndTool();
    void PenSizeChanged(double size);
    void CreateNewSeedImage();
    void PresetChanged(const QString& text);
    void ObserveChanged(bool checked);

    void SlotSegmentationFinished();
    void SlotInteractionEnd(const itk::Image<float, 3>::Pointer& image, unsigned int currentInteraction);
signals:
    void SignalDoSegmentation();
    void SignalStopSegmentation();

protected:

    Ui::WxAutoSegmentationViewControls* m_Controls;

    void ItkImageRSSSegmentation(Float3DImageType* itkImage);
    void ChangeTool(const char* toolName);

    mitk::DataNode::Pointer m_ObserveNode;
    CSFLSRobustStatSegmentor3DLabelMap<float>* m_pSegmentation;
    QThread* m_segmentationThread;


    mitk::DataNode::Pointer m_seedImageNode;
    mitk::PaintbrushTool* m_tool;
    bool m_bPaintForeground;
    IQF_MitkSegmentationTool* m_pMitkSegTool;
    bool m_bToolInited;

    int m_observeInterval;
};

#endif // SFLSSegmentationView_h__