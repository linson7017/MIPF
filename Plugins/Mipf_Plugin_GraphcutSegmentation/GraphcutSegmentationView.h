#ifndef GraphcutSegmentationView_h__
#define GraphcutSegmentationView_h__

#include "MitkPluginView.h"
//#include <GraphCut.h>

#include <mitkDataNode.h>
#include <mitkColorProperty.h>
#include <mitkImage.h>
#include <mitkPaintbrushTool.h>

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>
#include <QTimer>
#include <QtConcurrentRun>

#include <mitkTool.h>
#include "ITKImageTypeDef.h"
#include "Segmentation/IQF_GraphcutSegmentation.h"


#include "mitkSurfaceInterpolationController.h"

class QmitkDataStorageComboBox;
class QSlider;
class IQF_MitkSegmentationTool;

struct SegmentationOption
{
    void SetOrgan(const QString& szOrganType);
    QString Organ;
    double Lambda;
    float Color[3];
    double Level;
    double Window;
};

class GraphcutSegmentationView : public QObject, public MitkPluginView
{
    Q_OBJECT
public:
    GraphcutSegmentationView();
    void SetupResource();
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);

    void Init();
    void Segment();
    void SaveMask();

    mitk::DataNode::Pointer CreateSegmentationNode(mitk::Image* origin,const std::string& organName,const mitk::Color& color);

    void InitTool();
    void InitSourceAndSinkNodes();
    void ChangeTool(const QString& toolName);

    void SwitchToForeground();
    void SwitchToBackground();
    void RefreshGrapcutImage();
    void RefreshSourceAndSink();
    void RefreshROI();
    void EstimationSampleRate();


    void RefreshContourValueRange(double min,double max,double value);
    void Reset();

    void OnSurfaceInterpolationInfoChanged(const itk::EventObject &);

    void GenerateSurface();

    void ExtractROI();
    void CropImage();
protected slots:
    void OnImageSelectionChanged(const mitk::DataNode* node);
    void OnContourValueChanged(int value);

    void OnSurfaceInterpolationFinished();
private:
    //Down Sample Image


    QmitkDataStorageComboBox* m_imageComboBox;

    mitk::DataNode* m_refImageNode;
    mitk::DataNode* m_workImageNode;
    mitk::DataNode::Pointer m_sourceSinkNode;
    mitk::DataNode* m_currentResultNode;

    mitk::DataNode::Pointer m_resultSurfaceImageNode;
    itk::SmartPointer<Float3DImageType> m_originImage;

    //GraphCut<Int3DImageType> m_graphcut;
    IQF_GraphcutSegmentation* m_graphcut;

    //vtkSmartPointer<vtkInteractorStyleScribble> m_graphcutStyle;

    itk::ImageRegion<3> m_imageRegion;

    typedef std::set<itk::Index<3>, IndexSortCriterion > SetOfPixels;
    SetOfPixels m_sources;
    SetOfPixels m_sinks;
    SetOfPixels* m_selectedPixelSet;
    mitk::PaintbrushTool* m_tool;

    bool m_bInited;

    mitk::Image* m_originMitkImage;

    bool m_bPaintForeground;

    //InterpolationController
    void InitInterpolator(mitk::DataNode* imageNode);
    void Run3DInterpolation();
    void GeneratedInterpolatedSourceAndSink();
    void ConvertSurfaceToImage(mitk::Surface* surface, mitk::Image* referenceImage, mitk::Image* output);
    mitk::SurfaceInterpolationController::Pointer m_SurfaceInterpolator;
    unsigned int SurfaceInterpolationInfoChangedObserverTag;

    mitk::DataNode::Pointer m_InterpolatedSinkSurfaceNode;
    mitk::DataNode::Pointer m_InterpolatedSourceSurfaceNode;

    QFuture<void> m_Future;
    QFutureWatcher<void> m_Watcher;
    QTimer *m_Timer;

    QString m_currentResultName;

    IQF_MitkSegmentationTool* m_pSegTool;

    //crop tool
    mitk::DataNode::Pointer m_pCropObjectNode;
    mitk::GeometryData::Pointer m_pCropObject;

    double m_bSampleRate;

    SegmentationOption m_segOption;

    int m_roi[6];
};

#endif // GraphcutSegmentationView_h__