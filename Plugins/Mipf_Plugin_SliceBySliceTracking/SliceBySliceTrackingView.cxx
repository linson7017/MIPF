#include "SliceBySliceTrackingView.h"
#include "iqf_main.h"
#include "Res/R.h"

//mitk
#include "mitkImage.h"
#include "mitkImageCast.h"
#include "mitkSurface.h"
#include "mitkNodePredicateDataType.h"

#include "MitkMain/IQF_MitkDataManager.h"
#include "MitkMain/IQF_MitkRenderWindow.h"
#include "Grid/Math/IQF_MathUtil.h"

#include "Tracking/IQF_SliceBySliceTracking.h"

#include "QmitkPointListViewWidget.h"
#include "QmitkPointListWidget.h"
#include "QmitkDataStorageComboBox.h"

#include <QComboBox>

#include "vtkPolyData.h"
#include "vtkPolyVertex.h"
#include "vtkUnstructuredGrid.h"
#include "vtkAppendPolyData.h"
#include "vtkSphereSource.h"

SliceBySliceTrackingView::SliceBySliceTrackingView():MitkPluginView()
{

}


void SliceBySliceTrackingView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "SliceBySliceTracking.Track") == 0)
    {
        std::cout << "Resceive command " << szMessage << std::endl;
        return;
        IQF_MitkDataManager* pMitkDataStorage = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr("QF_MitkMain_DataManager");
        if (pMitkDataStorage)
        {
            IQF_SliceBySliceTracking* pTrakcer = (IQF_SliceBySliceTracking*)m_pMain->GetInterfacePtr(QF_Algorithm_SliceBySliceTracking);
            
            //Set Current Image
            mitk::DataNode::Pointer currentNode = pMitkDataStorage->GetCurrentNode();
            mitk::Image::Pointer image = 0;
            if (currentNode.IsNotNull())
            {
                image = dynamic_cast<mitk::Image *>(currentNode->GetData());
            }
            if (!image)
            {
                std::cout << "Image NULL" << std::endl;
                return;
            }

            mitk::Point3D origin = image->GetGeometry()->GetOrigin();
            vtkSmartPointer<vtkImageData> vtkImage = vtkSmartPointer<vtkImageData>::New();
            vtkImage->DeepCopy(image->GetVtkImageData());
            vtkImage->SetOrigin(origin[0], origin[1], origin[2]);
            pTrakcer->SetInputImage(vtkImage);
            

            //Set the Seed To Tracker
            XMarkerList seedList;
            mitk::PointSet::PointsContainer* points = m_PointSet->GetPointSet()->GetPoints();
            for (mitk::PointSet::PointsConstIterator pointsIterator = points->Begin();
                pointsIterator != points->End();
                ++pointsIterator)
            {
                mitk::Point3D p = pointsIterator.Value();
                seedList.appendItem(XMarker(p.GetElement(0), p.GetElement(1), p.GetElement(2)));
                mitk::Point3D index;
                image->GetUpdatedGeometry()->WorldToIndex(p, index);
                std::cout << "Seed Point " << index.GetElement(0) << "," << index.GetElement(1) << "," << index.GetElement(2) << std::endl;
                
            }
            pTrakcer->SetSeedPoints(seedList);

            int trackMode = BLOB_TRACK;
            QComboBox* modeSelector = (QComboBox*)R::Instance()->getObjectFromGlobalMap("SliceBySliceTracking.TrackMode");
            if (modeSelector)
            {
                if (modeSelector->currentText().compare("Blob",Qt::CaseInsensitive)==0)
                {
                    trackMode = BLOB_TRACK;
                }
                else if (modeSelector->currentText().compare("Brightest Point", Qt::CaseInsensitive) == 0)
                {
                    trackMode = BRIGHT_POINT_TRACK;
                }    
            }

            pTrakcer->Track(trackMode);
            std::vector< std::vector<Vector3> > result;
            pTrakcer->GetOutput(result);
            ShowResults(result);
        }
    }
    else if (strcmp(szMessage, MITK_MESSAGE_MULTIWIDGET_INITIALIZED) == 0)
    {
          
    }
}


void SliceBySliceTrackingView::ShowResults(std::vector< std::vector<Vector3> > graph)
{

    IQF_MathUtil* pMathUtil = (IQF_MathUtil*)m_pMain->GetInterfacePtr(QF_Algorithm_MathUtil);
    if (!pMathUtil)
    {
        return;
    }
    int color[6][3] = { { 1.0,0.0,1.0 },
    { 1.0,0.0,0.0 },
    { 0.0,1.0,0.0 },
    { 0.0,0.0,1.0 },
    { 1.0,1.0,0.0 },
    { 0.0,1.0,1.0 } };

    for (int i = 0; i < graph.size(); i++)
    {
        std::vector<Vector3> points = graph.at(i);
        // Create a polydata object
        vtkSmartPointer<vtkAppendPolyData> appendFilter =
            vtkSmartPointer<vtkAppendPolyData>::New();
        for (auto data : points)
        {
            auto sphere = vtkSmartPointer<vtkSphereSource>::New();
            sphere->SetCenter(data.x(), data.y(), data.z());
            sphere->SetRadius(0.5);
            sphere->Update();
            appendFilter->AddInputData(sphere->GetOutput());
        }
        appendFilter->Update();


        // Set the points and vertices we created as the geometry and topology of the polydata
        mitk::Surface::Pointer pointsSurface = mitk::Surface::New();
        pointsSurface->SetVtkPolyData(appendFilter->GetOutput());
        pointsSurface->Update();
        mitk::DataNode::Pointer pointsNode = mitk::DataNode::New();
        pointsNode->SetData(pointsSurface);
        std::string pointsName = "points";
        char temp[10];
        pointsName.append(itoa(i + 1, temp, 10));
        pointsNode->SetProperty("name", mitk::StringProperty::New(pointsName));
        pointsNode->SetProperty("color", mitk::ColorProperty::New(0.0, 1.0, 1.0));
        pointsNode->Update();


        //line
        Vector3 center, normal;
        pMathUtil->FitLine(points, center,normal);

        vtkSmartPointer<vtkPolyData> linepolydata = vtkSmartPointer<vtkPolyData>::New();
        vtkSmartPointer<vtkPoints> linepoints = vtkSmartPointer<vtkPoints>::New();
        double ext = 150;
        linepoints->InsertNextPoint(center.x()-normal.x()*ext, center.y() - normal.y()*ext, center.z() - normal.z()*ext);
        linepoints->InsertNextPoint(center.x() + normal.x()*ext, center.y() + normal.y()*ext, center.z() + normal.z()*ext);
        linepolydata->Allocate();
        linepolydata->SetPoints(linepoints);
        vtkIdType connectivity[2];
        connectivity[0] = 0;
        connectivity[1] = 1;
        linepolydata->InsertNextCell(VTK_LINE, 2, connectivity);
        
        mitk::Surface::Pointer line = mitk::Surface::New();
        line->SetVtkPolyData(linepolydata);
        line->Update();
        mitk::DataNode::Pointer lineNode = mitk::DataNode::New();
        lineNode->SetData(line);
        std::string name = "line";
        char num[10];
        name.append(itoa(i+1, num,10));
        lineNode->SetProperty("name", mitk::StringProperty::New(name));
        lineNode->SetProperty("color", mitk::ColorProperty::New(1.0, 1.0, 1.0));
        lineNode->Update();


        IQF_MitkDataManager* pMitkDataStorage = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr("QF_MitkMain_DataManager");
        if (pMitkDataStorage)
        {
            GetDataStorage()->Add(lineNode, pMitkDataStorage->GetCurrentNode());
            GetDataStorage()->Add(pointsNode, pMitkDataStorage->GetCurrentNode());
            mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
    }
}

void SliceBySliceTrackingView::SetupResource()
{
    m_pMain->Attach(this);
    QmitkDataStorageComboBox* selector = (QmitkDataStorageComboBox*)R::Instance()->getObjectFromGlobalMap("SliceBySliceTracking.DataSelector");
    if (selector)
    {
        IQF_MitkDataManager* pMitkDataStorage = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr("QF_MitkMain_DataManager");
        selector->SetDataStorage(GetDataStorage());
        selector->SetPredicate(CreateImagePredicate());
    }
    IQF_MitkRenderWindow* pRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
    if (pRenderWindow)
    {
        m_PointListWidget = (QmitkPointListWidget*)R::Instance()->getObjectFromGlobalMap("SliceBySliceTracking.PointListWidget");
        m_PointListWidget = new QmitkPointListWidget();

        m_PointSet = mitk::PointSet::New();
        mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
        pointSetNode->SetData(m_PointSet);
        pointSetNode->SetName("seed points for tracking");
        pointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true));
        pointSetNode->SetProperty("layer", mitk::IntProperty::New(1024));

        // add the pointset to the data storage (for rendering and access by other modules)
        IQF_MitkDataManager* pMitkDataStorage = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr("QF_MitkMain_DataManager");
        GetDataStorage()->Add(pointSetNode);
        // tell the GUI widget about the point set
        m_PointListWidget->SetPointSetNode(pointSetNode);
        m_PointListWidget->SetMultiWidget(pRenderWindow->GetMitkStdMultiWidget());
    }
}

