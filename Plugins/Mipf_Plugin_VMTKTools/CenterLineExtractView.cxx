#include "CenterLineExtractView.h"
#include "iqf_main.h"
#include "Res/R.h"


#include "QmitkPointListWidget.h"

//vtk
#include "vtkCleanPolyData.h"
#include "vtkTriangleFilter.h"
#include "vtkLinearSubdivisionFilter.h"
#include "vtkWindowedSincPolyDataFilter.h"
#include "vtkPolyDataNormals.h"
#include "vtkDecimatePro.h"
#include "vtkPointLocator.h"
#include "vtkSphere.h"
#include "vtkClipPolyData.h"
#include "vtkPointData.h"
#include "vtkDoubleArray.h"
#include "vtkCell.h"
#include "vtkPolyDataConnectivityFilter.h"
#include "vtkSphereSource.h"
#include "vtkSplineFilter.h"

//vmkt
#include "vmtk/vtkvmtkPolyDataCenterlines.h"
#include "vmtk/vtkvmtkPolyDataNetworkExtraction.h"
#include "vmtk/vtkvmtkCapPolyData.h"
#include "vmtk/vtkvmtkPolyBall.h"


#include "MitkStd/IQF_MitkPointList.h"

#include "VesselTools/IQF_CenterLineExtraction.h"
#include "Core/IQF_ObjectFactory.h"


CenterLineExtractView::CenterLineExtractView() :MitkPluginView()
{

}

void CenterLineExtractView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "") == 0)
    {
        //do what you want for the message
    }
}

void CenterLineExtractView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(CreateSurfacePredicate());

    //seed point 
    m_pTargetPointSetNode = mitk::DataNode::New();
    m_pTargetPointSet = mitk::PointSet::New();
    m_pTargetPointSetNode->SetData(m_pTargetPointSet);
    m_pTargetPointSetNode->SetName("seed points for tracking");
    m_pTargetPointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true));
    m_pTargetPointSetNode->SetProperty("layer", mitk::IntProperty::New(1024));
    GetDataStorage()->Add(m_pTargetPointSetNode);
    m_ui.SourceSeedWidget->SetPointSetNode(m_pTargetPointSetNode);
    m_ui.SourceSeedWidget->SetMultiWidget(GetMitkRenderWindowInterface()->GetMitkStdMultiWidget());

    m_pSourcePointSetNode = mitk::DataNode::New();
    m_pSourcePointSet = mitk::PointSet::New();
    m_pSourcePointSetNode->SetData(m_pSourcePointSet);
    m_pSourcePointSetNode->SetName("seed points for tracking");
    m_pSourcePointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true));
    m_pSourcePointSetNode->SetProperty("layer", mitk::IntProperty::New(1024));
    GetDataStorage()->Add(m_pSourcePointSetNode);
    m_ui.TargetSeedWidget->SetPointSetNode(m_pSourcePointSetNode);
    m_ui.TargetSeedWidget->SetMultiWidget(GetMitkRenderWindowInterface()->GetMitkStdMultiWidget());

    //end point
    IQF_PointListFactory* pFactory = (IQF_PointListFactory*)m_pMain->GetInterfacePtr(QF_MitkStd_PointListFactory);
    if (!pFactory)  return;
    m_pEndPointList = pFactory->CreatePointList();
    m_pEndPointList->Initialize();
    m_pEndPointSetNode =  m_pEndPointList->CreateNewPointSetNode();
    GetDataStorage()->Add(m_pEndPointSetNode);

    m_ui.EndPointList->SetMultiWidget(GetMitkRenderWindowInterface()->GetMitkStdMultiWidget());
    dynamic_cast<QmitkPointListModel *>(m_ui.EndPointList->model())->SetPointSetNode(m_pEndPointSetNode);

    connect(m_ui.ApplyBtn, SIGNAL(clicked()), this, SLOT(Extract()));
    connect(m_ui.SelectEndPointBtn, SIGNAL(clicked(bool)), this, SLOT(SelectEndPoint(bool)));
    connect(m_ui.SmoothBtn, SIGNAL(clicked()), this, SLOT(Smooth()));
    
}

void CenterLineExtractView::ConvertVTKPointsToMitkPointSet(vtkPoints* vps, mitk::PointSet* mps)
{
    for (int i=0;i<vps->GetNumberOfPoints();i++)
    {
        mitk::Point3D mp;
        auto currentPoint = vps->GetPoint(i);
        mp.SetElement(0, currentPoint[0]);
        mp.SetElement(1, currentPoint[1]);
        mp.SetElement(2, currentPoint[2]);
        mps->InsertPoint(i,mp);
    }
}

int CenterLineExtractView::FoundMinimumIndex(std::vector<double>& v)
{
    double minValue = DBL_MAX;
    double minIndex = 0;
    int i = 0;
    for (std::vector<double>::iterator it = v.begin(); it!= v.end(); it++)
    {
           if (*it<minValue)
           {
               minValue = *it;
               minIndex = i;
           }
           i++;
    }
    std::cout << "Minimum Distance:" << minValue << std::endl;
    return minIndex;
}

template <typename T>
void RemoveIndex(std::vector<T>& v, int index)
{
    if (v.size()<2)
    {
        return;
    }
    int i = 0;
    for (std::vector<T>::iterator it = v.begin(); it != v.end(); it++)
    {
        if (i==index)
        {
            v.erase(it);
        }
        i++;
    }
}

void CenterLineExtractView::Smooth()
{
    vtkPolyData* polyData = dynamic_cast<mitk::Surface*>(m_ui.DataSelector->GetSelectedNode()->GetData())->GetVtkPolyData();

    IQF_ObjectFactory* pObjectFactory = (IQF_ObjectFactory*)GetInterfacePtr(QF_Core_ObjectFactory);
    if (pObjectFactory)
    {
        IQF_CenterLineExtraction* pCenterLineExtraction = (IQF_CenterLineExtraction*)pObjectFactory->CreateObject(Object_ID_CenterLineExtraction);
        if (pCenterLineExtraction)
        {
            auto preparedModel = vtkSmartPointer<vtkPolyData>::New();
            auto model = vtkSmartPointer<vtkPolyData>::New();
            auto network = vtkSmartPointer<vtkPolyData>::New();
            auto voronoi = vtkSmartPointer<vtkPolyData>::New();
            auto endPoints = vtkSmartPointer<vtkPoints>::New();

            //   pCenterLineExtraction->ExtractCenterLineNetwork(polyData, m_pPointSet->GetPoint(0).GetDataPointer(),
            //      network.Get(), endPoints.Get());
            //do not input start point
            pCenterLineExtraction->ExtractCenterLineNetwork(polyData, nullptr,
                network.Get(), endPoints.Get());

            auto smoothedVessel = vtkSmartPointer<vtkPolyData>::New();
            pCenterLineExtraction->ReconstructTubularSurfaceByCenterLine(network.Get(), smoothedVessel.Get());

            //Display Data
            mitk::DataNode::Pointer smoothedSurfaceDataNode = mitk::DataNode::New();
            mitk::Surface::Pointer smoothedSurface = mitk::Surface::New();
            smoothedSurface->SetVtkPolyData(smoothedVessel.Get());
            smoothedSurfaceDataNode->SetData(smoothedSurface);
            smoothedSurfaceDataNode->SetName("Smoothed Surface");
            smoothedSurfaceDataNode->SetColor(0.0, 1.0, 0.0);
            GetDataStorage()->Add(smoothedSurfaceDataNode, m_ui.DataSelector->GetSelectedNode());

            RequestRenderWindowUpdate();
        }
    }
}

void CenterLineExtractView::Extract()
{
    vtkPolyData* polyData = dynamic_cast<mitk::Surface*>(m_ui.DataSelector->GetSelectedNode()->GetData())->GetVtkPolyData();

    IQF_ObjectFactory* pFactory = (IQF_ObjectFactory*)GetInterfacePtr(QF_Core_ObjectFactory);
    IQF_CenterLineExtraction* pExtraction = (IQF_CenterLineExtraction*)pFactory->CreateObject(Object_ID_CenterLineExtraction);

   auto  sourceSeeds = vtkSmartPointer<vtkPoints>::New();
   auto  targetSeeds = vtkSmartPointer<vtkPoints>::New();
   for (mitk::PointSet::PointsConstIterator pointsIterator = m_pSourcePointSet->GetPointSet()->GetPoints()->Begin(); // really nice syntax to get an interator for all points
       pointsIterator != m_pSourcePointSet->GetPointSet()->GetPoints()->End();
       ++pointsIterator)
   {
       sourceSeeds->InsertNextPoint(pointsIterator.Value().GetDataPointer());
   }
   for (mitk::PointSet::PointsConstIterator pointsIterator2 = m_pTargetPointSet->GetPointSet()->GetPoints()->Begin(); // really nice syntax to get an interator for all points
       pointsIterator2 != m_pTargetPointSet->GetPointSet()->GetPoints()->End();
       ++pointsIterator2)
   {
       targetSeeds->InsertNextPoint(pointsIterator2.Value().GetDataPointer());

   }

   auto output = vtkSmartPointer<vtkPolyData>::New();
   pExtraction->ExtractCenterLine(polyData, sourceSeeds, targetSeeds, output);
   ImportVtkPolyData(output, "outptu");

   auto surface = vtkSmartPointer<vtkPolyData>::New();
   pExtraction->ReconstructTubularSurfaceByCenterLine(output.Get(), surface.Get());
   ImportVtkPolyData(surface, "surface");
   return;


    IQF_ObjectFactory* pObjectFactory = (IQF_ObjectFactory*)GetInterfacePtr(QF_Core_ObjectFactory);
    if (pObjectFactory)
    {
        IQF_CenterLineExtraction* pCenterLineExtraction = (IQF_CenterLineExtraction*)pObjectFactory->CreateObject(Object_ID_CenterLineExtraction);
        if (pCenterLineExtraction)
        {
            auto preparedModel = vtkSmartPointer<vtkPolyData>::New();
            auto model = vtkSmartPointer<vtkPolyData>::New();
            auto network = vtkSmartPointer<vtkPolyData>::New();
            auto voronoi = vtkSmartPointer<vtkPolyData>::New();
            auto endPoints = vtkSmartPointer<vtkPoints>::New();

         //   pCenterLineExtraction->ExtractCenterLineNetwork(polyData, m_pPointSet->GetPoint(0).GetDataPointer(),
          //      network.Get(), endPoints.Get());
            //do not input start point
            pCenterLineExtraction->ExtractCenterLineNetwork(polyData, nullptr,
                      network.Get(), endPoints.Get());

            ConvertVTKPointsToMitkPointSet(endPoints, m_pEndPointList->GetPointSet());

            mitk::DataNode::Pointer networkDataNode = mitk::DataNode::New();
            mitk::Surface::Pointer networkSurface = mitk::Surface::New();
            networkSurface->SetVtkPolyData(network.Get());
            networkDataNode->SetData(networkSurface);
            networkDataNode->SetName("Center Lines");
            networkDataNode->SetColor(0.0, 1.0, 0.0);
            GetDataStorage()->Add(networkDataNode, m_ui.DataSelector->GetSelectedNode());

            RequestRenderWindowUpdate();
        }
    }
}

void CenterLineExtractView::SelectEndPoint(bool bSelecting)
{
    m_pEndPointList->AddPoint(bSelecting);
}