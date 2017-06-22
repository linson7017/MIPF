#ifndef GraphcutSegmentationView_h__
#define GraphcutSegmentationView_h__

#include "MitkPluginView.h"
#include <GraphCut.h>

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


#include "mitkSurfaceInterpolationController.h"

class QmitkDataStorageComboBox;
class QSlider;

class GraphcutSegmentationView : public QObject, public MitkPluginView
{
    Q_OBJECT
public:
    GraphcutSegmentationView(QF::IQF_Main* pMain);
    void Constructed(R* pR);
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);

    void Init();
    void Segment();
    void SaveMask();

    mitk::DataNode::Pointer CreateSegmentationNode(mitk::Image* origin,const std::string& organName,const mitk::Color& color);

    void ScribbleEventHandler(vtkObject* caller, long unsigned int eventId, void* callData);

    void UpdateSelections();

    void InitTool();
    void InitSourceAndSinkNodes();
    void ChangeTool(const QString& toolName);

    void SwitchToForeground();
    void SwitchToBackground();
    void RefreshGrapcutImage();
    void RefreshSourceAndSink();
    void RefreshROI();

    void GenerateSurface();
    void RefreshContourValueRange(double min,double max,double value);
    void Reset();

    void OnSurfaceInterpolationInfoChanged(const itk::EventObject &);

protected slots:
    void OnImageSelectionChanged(const mitk::DataNode* node);
    void OnSelectPoint(const QVector3D& pixelIndex);
    void OnContourValueChanged(int value);

    void OnSurfaceInterpolationFinished();
private:
    //Down Sample Image


    QmitkDataStorageComboBox* m_imageComboBox;

    mitk::DataNode* m_refImageNode;
    mitk::DataNode* m_workImageNode;
    mitk::DataNode::Pointer m_sourceImageNode;
    mitk::DataNode::Pointer m_sinkImageNode;
    mitk::DataNode::Pointer m_resultSurfaceImageNode;
    itk::SmartPointer<Float3DImageType> m_originImage;

    GraphCut<Int3DImageType> m_graphcut;

    //vtkSmartPointer<vtkInteractorStyleScribble> m_graphcutStyle;

    itk::ImageRegion<3> m_imageRegion;

    typedef std::vector<itk::Index<3> > VectorOfPixels;
    VectorOfPixels m_sources;
    VectorOfPixels m_sinks;
    VectorOfPixels* m_selectedPixelSet;
    mitk::PaintbrushTool* m_tool;

    QSlider* m_contourValueSlider;
    bool m_bInited;

    double m_ContourValue ;
    double m_ContourValueMin ;
    double m_ContourValueMax;

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


    int m_roi[6];
};

#endif // GraphcutSegmentationView_h__