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

static void QMatrixToVtkMatrix(const QMatrix4x4& qm, vtkMatrix4x4* vm)
{
    vm->Identity();
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            vm->SetElement(i, j, qm(i, j));
        }
    }
}

void TransformNode(mitk::DataNode* node,QMatrix4x4 transform)
{
    vtkMatrix4x4* matrix = node->GetData()->GetGeometry()->GetVtkMatrix();
    auto tm =  vtkSmartPointer<vtkMatrix4x4>::New();
    auto rm = vtkSmartPointer<vtkMatrix4x4>::New();

    QMatrixToVtkMatrix(transform, tm.Get());
    vtkMatrix4x4::Multiply4x4(tm.Get(), matrix, rm.Get());
    node->GetData()->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(rm);
}

void Use()
{
    //计算坐标系变换矩阵
    QMatrix4x4 initImgCoord = CaculateMatrix();   //计算图像坐标系下磁定位片矩阵
    QMatrix4x4 initMagCoord;  //磁定位坐标系下此定位片矩阵
    QMatrix4x4 transform = initImgCoord*initMagCoord.inverted();  //获得两个坐标系的变换矩阵
    
    //根据当前磁定位器矩阵计算图像的变换矩阵
    mitk::DataNode::Pointer node;
    QMatrix4x4 currentMagCoord;
    QMatrix4x4 currentImageCoord = transform * currentMagCoord;
    QMatrix4x4 currentImageTransform = currentImageCoord * initImgCoord.inverted();
    TransformNode(node, currentImageTransform);

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


void LankMarkExtractView::Extract()
{
    std::vector<std::vector<double>>  vecModelDistance;

    std::vector<double> vecModelOne;
    vecModelOne.push_back(7);
    vecModelOne.push_back(12);
    vecModelOne.push_back(11);
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


    // Create a vtkPoints container and store the points in it
    vtkSmartPointer<vtkPoints> pts =
        vtkSmartPointer<vtkPoints>::New();
    pts->InsertNextPoint(origin.x(),origin.y(),origin.z());
    pts->InsertNextPoint(px.x(), px.y(), px.z());
    pts->InsertNextPoint(py.x(), py.y(), py.z());
    pts->InsertNextPoint(pz.x(), pz.y(), pz.z());

    // Add the points to the polydata container
    linesPolyData->SetPoints(pts);

    // Create the first line (between Origin and P0)
    vtkSmartPointer<vtkLine> line0 =
        vtkSmartPointer<vtkLine>::New();
    line0->GetPointIds()->SetId(0, 0); // the second 0 is the index of the Origin in linesPolyData's points
    line0->GetPointIds()->SetId(1, 1); // the second 1 is the index of P0 in linesPolyData's points

                                       // Create the second line (between Origin and P1)
    vtkSmartPointer<vtkLine> line1 =
        vtkSmartPointer<vtkLine>::New();
    line1->GetPointIds()->SetId(0, 0); // the second 0 is the index of the Origin in linesPolyData's points
    line1->GetPointIds()->SetId(1, 2); // 2 is the index of P1 in linesPolyData's points

                                       // Create the second line (between Origin and P1)
    vtkSmartPointer<vtkLine> line2 =
        vtkSmartPointer<vtkLine>::New();
    line2->GetPointIds()->SetId(0, 0); // the second 0 is the index of the Origin in linesPolyData's points
    line2->GetPointIds()->SetId(1, 3); // 2 is the index of P1 in linesPolyData's points

                                       // Create a vtkCellArray container and store the lines in it
    vtkSmartPointer<vtkCellArray> lines =
        vtkSmartPointer<vtkCellArray>::New();
    lines->InsertNextCell(line0);
    lines->InsertNextCell(line1);
    lines->InsertNextCell(line2);


    // Add the lines to the polydata container
    linesPolyData->SetLines(lines);

    unsigned char red[3] = { 255, 0, 0 };
    unsigned char green[3] = { 0, 255, 0 };
    unsigned char blue[3] = { 0, 0, 255 };


    // Create a vtkUnsignedCharArray container and store the colors in it
    vtkSmartPointer<vtkUnsignedCharArray> colors =
        vtkSmartPointer<vtkUnsignedCharArray>::New();
    colors->SetNumberOfComponents(3);
    colors->InsertNextTupleValue(red);
    colors->InsertNextTupleValue(green);
    colors->InsertNextTupleValue(blue);

    linesPolyData->GetCellData()->SetScalars(colors);

    mitk::DataNode::Pointer node = ImportVtkPolyData(linesPolyData, "coord");

    ////////////////////////////



    for (int i=0;i<results.size();i++)
    {
        results[i].PrintSelf();
        m_pPointSet->InsertPoint(results[i].Coord);
    }
    return;

}
