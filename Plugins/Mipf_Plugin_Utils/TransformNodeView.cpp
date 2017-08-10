#include "TransformNodeView.h"


#include <QDebug>

#include <vtkMatrix4x4.h>

#include "mitkDataNode.h"

TransformNodeView::TransformNodeView()
{
}


TransformNodeView::~TransformNodeView()
{
}

void TransformNodeView::CreateView()
{
    m_ui.setupUi(this);

    m_ui.NodeSelector->SetDataStorage(GetDataStorage());   
    m_ui.NodeSelector->SetPredicate(mitk::NodePredicateOr::New(CreatePredicate(1), CreatePredicate(2)));

    connect(m_ui.TransfromBtn, SIGNAL(clicked()), this, SLOT(Transform()));
}

void TransformNodeView::Transform()
{
    QString matrixStr = m_ui.TransformMatrixText->toPlainText();
    matrixStr.replace("\n", " ");
    qDebug() << matrixStr;
    qDebug() << matrixStr.split(" ");
    
    QStringList matrixElements = matrixStr.split(" ");
    vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
    for (int i=0;i<matrixElements.size();i++)
    {
        matrix->SetElement(i / 4, i % 4, matrixElements.at(i).toDouble());
    }

    mitk::DataNode* node = m_ui.NodeSelector->GetSelectedNode();
    vtkMatrix4x4* originMatrix = node->GetData()->GetGeometry()->GetVtkMatrix();
    vtkMatrix4x4::Multiply4x4(matrix, originMatrix, matrix);
    node->GetData()->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(matrix);

    RequestRenderWindowUpdate();
}
