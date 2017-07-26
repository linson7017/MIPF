#ifndef VesselSegmentationView_h__
#define VesselSegmentationView_h__

#include "MitkPluginView.h"
#include "vtkPolyData.h"


#include <QWidget>

#include "ui_VesselSegmentationController.h"

class VesselSegmentationView :public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    VesselSegmentationView();
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
    void CreateView();

private:
    protected slots :

    void OnCreateSmoothSurface();

    void OnCreateVesselSurfaceFromMask();

    void OnCurrentSelectedDataNode(mitk::DataNode*  node, int type);

protected:

    mitk::DataNode::Pointer m_SourcePointSetNode;

    mitk::PointSet::Pointer m_SourcePointSet;

    mitk::DataNode::Pointer m_SourcePointSetNode_re;

    mitk::PointSet::Pointer m_SourcePointSet_re;

    mitk::DataNode::Pointer m_TargetPointSetNode_re;

    mitk::PointSet::Pointer m_TargetPointSet_re;

    mitk::DataNode::Pointer newNode;

protected:

    void InitializeGUI();

    void InitializePointListWidget();

    template < typename TPixel, unsigned int VImageDimension >
    void ItkImageRegionGrowing(itk::Image< TPixel, VImageDimension >* itkImage, mitk::Geometry3D* imageGeometry, mitk::DataNode* parent);

    vtkImageData* ExecuteFM(vtkImageData* image, double lowerThreshold, double higherThreshold, vtkIdList*  sourceSeedIds, vtkIdList*targetSeedIds);

    vtkPolyData* MarchingCubes(vtkImageData* image, double threshold);

    vtkPolyData* ComputeCenterLine(vtkPolyData* CenterIndata, vtkIdList*  inletSeedIds, vtkIdList*  outletSeedIds);


    Ui::WxVesselSegmentationController	ui;
    QmitkStdMultiWidget*							m_Viewer3D;
    int																m_SelectedDataNodeType;
};

#endif // VesselSegmentationView_h__