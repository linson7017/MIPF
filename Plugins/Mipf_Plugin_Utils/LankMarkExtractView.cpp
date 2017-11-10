#include "LankMarkExtractView.h"


#include "ITKImageTypeDef.h"
#include "mitkImageCast.h"

#include "itkBinaryThresholdImageFilter.h"
#include "itkMedianImageFilter.h"
#include "ITK_Helpers.h"

#include "itkRegionOfInterestImageFilter.h"

#include "LandMarkExtractor.h"

LankMarkExtractView::LankMarkExtractView()
{
}


LankMarkExtractView::~LankMarkExtractView()
{
}

void LankMarkExtractView::CreateView()
{
    m_ui.setupUi(this);

    connect(m_ui.ExtractBtn, SIGNAL(clicked()), this, SLOT(Extract()));

    m_ui.ImageSelector->SetDataStorage(GetDataStorage());
    m_ui.ImageSelector->SetPredicate(mitk::NodePredicateDataType::New("Image"));
    connect(m_ui.ImageSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode *)),this,SLOT(OnImageSelectionChanged(const mitk::DataNode *)));

    m_ui.PointListWidget->SetMultiWidget(m_pMitkRenderWindow->GetMitkStdMultiWidget());

    mitk::DataNode* tempNode;
    if (m_pPointSet.IsNull())  
        m_pPointSet = mitk::PointSet::New();
    if (m_pPointSetNode.IsNull())
    {
        m_pPointSetNode = mitk::DataNode::New();
        m_pPointSetNode->SetData(m_pPointSet);
        m_pPointSetNode->SetName("landmark points");
        m_pPointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true));
        m_pPointSetNode->SetProperty("label", mitk::StringProperty::New("P"));
        m_pPointSetNode->SetProperty("layer", mitk::IntProperty::New(100));
    }
    tempNode = NULL;
    tempNode = GetDataStorage()->GetNamedNode("landmark points");
    if (tempNode == NULL)
    {
        GetDataStorage()->Add(m_pPointSetNode);
        m_ui.PointListWidget->SetPointSetNode(m_pPointSetNode);
    }


    m_ui.ThresholdSlider->setDecimals(1);
    m_ui.ThresholdSlider->setSpinBoxAlignment(Qt::AlignVCenter);
    m_ui.ThresholdSlider->setMaximum(200000000);
    m_ui.ThresholdSlider->setMinimum(500);
    m_ui.ThresholdSlider->setMaximumValue(5000);
    m_ui.ThresholdSlider->setMinimumValue(1800);

}

void LankMarkExtractView::OnImageSelectionChanged(const mitk::DataNode *node)
{
    if (!node)
    {
        return;
    }
    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());
    m_ui.ThresholdSlider->setMaximum(mitkImage->GetScalarValueMax());
    m_ui.ThresholdSlider->setMinimum(mitkImage->GetScalarValueMin());
}


void LankMarkExtractView::Extract()
{
    std::vector<std::vector<double>>  vecModelDistance;

    std::vector<double> vecModelOne;
    vecModelOne.push_back(7);
    vecModelOne.push_back(9);
    vecModelOne.push_back(11);
    vecModelDistance.push_back(vecModelOne);
    std::vector<double> vecModelTwo;
    vecModelTwo.push_back(10);
    vecModelTwo.push_back(12);
    vecModelTwo.push_back(14);
    vecModelDistance.push_back(vecModelTwo);
    std::vector<double> vecModelThree;
    vecModelThree.push_back(7);
    vecModelThree.push_back(12);
    vecModelThree.push_back(11);
    vecModelDistance.push_back(vecModelThree);

    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());
    LandMarkExtractor extractor;
    std::vector<LandMarkPoint> results = LandMarkExtractor::ExtractLandMarks(mitkImage, vecModelDistance, 3.0, 
        m_ui.ThresholdSlider->minimumValue(),m_ui.ThresholdSlider->maximumValue(),
        m_ui.XCutRate->value(), m_ui.YCutRate->value(), m_ui.ZCutRate->value());
    for (int i=0;i<results.size();i++)
    {
        results[i].PrintSelf();
        m_pPointSet->InsertPoint(results[i].Coord);
    }
    return;

}
