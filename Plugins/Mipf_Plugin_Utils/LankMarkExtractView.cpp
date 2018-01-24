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


double Distance(double* p1,double* p2)
{
    return sqrt((p1[0]-p2[0])*(p1[0] - p2[0])+
        (p1[1] - p2[1])*(p1[1] - p2[1])+
        (p1[2] - p2[2])*(p1[2] - p2[2]));

}

void LankMarkExtractView::Extract()
{
    std::vector<std::vector<double>>  vecModelDistance;

    double p0[] = { -19.0,-9.7,0 };
    double p1[] = { 16.5,-3.7,0 };
    double p2[] = { 0,-12.7,21.5 };
    double p3[] = { 0,-6.7,-18 };


    std::vector<double> vecModelOne;
    vecModelOne.push_back(Distance(p0,p1));
    vecModelOne.push_back(Distance(p0, p2));
    vecModelOne.push_back(Distance(p0, p3));
    vecModelDistance.push_back(vecModelOne);
    std::vector<double> vecModelTwo;
    vecModelTwo.push_back(10);
    vecModelTwo.push_back(12);
    vecModelTwo.push_back(14);
    vecModelDistance.push_back(vecModelTwo);
    std::vector<double> vecModelThree;
    vecModelThree.push_back(7);
    vecModelThree.push_back(9);
    vecModelThree.push_back(11);
    vecModelDistance.push_back(vecModelThree);

    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());
    LandMarkExtractor extractor;
    std::vector<LandMarkPoint> results = LandMarkExtractor::ExtractLandMarks(mitkImage, vecModelDistance, 3.0, 
        m_ui.ThresholdSlider->minimumValue(),m_ui.ThresholdSlider->maximumValue(),
        m_ui.XCutRate->value(), m_ui.YCutRate->value(), m_ui.ZCutRate->value());

    QList<QVector3D> block;
    QList<QVector3D> offset;
    block.append(QVector3D(results[0].Coord.GetElement(0), results[0].Coord.GetElement(1), results[0].Coord.GetElement(2)));
    block.append(QVector3D(results[1].Coord.GetElement(0), results[1].Coord.GetElement(1), results[1].Coord.GetElement(2)));
    block.append(QVector3D(results[2].Coord.GetElement(0), results[2].Coord.GetElement(1), results[2].Coord.GetElement(2)));
    block.append(QVector3D(results[3].Coord.GetElement(0), results[3].Coord.GetElement(1), results[3].Coord.GetElement(2)));
    offset.append(QVector3D(-4.5, -7, -2));
    offset.append(QVector3D(-4.5, -14, -2));
    offset.append(QVector3D(-4.5, 5, -2));
    offset.append(QVector3D(-15.5, -7, -2));

    QMatrix4x4 m = CaculateMatrix(block, offset);

    //////////////////////////////
    vtkSmartPointer<vtkPolyData> linesPolyData =
        vtkSmartPointer<vtkPolyData>::New();

    QVector3D origin = m.column(3).toVector3D();
    QVector3D px = origin + m.column(0).toVector3D() * 10;
    QVector3D py = origin + m.column(1).toVector3D() * 10;
    QVector3D pz = origin + m.column(2).toVector3D() * 10;


    for (int i=0;i<4;i++)
    {
        /* vtkSmartPointer<vtkPoints> pts =
             vtkSmartPointer<vtkPoints>::New();
         vtkSmartPointer<vtkCellArray> vertices =
             vtkSmartPointer<vtkCellArray>::New();
         vtkIdType pid[1];
         pid[0] = pts->InsertNextPoint(results[i].Coord.GetElement(0), results[i].Coord.GetElement(1), results[i].Coord.GetElement(2));
         vertices->InsertNextCell(1, pid);
 */
 // Add the points to the polydata container
      /*  auto pointPolyData = vtkSmartPointer<vtkPolyData>::New();
        pointPolyData->SetPoints(pts);
        pointPolyData->SetVerts(vertices);*/

        vtkSmartPointer<vtkSphereSource> sphereSource =
            vtkSmartPointer<vtkSphereSource>::New();
        sphereSource->SetCenter(results[i].Coord.GetElement(0), results[i].Coord.GetElement(1), results[i].Coord.GetElement(2));
        sphereSource->SetRadius(1.0);
        sphereSource->Update();

        

        QString pointName = QString("Point%1").arg(i);
        mitk::DataNode::Pointer node = ImportVtkPolyData(sphereSource->GetOutput(), pointName.toStdString().c_str());
    }
    // Create a vtkPoints container and store the points in it
    

    ////////////////////////////



    for (int i=0;i<results.size();i++)
    {
        results[i].PrintSelf();
        m_pPointSet->InsertPoint(results[i].Coord);
    }
    return;

}
