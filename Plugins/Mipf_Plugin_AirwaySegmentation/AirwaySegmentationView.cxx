#include "AirwaySegmentationView.h"
#include "iqf_main.h"
#include "Res/R.h"

#include "QmitkPointListWidget.h"
#include <QPushButton>
#include <mitkImageCast.h>

//qmitk
#include <QmitkDataStorageComboBox.h>

#include "Segmentation/IQF_AirwaySegmentation.h"
#include "Segmentation/IQF_SegmentationMethodFactory.h"
#include "MitkStd/IQF_MitkPointList.h"

AirwaySegmentationView::AirwaySegmentationView() :MitkPluginView(), m_pPointList(NULL)
{
	connect(&m_watcher, &QFutureWatcher<void>::finished, this, &AirwaySegmentationView::AirwayFinished);

	m_labelimage = mitk::Image::New();
	m_result = mitk::DataNode::New();    
}

AirwaySegmentationView::~AirwaySegmentationView()
{
    delete m_pPointList;
}

void AirwaySegmentationView::CreateView()
{

    m_ui.setupUi(this);
  
	if (!m_pPointList)
	{
        IQF_PointListFactory* pFactory = (IQF_PointListFactory*)m_pMain->GetInterfacePtr(QF_MitkStd_PointListFactory);
        m_pPointList = pFactory->CreatePointList();
	}

    mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
    m_pPointList->CreateNewPointSetNode(pointSetNode);

    m_ui.ImageSelector->SetDataStorage(GetDataStorage());
    m_ui.ImageSelector->SetPredicate(CreatePredicate(Image));


    connect(m_ui.SeedSelectBtn,SIGNAL(clicked(bool)) , this, SLOT(OnSelectSeed(bool)));
    connect(m_ui.SeedClearBtn, SIGNAL(clicked()), this, SLOT(OnClearSeed()));
    connect(m_ui.SegmentBtn, SIGNAL(clicked()), this, SLOT(Segment()));

}


void AirwaySegmentationView::OnSelectSeed(bool bSelecting)
{
     if (m_pPointList)
     {
         m_pPointList->AddPoint(bSelecting);
     }
}

void AirwaySegmentationView::OnClearSeed()
{
    if (m_pPointList)
    {
        m_pPointList->GetPointSet()->Clear();
    }
}


void AirwaySegmentationView::Segment()
{
    m_ui.SegmentBtn->setText("Wait...");
    m_ui.SegmentBtn->setDisabled(true);
	m_future = QtConcurrent::run(this, &AirwaySegmentationView::DoSomething);
	m_watcher.setFuture(m_future);
}

void AirwaySegmentationView::AirwayFinished()
{
	//GetDataStorage()->Remove(GetDataStorage()->GetNamedNode("Result"));
	if (!m_result)
	{
		m_result = mitk::DataNode::New();
	}
	m_result->SetData(m_labelimage);
	m_result->SetName("Result");
	GetDataStorage()->Add(m_result);

    m_ui.SegmentBtn->setText( "Done");
    m_ui.SegmentBtn->setDisabled(false);
	RequestRenderWindowUpdate();
}

void AirwaySegmentationView::DoSomething()
{
	#define DIM 3

	typedef signed short    InputPixelType;
	typedef unsigned short  OutputPixelType;

	typedef itk::Image<InputPixelType, DIM>  InputImageType;
	typedef itk::Image<OutputPixelType, DIM> OutputImageType;

	mitk::DataNode::Pointer sourceimage = m_ui.ImageSelector->GetSelectedNode();
	mitk::Image* mitkSourceImage = dynamic_cast<mitk::Image*>(sourceimage->GetData());


	InputImageType::Pointer itkSourceImage = InputImageType::New();
	mitk::CastToItkImage<InputImageType>(mitkSourceImage, itkSourceImage);

	mitk::Point3D SeedPoint = m_pPointList->GetPointSet()->GetPoint(0);
	double seed[3];
	seed[0] = SeedPoint[0];
	seed[1] = SeedPoint[1];
	seed[2] = SeedPoint[2];

	OutputImageType::Pointer itkResultImage = OutputImageType::New();

    IQF_SegmentationMethodFactory* pFactory = (IQF_SegmentationMethodFactory*)m_pMain->GetInterfacePtr(QF_Segmentation_Factory);
    if (!pFactory)
    {    
        return;
    }
    IQF_AirwaySegmentation* pSeg = pFactory->CreateAirwaySegmentationMethod();
	if (pSeg)
	{
		pSeg->SetSourceImage((TInputImageType*)itkSourceImage.GetPointer());
		pSeg->SetLabelImage((TOutputImageType*)itkResultImage.GetPointer());
		pSeg->SetSeedPoint(seed);
		pSeg->DoSegmentation();

		mitk::CastToMitkImage(itkResultImage, m_labelimage);
	}
	
}


