#ifndef CenterLineExtractView_h__
#define CenterLineExtractView_h__

#include "MitkPluginView.h"
#include <QWidget>
#include "ui_CenterLineExtractView.h"

#include "mitkPointSet.h"
#include "vtkPolyData.h"

class    IQF_MitkPointList;

class CenterLineExtractView : public QWidget,public MitkPluginView
{
    Q_OBJECT
public:
    CenterLineExtractView();
    void CreateView();
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
    void PrepareModel( vtkPolyData* polyData,vtkPolyData* outputPolyData);
    void DecimateSurface(vtkPolyData* polyData, vtkPolyData* outputPolyData);
    void OpenSurfaceAtPoint(vtkPolyData* polyData, vtkPolyData* outputPolyData,double* seed);
    void ExtractNetwork( vtkPolyData* polyData, vtkPolyData* outputPolyData);
    vtkSmartPointer<vtkPoints> ClipSurfaceAtEndPoints(vtkPolyData* networkPolyData, vtkPolyData* surfacePolyData, vtkPolyData* outputPolyData);
    void ComputeCenterlines(vtkPolyData* polyData, vtkIdList* inletSeedIds, vtkIdList* outletSeedIds,
        vtkPolyData* outPolyData,vtkPolyData* outPolyData2);
    static int FoundMinimumIndex(std::vector<double>& v);
    static void ConvertVTKPointsToMitkPointSet(vtkPoints* vps,mitk::PointSet* mps);
protected slots:
    void Extract();
    void SelectEndPoint(bool bSelecting);
private:
    Ui::CenterLineExtractView m_ui;

    mitk::PointSet::Pointer m_pPointSet;
    mitk::DataNode::Pointer m_pPointSetNode;

    mitk::PointSet::Pointer m_pEndPointSet;
    mitk::DataNode::Pointer m_pEndPointSetNode;
    IQF_MitkPointList* m_pEndPointList;
};

#endif // CenterLineExtractView_h__