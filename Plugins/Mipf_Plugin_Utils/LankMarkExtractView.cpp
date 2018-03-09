#include "LankMarkExtractView.h"


#include "ITKImageTypeDef.h"
#include "mitkImageCast.h"

#include "itkBinaryThresholdImageFilter.h"
#include "itkMedianImageFilter.h"
#include "ITK_Helpers.h"

#include "itkRegionOfInterestImageFilter.h"

#include "LandMarkExtractor.h"

#include <QMatrix4x4>

//vtk
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPoints.h>
#include <vtkLine.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>

int maxSubArray(vector<int> &nums)
{
    if (nums.size()==1)
    {
        return nums[0];
    }
    int cen = nums.size();
    int left = std::accumulate(nums.begin(), nums.begin() + cen, 0);
    int right = std::accumulate(nums.begin() + cen, nums.end() + cen, 0);
}

QMatrix4x4 CaculateMatrix(QList<QVector3D> points, QList<QVector3D> offset)
{
    
    QVector3D x = (points[0] - points[3]).normalized();
    QVector3D y = (points[2] - points[0]).normalized();
    QVector3D z = QVector3D::crossProduct(x, y);

    QMatrix4x4 m;
    m.setToIdentity();
    m.setColumn(0, x.toVector4D());
    m.setColumn(1, y.toVector4D());
    m.setColumn(2, z.toVector4D());

    QVector3D p = points[0] - m*offset[0];
    m.setColumn(3, p.toVector4D());
    m(3, 3) = 1.0;

    return m;
}

double Distance(double* p1,double* p2)
{
    return sqrt((p1[0]-p2[0])*(p1[0] - p2[0])+
        (p1[1] - p2[1])*(p1[1] - p2[1])+
        (p1[2] - p2[2])*(p1[2] - p2[2]));

}


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
    connect(m_ui.AddModelBtn, SIGNAL(clicked()), this, SLOT(AddModel()));
    connect(m_ui.RemoveModelBtn, SIGNAL(clicked()), this, SLOT(RemoveModel()));

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


    m_ui.ModelList->setEditTriggers(QAbstractItemView::DoubleClicked);
    m_ui.ModelList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_ui.ModelList->setSelectionBehavior(QAbstractItemView::SelectItems);
    QStringList defaultModels;
    defaultModels << "10.0,12.0,14.0"<<"7.0,9.0,11.0";
    m_ui.ModelList->addItems(defaultModels);
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
    for (int i=0;i<m_ui.ModelList->count();i++)
    {
        QString str = m_ui.ModelList->item(i)->text();
        if (str.isEmpty())
        {
            continue;
        }
        QStringList distanceList = str.split(QRegExp("[^\\d\.]"));
        qDebug() << distanceList;
        if (distanceList.size()>2)
        {
            std::vector<double> vecModel;
            foreach(QString distance, distanceList)
            {
                vecModel.push_back(distance.toDouble());
            }
            vecModelDistance.push_back(vecModel);
        }
    }

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


void LankMarkExtractView::AddModel()
{
    QListWidgetItem* item = new QListWidgetItem("Please input model");
    item->setFlags(item->flags()|Qt::ItemIsEditable);
    item->setSelected(true);
    m_ui.ModelList->addItem(item);
}

void LankMarkExtractView::RemoveModel()
{
    m_ui.ModelList->takeItem(m_ui.ModelList->currentRow());
}
