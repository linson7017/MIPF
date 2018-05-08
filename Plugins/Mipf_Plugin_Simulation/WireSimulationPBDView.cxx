#include "WireSimulationPBDView.h" 
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
#include <vtkVertexGlyphFilter.h>

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

//PBD
#include "Demos/Simulation/TimeManager.h"

using namespace PBD;
using namespace Eigen;
using namespace std;
using namespace Utilities;
  
WireSimulationPBDView::WireSimulationPBDView() :MitkPluginView() 
{
    m_timeStep = 33;
    numberOfPoints = 32;
    doPause = false;
}
                                                                                                                                                                                                                            ;
WireSimulationPBDView::~WireSimulationPBDView() 
{
    m_pEntrancePointList->Release();
    m_pDirectionPointList->Release();
}
 
void WireSimulationPBDView::CreateView()
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

    connect(m_ui.ApplyBtn, &QPushButton::clicked, this, &WireSimulationPBDView::Apply);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(Refresh()));

    connect(m_ui.SelectEntrancePointBtn, SIGNAL(clicked(bool)), this, SLOT(SelectEntrancePoint(bool)));
    connect(m_ui.SelectDirectionPointBtn, SIGNAL(clicked(bool)), this, SLOT(SelectDirectionPoint(bool)));
    connect(m_ui.ClearEntrancePointBtn, SIGNAL(clicked()), this, SLOT(ClearEntrancePoint()));
    connect(m_ui.ClearDirectionPointBtn, SIGNAL(clicked()), this, SLOT(ClearDirectionPoint()));

    connect(m_ui.DynamicCB, SIGNAL(clicked(bool)), this, SLOT(EnableDynamic(bool)));

    
} 
 
WndHandle WireSimulationPBDView::GetPluginHandle() 
{
    return this; 
}

void WireSimulationPBDView::Update(const char* szMessage, int iValue /* = 0 */, void* pValue /* = 0 */)
{
       if (strcmp(szMessage,"Cell_Along_Line")==0)
       {

       }
}

void WireSimulationPBDView::EnableDynamic(bool enable)
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

void WireSimulationPBDView::ClearEntrancePoint()
{
      if (m_pEntrancePointList)
      {
          m_pEntrancePointList->GetPointSet()->Clear();
      }
}

void WireSimulationPBDView::ClearDirectionPoint()
{
    if (m_pDirectionPointList)
    {
        m_pDirectionPointList->GetPointSet()->Clear();
    }
}

void WireSimulationPBDView::SelectEntrancePoint(bool select)
{
    m_pEntrancePointList->AddPoint(select);
    m_pDirectionPointList->AddPoint(!select);
    if (select)
    {
        m_ui.SelectDirectionPointBtn->setChecked(false);
    }
}

void WireSimulationPBDView::SelectDirectionPoint(bool select)
{
    m_pDirectionPointList->AddPoint(select);
    m_pEntrancePointList->AddPoint(!select);
    if (select)
    {
        m_ui.SelectEntrancePointBtn->setChecked(false);
    }
}

void WireSimulationPBDView::Apply()
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

void WireSimulationPBDView::InitializeScene()
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

    //create rod
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
    model.m_objects = objs;
    model.m_imgs = imgs;
    TimeManager::getCurrent()->setTimeStepSize(0.002f);
    model.setBendingAndTwistingStiffness(Vector3r(0.2, 0.2, 0.2));
    createRod(entrancePoint, direction);

    sim.setGravity(Vector3r(0, 9.8, 0));
    sim.setVelocityUpdateMethod(0);
    //add Tet model
    //points
    //std::vector<Vector3r> points;
    //int pointNum = surface->GetVtkPolyData()->GetPoints()->GetNumberOfPoints();
    //for (int i = 0; i < pointNum; i++)
    //{
    //    points.push_back(Vector3r(surface->GetVtkPolyData()->GetPoints()->GetPoint(i)));
    //}
    ////indices
    //std::vector<unsigned int> nIndices;
    //int indicesNum = surface->GetVtkPolyData()->GetCellData()->GetNumberOfTuples() * 3;
    //for (int i = 0; i < surface->GetVtkPolyData()->GetNumberOfCells(); i++)
    //{
    //    vtkCell* cell = surface->GetVtkPolyData()->GetCell(i);
    //    for (int j = 0; j < cell->GetPointIds()->GetNumberOfIds(); j++)
    //    {
    //        nIndices.push_back(cell->GetPointId(j));
    //    }
    //}
    //collision
    //model.addTetModel(pointNum, (unsigned int)nIndices.size() / 4u, points.data(), nIndices.data());
    //// init constraints
    //short simulationMethod = 1;
    //ParticleData &pd = model.getParticles();
    //for (unsigned int i = numberOfPoints; i < pd.getNumberOfParticles(); i++)
    //{
    //    pd.setMass(i, 0);
    //}
    //for (unsigned int cm = 0; cm < model.getTetModels().size(); cm++)
    //{
    //    const unsigned int nTets = model.getTetModels()[cm]->getParticleMesh().numTets();
    //    const unsigned int *tets = model.getTetModels()[cm]->getParticleMesh().getTets().data();
    //    const IndexedTetMesh::VertexTets *vTets = model.getTetModels()[cm]->getParticleMesh().getVertexTets().data();
    //    if (simulationMethod == 1)
    //    {
    //        const unsigned int offset = model.getTetModels()[cm]->getIndexOffset();
    //        const unsigned int nEdges = model.getTetModels()[cm]->getParticleMesh().numEdges();
    //        const IndexedTetMesh::Edge *edges = model.getTetModels()[cm]->getParticleMesh().getEdges().data();
    //        for (unsigned int i = 0; i < nEdges; i++)
    //        {
    //            const unsigned int v1 = edges[i].m_vert[0] + offset;
    //            const unsigned int v2 = edges[i].m_vert[1] + offset;

    //            model.addDistanceConstraint(v1, v2);
    //        }

    //        for (unsigned int i = 0; i < nTets; i++)
    //        {
    //            const unsigned int v1 = tets[4 * i];
    //            const unsigned int v2 = tets[4 * i + 1];
    //            const unsigned int v3 = tets[4 * i + 2];
    //            const unsigned int v4 = tets[4 * i + 3];

    //            model.addVolumeConstraint(v1, v2, v3, v4);
    //        }
    //    }
    //    else if (simulationMethod == 2)
    //    {
    //        TetModel::ParticleMesh &mesh = model.getTetModels()[cm]->getParticleMesh();
    //        for (unsigned int i = 0; i < nTets; i++)
    //        {
    //            const unsigned int v1 = tets[4 * i];
    //            const unsigned int v2 = tets[4 * i + 1];
    //            const unsigned int v3 = tets[4 * i + 2];
    //            const unsigned int v4 = tets[4 * i + 3];

    //            model.addFEMTetConstraint(v1, v2, v3, v4);
    //        }
    //    }
    //    else if (simulationMethod == 3)
    //    {
    //        TetModel::ParticleMesh &mesh = model.getTetModels()[cm]->getParticleMesh();
    //        for (unsigned int i = 0; i < nTets; i++)
    //        {
    //            const unsigned int v1 = tets[4 * i];
    //            const unsigned int v2 = tets[4 * i + 1];
    //            const unsigned int v3 = tets[4 * i + 2];
    //            const unsigned int v4 = tets[4 * i + 3];

    //            model.addStrainTetConstraint(v1, v2, v3, v4);
    //        }
    //    }
    //    else if (simulationMethod == 4)
    //    {
    //        TetModel::ParticleMesh &mesh = model.getTetModels()[cm]->getParticleMesh();
    //        for (unsigned int i = 0; i < nTets; i++)
    //        {
    //            const unsigned int v[4] = { tets[4 * i], tets[4 * i + 1], tets[4 * i + 2], tets[4 * i + 3] };
    //            // Important: Divide position correction by the number of clusters 
    //            // which contain the vertex.
    //            const unsigned int nc[4] = { vTets[v[0]].m_numTets, vTets[v[1]].m_numTets, vTets[v[2]].m_numTets, vTets[v[3]].m_numTets };
    //            model.addShapeMatchingConstraint(4, v, nc);
    //        }
    //    }
    //    model.getTetModels()[cm]->updateMeshNormals(pd);
    //}

    /*SimulationModel::TetModelVector &tm = model.getTetModels();
    for (unsigned int i = 0; i < tm.size(); i++)
    {
        const unsigned int nVert = tm[i]->getParticleMesh().numVertices();
        unsigned int offset = tm[i]->getIndexOffset();
        tm[i]->setFrictionCoeff(0.1);
        cd.addCollisionObjectWithoutGeometry(i, CollisionDetection::CollisionObject::TetModelCollisionObjectType, &pd.getPosition(offset), nVert);
    }*/

}

void WireSimulationPBDView::RefreshData()
{
    mitk::DataNode* node = GetDataStorage()->GetNamedNode("wire");

    unsigned long offset=0;
    for (unsigned int i = 0; i < model.getTetModels().size(); i++)
    {
        offset += model.getTetModels()[i]->getVisVertices().size();
    }

    vtkSmartPointer<vtkAppendPolyData> appendFilter =
        vtkSmartPointer<vtkAppendPolyData>::New();

    ParticleData &particles = model.getParticles();
    ParticleData &ghostParticles = model.getGhostParticles();
    SimulationModel::ConstraintVector &constraints = model.getConstraints();

    auto polydata = vtkSmartPointer<vtkPolyData>::New();
    auto points = vtkSmartPointer<vtkPoints>::New();
    auto polyLine = vtkSmartPointer<vtkPolyLine>::New();
    polyLine->GetPointIds()->SetNumberOfIds(numberOfPoints);
    auto cells = vtkSmartPointer<vtkCellArray>::New();
    for (int i = 0; i < numberOfPoints; i++)
    {
        vtkIdType pid =  points->InsertNextPoint(particles.getPosition(i).data());
        polyLine->GetPointIds()->SetId(i, pid);

        auto sphere = vtkSmartPointer<vtkSphereSource>::New();
        sphere->SetCenter(particles.getPosition(i).data());
        sphere->SetRadius(0.2);
        sphere->Update();
        appendFilter->AddInputData(sphere->GetOutput());

        if (i<numberOfPoints-1)
        {
            auto ghostSphere = vtkSmartPointer<vtkSphereSource>::New();
            ghostSphere->SetCenter(ghostParticles.getPosition(i).data());
            ghostSphere->SetRadius(0.1);
            ghostSphere->Update();
            appendFilter->AddInputData(ghostSphere->GetOutput());
        }
    }
    cells->InsertNextCell(polyLine);
    polydata->SetPoints(points);
    polydata->SetLines(cells);

    //for (unsigned int i = numberOfPoints; i < particles.getNumberOfParticles(); i++)
    //{

    //    vtkSmartPointer<vtkPoints> points =
    //        vtkSmartPointer<vtkPoints>::New();
    //    points->InsertNextPoint(particles.getPosition(i).data());
    //    vtkSmartPointer<vtkPolyData> pointsPolydata =
    //        vtkSmartPointer<vtkPolyData>::New();
    //    pointsPolydata->SetPoints(points);
    //   /* vtkSmartPointer<vtkVertexGlyphFilter> vertexFilter =
    //        vtkSmartPointer<vtkVertexGlyphFilter>::New();
    //    vertexFilter->SetInputData(pointsPolydata);
    //    vertexFilter->Update();*/
    //    appendFilter->AddInputData(pointsPolydata);
    //}

    //vtkSmartPointer<vtkTubeFilter> tubeFilter =
    //    vtkSmartPointer<vtkTubeFilter>::New();
    //tubeFilter->SetInputData(polydata);
    //tubeFilter->SetRadius(.025); //default is .5
    //tubeFilter->SetNumberOfSides(particles.getNumberOfParticles());
    //tubeFilter->Update();
   // appendFilter->AddInputData(tubeFilter->GetOutput());
    appendFilter->AddInputData(polydata);
    appendFilter->Update();
    
    mitk::Surface* surface = dynamic_cast<mitk::Surface*>(node->GetData());
    surface->SetVtkPolyData(appendFilter->GetOutput());
}

void WireSimulationPBDView::Refresh()
{
    for (unsigned int i = 0; i < 8; i++)
        sim.step(model);
    RefreshData();
    RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
}

void WireSimulationPBDView::createRod(const mitk::Point3D& entrancePoint, const mitk::Vector3D& headingDirection)
{
    ParticleData &particles = model.getParticles();
    ParticleData &ghostParticles = model.getGhostParticles();
    SimulationModel::ConstraintVector &constraints = model.getConstraints();

    Vector3r entrance(entrancePoint.GetDataPointer());
    Vector3r direction(headingDirection.GetDataPointer());
    direction = -direction;
    double interval = 1.0;
    ////centreline points
    for (unsigned int i = 0; i < numberOfPoints; i++)
    {
        particles.addVertex(i*interval *direction+ entrance);
    }

    //edge ghost points
    for (unsigned int i = 0; i < numberOfPoints - 1; i++)
    {
        ghostParticles.addVertex(i*interval *direction+Vector3r(0.5,1.0,0.0) + entrance);
    }

    //lock two first particles and first ghost point
    //particles.setMass(0, 0.0f);
    //particles.setMass(1, 0.0f);
   // ghostParticles.setMass(0, 0.0f);

    for (unsigned int i = 0; i < numberOfPoints - 1; i++)
    {
        model.addDistanceConstraint(i, i + 1);
        model.addPerpendiculaBisectorConstraint(i, i + 1, i);
        model.addGhostPointEdgeDistanceConstraint(i, i + 1, i);

        if (i < numberOfPoints - 2)
        {
            //  Single rod element:
            //      D   E		//ghost points
            //		|	|
            //  --A---B---C--	// rod points
            int pA = i;
            int pB = i + 1;
            int pC = i + 2;
            int pD = i;
            int pE = i + 1;
            model.addDarbouxVectorConstraint(pA, pB, pC, pD, pE);
        }
    }
    model.addParticalPolyDataConstraint();
}