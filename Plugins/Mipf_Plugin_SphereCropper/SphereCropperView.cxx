#include "SphereCropperView.h" 
#include "iqf_main.h"  
#include "Rendering/WxSphereShapeVtkMapper2D.h"
#include "Rendering/WxSphereShapeVtkMapper3D.h"
#include "Interactions/CutImplementation.h"
#include "Interactions/SphereVolumeCutImplementation.h"
#include "Interactions/SphereSurfaceCutImplementation.h"

#include <mitkSurface.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>
  
SphereCropperView::SphereCropperView() 
	:MitkPluginView(),	
	m_Node(nullptr),
	m_SphereShapeInteractor(nullptr),
	m_pImplementation(nullptr),
    m_resultSurface(nullptr)
{
}
 
SphereCropperView::~SphereCropperView() 
{
	//disable interactor
	if (m_SphereShapeInteractor != nullptr)
	{
		m_SphereShapeInteractor->SetDataNode(nullptr);
	}
	if (m_pImplementation)
	{
		delete m_pImplementation;
	}
}
 
void SphereCropperView::CreateView()
{
	m_Controls.setupUi(this);

	m_Controls.boundingShapeSelector->SetDataStorage(this->GetDataStorage());
//	m_Controls.boundingShapeSelector->SetPredicate(CreateImagePredicate());
	m_Controls.boundingShapeSelector->SetPredicate(mitk::NodePredicateOr::New(
			CreateImagePredicate(), CreateSurfacePredicate()));
	m_Controls.buttonCreateNewBoundingBox->setEnabled(false);
	m_Controls.boundingShapeSelector->setEnabled(true);
	m_Controls.buttonStart->setEnabled(false);
	m_Controls.buttonEnd->setEnabled(false);
	m_Controls.checkBoxInsideOut->setEnabled(false);
	m_Controls.buttonCut->setEnabled(false);
	m_Controls.buttonUndo->setEnabled(false);
	m_Controls.buttonRedo->setEnabled(false);

	connect(m_Controls.buttonCreateNewBoundingBox, SIGNAL(clicked()), this, SLOT(DoCreateNewSphereObject()));
	connect(m_Controls.boundingShapeSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode*)),
		this, SLOT(OnDataSelectionChanged(const mitk::DataNode*)));
	connect(m_Controls.buttonStart, SIGNAL(clicked()), this, SLOT(StartInteraction()));
	connect(m_Controls.buttonEnd, SIGNAL(clicked()), this, SLOT(EndInteraction()));
	connect(m_Controls.checkBoxInsideOut, SIGNAL(clicked(bool)), this, SLOT(InsideOutChanged(bool)));
	connect(m_Controls.buttonCut, SIGNAL(clicked()), this, SLOT(Cut()));
	connect(m_Controls.buttonUndo, SIGNAL(clicked()), this, SLOT(Undo()));
	connect(m_Controls.buttonRedo, SIGNAL(clicked()), this, SLOT(Redo()));
} 
 
WndHandle SphereCropperView::GetPluginHandle() 
{
    return this; 
}

void SphereCropperView::InsideOutChanged(bool checked)
{
	if (m_pImplementation)
	{
		m_pImplementation->SetInsideOut(checked);
	}
}

void SphereCropperView::Cut()
{
	if (m_pImplementation)
	{
		m_pImplementation->Cut(m_SphereShapeInteractor->GetSphere(), nullptr);
	}
}

void SphereCropperView::Undo()
{
	if (m_SphereShapeInteractor.IsNotNull())
	{
		m_SphereShapeInteractor->Undo();
	}
}

void SphereCropperView::Redo()
{
	if (m_SphereShapeInteractor.IsNotNull())
	{
		m_SphereShapeInteractor->Redo();
	}
}

void SphereCropperView::DoCreateNewSphereObject()
{
	if (m_Node.IsNotNull())
	{
		if (!m_pImplementation)
		{
			if (dynamic_cast<mitk::Surface *>(m_Node->GetData()) != nullptr)
			{
				m_pImplementation = new SphereSurfaceCutImplementation();
			}
			else if(dynamic_cast<mitk::Image *>(m_Node->GetData()) != nullptr)
			{
				m_pImplementation = new SphereVolumeCutImplementation();
			}			
			m_pImplementation->SetInsideOut(m_Controls.checkBoxInsideOut->isChecked());
		}
		m_pImplementation->Init(m_Node);

		if (m_SphereShapeInteractor.IsNull())
		{
			m_SphereShapeInteractor = mitk::WxSphereShapeInteractor::New();
			//消息连接
			m_SphereShapeInteractor->UndoEvent.AddListener(mitk::MessageDelegate<CutImplementation>(m_pImplementation, &CutImplementation::Undo));
			m_SphereShapeInteractor->RedoEvent.AddListener(mitk::MessageDelegate<CutImplementation>(m_pImplementation, &CutImplementation::Redo));
            m_SphereShapeInteractor->ProcessEvent.AddListener(
                mitk::MessageDelegate2<CutImplementation, vtkObject*, mitk::InteractionEvent *>(m_pImplementation, &CutImplementation::Cut));

            static_cast<SphereVolumeCutImplementation*>(m_pImplementation)->CutFinishedEvent.AddListener(
                mitk::MessageDelegate1<SphereCropperView, vtkObject*>(this, &SphereCropperView::CutFinished));


			m_SphereShapeInteractor->SetDataStorage(GetDataStorage());
			std::string configpath = m_pMain->GetConfigPath();
			configpath.append("/mitk/Interactions/");
			m_SphereShapeInteractor->LoadStateMachine(configpath + "SphereShapeInteraction.xml");
			m_SphereShapeInteractor->SetEventConfig(configpath + "SphereShapeMouseConfig.xml");
		}
		m_SphereShapeInteractor->SetDataNode(m_Node);

		m_Controls.buttonStart->setEnabled(true);
		m_Controls.buttonEnd->setEnabled(true);
		m_Controls.checkBoxInsideOut->setEnabled(true);
		m_Controls.buttonCut->setEnabled(true);
		m_Controls.buttonUndo->setEnabled(true);
		m_Controls.buttonRedo->setEnabled(true);
	}
}

#include <vtkMarchingCubes.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkTransformPolyDataFilter.h>
#include <mitkSurface.h>
#include <MitkSegmentation/IQF_MitkSurfaceTool.h>

template <class T1, class T2, class T3>
inline void mitkVtkLinearTransformPoint(T1 matrix[4][4], T2 in[3], T3 out[3])
{
    T3 x = matrix[0][0] * in[0] + matrix[0][1] * in[1] + matrix[0][2] * in[2] + matrix[0][3];
    T3 y = matrix[1][0] * in[0] + matrix[1][1] * in[1] + matrix[1][2] * in[2] + matrix[1][3];
    T3 z = matrix[2][0] * in[0] + matrix[2][1] * in[1] + matrix[2][2] * in[2] + matrix[2][3];
    out[0] = x;
    out[1] = y;
    out[2] = z;
}

void SphereCropperView::CutFinished(vtkObject* obj)
{
    vtkImageData* image = static_cast<vtkImageData*>(obj);
    double origin[3];

    auto filter = vtkSmartPointer<vtkMarchingCubes>::New();
    filter->SetInputData(image);
    filter->SetValue(0, 0.5);
    filter->Update();
    /*
        vtkSmartPointer<vtkSmoothPolyDataFilter> smoother =
            vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
        smoother->SetInputData(filter->GetOutput());
        smoother->SetNumberOfIterations(15);
        smoother->SetRelaxationFactor(0.1);
        smoother->FeatureEdgeSmoothingOff();
        smoother->BoundarySmoothingOn();
        smoother->Update();*/


    vtkMatrix4x4 *vtkmatrix = vtkMatrix4x4::New();
    vtkPoints *points = filter->GetOutput()->GetPoints();
    mitk::Vector3D spacing = m_Controls.boundingShapeSelector->GetSelectedNode()->GetData()->GetGeometry()->GetSpacing();
    m_Controls.boundingShapeSelector->GetSelectedNode()->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix(vtkmatrix);
    double(*matrix)[4] = vtkmatrix->Element;
    unsigned int i, j;
    for (i = 0; i < 3; ++i)
        for (j = 0; j < 3; ++j)
            matrix[i][j] /= spacing[j];

    unsigned int n = points->GetNumberOfPoints();
    double point[3];

    for (i = 0; i < n; i++)
    {
        points->GetPoint(i, point);
        mitkVtkLinearTransformPoint(matrix, point, point);
        points->SetPoint(i, point);
    }
    vtkmatrix->Delete();

    /*auto polyData = vtkSmartPointer<vtkPolyData>::New();
    IQF_MitkSurfaceTool* pSurfaceTool = (IQF_MitkSurfaceTool*)GetInterfacePtr(QF_MitkSurface_Tool);
    pSurfaceTool->ExtractSurface(m_Controls.boundingShapeSelector->GetSelectedNode(), polyData);*/

    mitk::Surface::Pointer surface = mitk::Surface::New();
    surface->SetVtkPolyData(filter->GetOutput());
    if (!m_resultSurface)
    {
        m_resultSurface = mitk::DataNode::New();
        GetDataStorage()->Add(m_resultSurface);
    }
    m_resultSurface->SetData(surface);
}

void SphereCropperView::Update(const char * szMessage, int iValue, void * pValue)
{
}

void SphereCropperView::OnDataSelectionChanged(const mitk::DataNode * node)
{
	m_Node = m_Controls.boundingShapeSelector->GetSelectedNode();
	if (m_Node.IsNotNull())
	{
		m_Controls.buttonCreateNewBoundingBox->setEnabled(true);
	}
}

void SphereCropperView::StartInteraction()
{
	if (m_SphereShapeInteractor.IsNotNull())
	{
		m_SphereShapeInteractor->SetDataNode(m_Node);
	}
}

void SphereCropperView::EndInteraction()
{
	if (m_SphereShapeInteractor.IsNotNull())
	{
		m_SphereShapeInteractor->SetDataNode(nullptr);
	}
}