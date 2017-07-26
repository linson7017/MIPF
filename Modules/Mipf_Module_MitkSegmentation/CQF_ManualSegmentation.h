#ifndef CQF_ManualSegmentation_h__
#define CQF_ManualSegmentation_h__

#pragma once
#include "mitkDataNode.h"
#include "mitkSurfaceInterpolationController.h"

#include "MitkSegmentation/IQF_MitkSegmentationTool.h"
#include <QObject>
#include <QFuture>
#include <QFutureWatcher>

namespace QF
{
    class IQF_Main;
}

namespace mitk
{
    class ToolManager;
}

class SurfaceInterpolatorWatcher:public QObject
{
    Q_OBJECT
public:
    SurfaceInterpolatorWatcher(QF::IQF_Main* pMain) { m_pMain = pMain; }
    ~SurfaceInterpolatorWatcher() {}
    void SetSurfaceInterpolator(mitk::SurfaceInterpolationController* pSurfaceInterpolator)
    {
        m_pSurfaceInterpolator = pSurfaceInterpolator;
    }
protected slots:
    void OnSurfaceInterpolationFinished();
private:
    mitk::SurfaceInterpolationController* m_pSurfaceInterpolator;
    QF::IQF_Main* m_pMain;
};

class CQF_ManualSegmentation : public IQF_MitkSegmentationTool
{
    //Q_OBJECT
public:
    CQF_ManualSegmentation(QF::IQF_Main* pMain);
    ~CQF_ManualSegmentation();
    virtual void Initialize();
    virtual void Deinitialize();
    virtual bool CreateSegmentationNode(const mitk::DataNode* pRefNode, mitk::DataNode* pSegmentationNode,const char* szName, SegRGBColor& rgbColor= SegRGBColor(1.0,0.0,0.0));
    virtual bool CreateSegmentationNode(const mitk::Image* pOriginImage, mitk::DataNode* pSegmentationNode, const char* szName, SegRGBColor& rgbColor = SegRGBColor(1.0, 0.0, 0.0));
    virtual bool CreateLabelSetImageNode(const mitk::DataNode* pRefNode, mitk::DataNode* pLabelSetNode, const char* szName);
    virtual bool CreateLabelSetImageNode(const mitk::Image* pOriginImage, mitk::DataNode* pLabelSetNode, const char* szName);
    virtual mitk::ToolManager* GetToolManager();
    virtual void SetReferenceData(const mitk::DataNode* node);
    virtual void SetWorkingData(const mitk::DataNode* node);
    virtual mitk::DataNode* GetReferenceData();
    virtual mitk::DataNode* GetWorkingData();
    virtual mitk::Tool* ChangeTool(const char* szToolID);
    virtual mitk::Tool* GetActivedTool();
    virtual void SetSurfaceInterpolateOn(bool bEnableInterpolate = true);


private:
    void Run3DInterpolation();
    void OnSurfaceInterpolationInfoChanged(const itk::EventObject &);
    int GetToolIdByToolName(const std::string &toolName);
    void InitializeSurfaceInterpolate(const mitk::DataNode* imageNode);
private:

    QF::IQF_Main* m_pMain;
    mitk::ToolManager* m_pToolManager;


    QFuture<void> m_Future;
    QFutureWatcher<void> m_Watcher;
    unsigned int SurfaceInterpolationInfoChangedObserverTag;
    mitk::SurfaceInterpolationController::Pointer m_SurfaceInterpolator;
    bool m_bSurfaceInterpolateInited;
    SurfaceInterpolatorWatcher* m_surfaceInterpolatorWatcher;
};

#endif // CQF_ManualSegmentation_h__
