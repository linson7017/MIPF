#include "WireSimulationView.h" 
#include "iqf_main.h"  


#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPolyLine.h>
#include <vtkDelaunay3D.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkTriangle.h>
#include <vtkTubeFilter.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencil.h>
#include <vtkSphereSource.h>
#include <vtkCellData.h>
#include <vtkLine.h>

#include <vtkTriangleStrip.h>

#include <DER/Scene.h>
#include <DER/SceneLoader.h>
#include "MitkStd/IQF_MitkPointList.h"
#include "MitkSegmentation/IQF_MitkSurfaceTool.h"

#include "ITKImageTypeDef.h"

//mitk
#include "mitkPointSet.h"
#include "mitkImageCast.h"

#include "ITKVTK_Helpers.h"
  
WireSimulationView::WireSimulationView() :MitkPluginView() 
{
    m_timeStep = 33;
}
                                                                                                                                                                                                                            ;
WireSimulationView::~WireSimulationView() 
{
    m_pEntrancePointList->Release();
    m_pDirectionPointList->Release();
}
 
void WireSimulationView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.DataSelector->SetPredicate(CreateSurfacePredicate());
    m_ui.DataSelector->SetDataStorage(GetDataStorage());

    IQF_PointListFactory* pFactory = (IQF_PointListFactory*)m_pMain->GetInterfacePtr(QF_MitkStd_PointListFactory);
    m_pEntrancePointList = pFactory->CreatePointList();
    m_pDirectionPointList = pFactory->CreatePointList();
    m_pEntrancePointList->Attach(this);
    m_pDirectionPointList->Attach(this);
    mitk::DataNode::Pointer entrancePointSetNode = m_pEntrancePointList->CreateNewPointSetNode();
    mitk::DataNode::Pointer directionPointSetNode = m_pDirectionPointList->CreateNewPointSetNode();
    GetDataStorage()->Add(entrancePointSetNode);
    GetDataStorage()->Add(directionPointSetNode);

    connect(m_ui.ApplyBtn, &QPushButton::clicked, this, &WireSimulationView::Apply);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(Refresh()));

    connect(m_ui.SelectEntrancePointBtn, SIGNAL(clicked(bool)), this, SLOT(SelectEntrancePoint(bool)));
    connect(m_ui.SelectDirectionPointBtn, SIGNAL(clicked(bool)), this, SLOT(SelectDirectionPoint(bool)));
    connect(m_ui.ClearEntrancePointBtn, SIGNAL(clicked()), this, SLOT(ClearEntrancePoint()));
    connect(m_ui.ClearDirectionPointBtn, SIGNAL(clicked()), this, SLOT(ClearDirectionPoint()));

    connect(m_ui.DynamicCB, SIGNAL(clicked(bool)), this, SLOT(EnableDynamic(bool)));

    
} 
 
WndHandle WireSimulationView::GetPluginHandle() 
{
    return this; 
}

void WireSimulationView::Update(const char* szMessage, int iValue /* = 0 */, void* pValue /* = 0 */)
{
       if (strcmp(szMessage,"Cell_Along_Line")==0)
       {
           mitk::DataNode::Pointer node = m_ui.DataSelector->GetSelectedNode();
           double* cell = (double*)pValue;
          node->SetBoolProperty("deprecated usePointDataForColouring", true);
          mitk::Surface* surface = dynamic_cast<mitk::Surface*>(node->GetData());

          m_pEntrancePointList->InsertPoint(cell[0], cell[1], cell[2]);
          return;

          if (!surface->GetVtkPolyData()->GetCellData()->GetScalars())
          {
              vtkSmartPointer<vtkUnsignedCharArray> cellData =
                  vtkSmartPointer<vtkUnsignedCharArray>::New();
              cellData->SetNumberOfComponents(3);
              cellData->SetNumberOfTuples(surface->GetVtkPolyData()->GetNumberOfCells());
              surface->GetVtkPolyData()->GetCellData()->SetScalars(cellData);

              for (int i = 0; i < cellData->GetNumberOfTuples(); ++i)
              {
                  float rgb[3];
                  rgb[0] = 255; rgb[1] = 255; rgb[2] = 255; // banana
                  cellData->InsertTuple(i, rgb);
              }
          }
          float rgb[3];
          rgb[0] = 255; rgb[1] = 99; rgb[2] = 71; // tomato
          surface->GetVtkPolyData()->GetCellData()->GetScalars()->InsertTuple(iValue, rgb);
          
       }
}

void WireSimulationView::EnableDynamic(bool enable)
{
     if (enable)
     {
         if (!m_timer.isActive())
         {
             m_timer.start(m_timeStep);
         }     
     }
     else
     {
         if (m_timer.isActive())
         {
             m_timer.stop();
         }
     }
}

void WireSimulationView::ClearEntrancePoint()
{
      if (m_pEntrancePointList)
      {
          m_pEntrancePointList->GetPointSet()->Clear();
      }
}

void WireSimulationView::ClearDirectionPoint()
{
    if (m_pDirectionPointList)
    {
        m_pDirectionPointList->GetPointSet()->Clear();
    }
}

void WireSimulationView::SelectEntrancePoint(bool select)
{
    m_pEntrancePointList->AddPoint(select);
    m_pDirectionPointList->AddPoint(!select);
    if (select)
    {
        m_ui.SelectDirectionPointBtn->setChecked(false);
    }
}

void WireSimulationView::SelectDirectionPoint(bool select)
{
    m_pDirectionPointList->AddPoint(select);
    m_pEntrancePointList->AddPoint(!select);
    if (select)
    {
        m_ui.SelectEntrancePointBtn->setChecked(false);
    }
}

void WireSimulationView::Apply()
{
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    mitk::Surface::Pointer wire = mitk::Surface::New();
    node->SetData(wire);
    node->SetName("wire");
    GetDataStorage()->Add(node);

    InitializeScene();
    RefreshData();
    
    m_timer.start(m_timeStep);
}

void WireSimulationView::InitializeScene()
{
    m_ui.DataSelector->GetSelectedNode()->SetOpacity(0.6);
    mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_ui.DataSelector->GetSelectedNode()->GetData());
    if (!surface)
    {
        return;
    }

    IQF_MitkSurfaceTool* pSurfaceTool = (IQF_MitkSurfaceTool*)GetInterfacePtr(QF_MitkSurface_Tool);
    vtkSmartPointer<vtkImageData> whiteImage =
        vtkSmartPointer<vtkImageData>::New();
    double bounds[6];
    surface->GetVtkPolyData()->GetBounds(bounds);
    double spacing[3]; // desired volume spacing
    spacing[0] = 0.5;
    spacing[1] = 0.5;
    spacing[2] = 0.5;
    whiteImage->SetSpacing(spacing);

    // compute dimensions
    int dim[3];
    for (int i = 0; i < 3; i++)
    {
        dim[i] = static_cast<int>(ceil((bounds[i * 2 + 1] - bounds[i * 2]) / spacing[i]));
    }
    whiteImage->SetDimensions(dim);
    whiteImage->SetExtent(0, dim[0] - 1, 0, dim[1] - 1, 0, dim[2] - 1);

    double origin[3];
    origin[0] = bounds[0] + spacing[0] / 2;
    origin[1] = bounds[2] + spacing[1] / 2;
    origin[2] = bounds[4] + spacing[2] / 2;
    whiteImage->SetOrigin(origin);
    whiteImage->AllocateScalars(VTK_UNSIGNED_SHORT, 1);
    // fill the image with foreground voxels:
    unsigned char inval = 1;
    unsigned char outval = 0;
    vtkIdType count = whiteImage->GetNumberOfPoints();
    for (vtkIdType i = 0; i < count; ++i)
    {
        whiteImage->GetPointData()->GetScalars()->SetTuple1(i, inval);
    }

    // polygonal data --> image stencil:
    vtkSmartPointer<vtkPolyDataToImageStencil> pol2stenc =
        vtkSmartPointer<vtkPolyDataToImageStencil>::New();
    pol2stenc->SetInputData(surface->GetVtkPolyData());
    pol2stenc->SetOutputOrigin(origin);
    pol2stenc->SetOutputSpacing(spacing);
    pol2stenc->SetOutputWholeExtent(whiteImage->GetExtent());
    pol2stenc->Update();

    // cut the corresponding white image and set the background:
    vtkSmartPointer<vtkImageStencil> imgstenc =
        vtkSmartPointer<vtkImageStencil>::New();
    imgstenc->SetInputData(whiteImage);
    imgstenc->SetStencilConnection(pol2stenc->GetOutputPort());
    imgstenc->ReverseStencilOff();
    imgstenc->SetBackgroundValue(outval);
    imgstenc->Update();
    whiteImage->DeepCopy(imgstenc->GetOutput());


    std::vector<vtkSmartPointer<vtkPolyData>> objs;
    objs.push_back(surface->GetVtkPolyData());
    std::vector<vtkSmartPointer<vtkImageData>> imgs;
    imgs.push_back(whiteImage);


    SceneLoader loader;
   // mitk::Point3D entrancePoint = m_pEntrancePointList->GetPointSet()->GetPoint(0);
   // mitk::Point3D directionPoint = m_pDirectionPointList->GetPointSet()->GetPoint(0);
    mitk::Point3D entrancePoint;
    mitk::Point3D directionPoint;
    entrancePoint[0] = 29.58;
    entrancePoint[1] = 89.14;
    entrancePoint[2] = -5.56;

    directionPoint[0] = 29.58;
    directionPoint[1] = 89.14;
    directionPoint[2] = -7.56;

    mitk::Vector3D direction = directionPoint - entrancePoint;
    direction.Normalize();
    m_scene = loader.createGuideWireScene(mg::Vec3D(entrancePoint.GetElement(0), entrancePoint.GetElement(1), entrancePoint.GetElement(2)),
        mg::Vec3D(direction.GetElement(0), direction.GetElement(1), direction.GetElement(2)), 
        objs,imgs
        );
    m_scene->m_subject->Attach(this);
    m_scene->initialize();
}

void WireSimulationView::RefreshData()
{
    mitk::DataNode* node = GetDataStorage()->GetNamedNode("wire");

    vtkSmartPointer<vtkAppendPolyData> appendFilter =
        vtkSmartPointer<vtkAppendPolyData>::New();

    ElasticRod* strand = m_scene->m_strand;
    auto polydata = vtkSmartPointer<vtkPolyData>::New();
    auto points = vtkSmartPointer<vtkPoints>::New();
    auto polyLine = vtkSmartPointer<vtkPolyLine>::New();
    polyLine->GetPointIds()->SetNumberOfIds(strand->m_ppos.size());
    auto cells = vtkSmartPointer<vtkCellArray>::New();
    for (int i = 0; i < strand->m_ppos.size(); i++)
    {
        float* pos = strand->m_ppos[i].data();
        points->InsertNextPoint(pos[0], pos[1], pos[2]);
        polyLine->GetPointIds()->SetId(i, i);


        auto sphere = vtkSmartPointer<vtkSphereSource>::New();
        sphere->SetCenter(pos[0], pos[1], pos[2]);
        sphere->SetRadius(0.2);
        sphere->Update();
        appendFilter->AddInputData(sphere->GetOutput());

        mg::Vec3D m1 = strand->m_m1[i];
        m1.normalize();
        mg::Vec3D m2 = strand->m_m2[i];
        m2.normalize();
        mg::Vec3D p0 = strand->m_ppos[i] + m1 * 1;
        mg::Vec3D p1 = strand->m_ppos[i] + m2 * 1;
        auto ax = vtkSmartPointer<vtkPolyData>::New();
        vtkSmartPointer<vtkPoints> pts =
            vtkSmartPointer<vtkPoints>::New();
        pts->InsertNextPoint(pos[0], pos[1], pos[2]);
        pts->InsertNextPoint(p0.data());
        pts->InsertNextPoint(p1.data());

        // Add the points to the polydata container
        ax->SetPoints(pts);
        vtkSmartPointer<vtkLine> line0 =
            vtkSmartPointer<vtkLine>::New();
        line0->GetPointIds()->SetId(0, 0); 
        line0->GetPointIds()->SetId(1, 1); 

        vtkSmartPointer<vtkLine> line1 =
            vtkSmartPointer<vtkLine>::New();
        line1->GetPointIds()->SetId(0, 0); 
        line1->GetPointIds()->SetId(1, 2); 

        vtkSmartPointer<vtkCellArray> lines =
            vtkSmartPointer<vtkCellArray>::New();
        lines->InsertNextCell(line0);
        lines->InsertNextCell(line1);

        // Add the lines to the polydata container
        ax->SetLines(lines);
        appendFilter->AddInputData(ax);
    }
    cells->InsertNextCell(polyLine);
    polydata->SetPoints(points);
    polydata->SetLines(cells);                               

    vtkSmartPointer<vtkTubeFilter> tubeFilter =
        vtkSmartPointer<vtkTubeFilter>::New();
    tubeFilter->SetInputData(polydata);
    tubeFilter->SetRadius(.025); //default is .5
    tubeFilter->SetNumberOfSides(strand->m_ppos.size());
    tubeFilter->Update();
    appendFilter->AddInputData(tubeFilter->GetOutput());

    //std::vector<vtkSmartPointer<vtkPolyData>> objs = m_scene->m_objects;
    //for (int i=0;i<objs.size();i++)
    //{
    //    appendFilter->AddInputData(objs.at(i));
    //}
    //
    appendFilter->Update();
    mitk::Surface* surface = dynamic_cast<mitk::Surface*>(node->GetData());
    surface->SetVtkPolyData(appendFilter->GetOutput());
}

void WireSimulationView::Refresh()
{

    for (int i = 0; i < 1; ++i)
    {
        m_scene->update(0.01);
    }

    RefreshData();

    RequestRenderWindowUpdate();
}