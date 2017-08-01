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

AirwaySegmentationView::AirwaySegmentationView(QF::IQF_Main* pMain) :MitkPluginView(pMain), m_pSourceImageSelector(NULL)
{
    m_pMain->Attach(this);
	connect(&m_watcher, &QFutureWatcher<void>::finished, this, &AirwaySegmentationView::AirwayFinished);

	m_labelimage = mitk::Image::New();
	m_result = mitk::DataNode::New();

	IQF_MitkDataManager* pdata = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
	if (pdata)
	{
		pdata->GetDataStorage();
	}
}

void AirwaySegmentationView::Contructed(R* pR)
{
	QmitkPointListWidget* pw = (QmitkPointListWidget*)pR->getObjectFromGlobalMap("AirwaySegmentation.SeedWidget");
	if (pw)
	{
		m_PointSet = mitk::PointSet::New();
		mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
		pointSetNode->SetData(m_PointSet);
		pointSetNode->SetName("seed points for tracking");
		pointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true));
		pointSetNode->SetProperty("layer", mitk::IntProperty::New(1024));
		// add the pointset to the data storage (for rendering and access by other modules)
		GetDataStorage()->Add(pointSetNode);
		// tell the GUI widget about the point set
		pw->SetPointSetNode(pointSetNode);
		pw->SetMultiWidget(m_pMitkRenderWindow->GetMitkStdMultiWidget());
	}

	m_pSourceImageSelector = (QmitkDataStorageComboBox*)R::Instance()->getObjectFromGlobalMap("AirwaySegmentation.ImageSelector");
    if (m_pSourceImageSelector)
    {
        m_pSourceImageSelector->SetDataStorage(GetDataStorage());
        m_pSourceImageSelector->SetPredicate(CreatePredicate(1));
    }
}

void AirwaySegmentationView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "MITK_MESSAGE_AIRWAY_SEGMENT") == 0)
    {
        SetGuiProperty("AirwaySegmentation.Segment", "text","Please wait...");
		m_future = QtConcurrent::run(this, &AirwaySegmentationView::DoSomething);
		m_watcher.setFuture(m_future);
		//do what you want for the message
    }
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

    SetGuiProperty("AirwaySegmentation.Segment", "text", "Done");
	RequestRenderWindowUpdate();
}

void AirwaySegmentationView::DoSomething()
{
	#define DIM 3

	typedef signed short    InputPixelType;
	typedef unsigned short  OutputPixelType;

	typedef itk::Image<InputPixelType, DIM>  InputImageType;
	typedef itk::Image<OutputPixelType, DIM> OutputImageType;

	mitk::DataNode::Pointer sourceimage = m_pSourceImageSelector->GetSelectedNode();
	mitk::Image* mitkSourceImage = dynamic_cast<mitk::Image*>(sourceimage->GetData());


	InputImageType::Pointer itkSourceImage = InputImageType::New();
	mitk::CastToItkImage<InputImageType>(mitkSourceImage, itkSourceImage);

	mitk::Point3D SeedPoint = m_PointSet->GetPoint(0);
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


