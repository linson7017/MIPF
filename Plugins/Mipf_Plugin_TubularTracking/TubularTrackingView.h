#ifndef SliceBySliceTrackingView_h__
#define SliceBySliceTrackingView_h__

#include "MitkPluginView.h"
#include "mitkPointSet.h"
#include "mitkDataNode.h"
#include "Vector3.h"

class QmitkPointListWidget;
class TubularTrackingView : public MitkPluginView
{
public:
    TubularTrackingView();
    void InitResource();
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
private:
    void UpdateProcessImageExtent(int* extent);
    void GetProcessImageExtent(int* extent);
    void ShowResults(std::vector< std::vector<Vector3> > graph);

    mitk::PointSet::Pointer m_PointSet;
    QmitkPointListWidget* m_PointListWidget;
    mitk::DataNode::Pointer m_currentNode;
};


#endif // SliceBySliceTrackingView_h__
