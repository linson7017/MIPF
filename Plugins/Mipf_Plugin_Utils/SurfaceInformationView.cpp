#include "SurfaceInformationView.h"

#include <vtkPolyData.h>
#include <vtkCenterOfMass.h>

SurfaceInformationView::SurfaceInformationView()
{
}


SurfaceInformationView::~SurfaceInformationView()
{
}

void SurfaceInformationView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.Information->setHeaderLabels(QStringList() << "Name" << "Value");

}

WndHandle SurfaceInformationView::GetPluginHandle()
{
    m_pMain->Attach(this);
    return this;
}

void SurfaceInformationView::Update(const char* szMessage, int iValue /* = 0 */, void* pValue /* = 0 */)
{
      if (strcmp(szMessage,MITK_MESSAGE_NODE_SELECTION_CHANGED)==0)
      {
          std::vector<mitk::DataNode::Pointer> nodes = GetMitkDataManagerInterface()->GetSelectedNodes();
          if (nodes.size()==0)
          {
              return;
          }
          mitk::DataNode* node = nodes.front().GetPointer();
          node->UpdateOutputInformation();
          mitk::Surface* surface = dynamic_cast<mitk::Surface*>(node->GetData());
           if (!surface)
           {
               return;
           }
           vtkPolyData* polydata = surface->GetVtkPolyData();
           m_ui.Information->clear();

           QList<QTreeWidgetItem*> items;
           QTreeWidgetItem* item;

           vtkIdType num = polydata->GetNumberOfPoints();
           item = new QTreeWidgetItem(QStringList() << "Points" << QString("%1").arg(num));
           items.append(item);

           num = polydata->GetNumberOfCells();
           item = new QTreeWidgetItem(QStringList() << "Cells" << QString("%1").arg(num));
           items.append(item);

           num = polydata->GetNumberOfVerts();
           item = new QTreeWidgetItem(QStringList()<<"Verts"<<QString("%1").arg(num));
           items.append(item);

           num = polydata->GetNumberOfLines();
           item = new QTreeWidgetItem(QStringList() << "Lines" << QString("%1").arg(num));
           items.append(item);

           num = polydata->GetNumberOfPolys();
           item = new QTreeWidgetItem(QStringList() << "Polys" << QString("%1").arg(num));
           items.append(item);

           num = polydata->GetNumberOfStrips();
           item = new QTreeWidgetItem(QStringList() << "Strips" << QString("%1").arg(num));
           items.append(item);


           vtkSmartPointer<vtkCenterOfMass> centerOfMassFilter =
               vtkSmartPointer<vtkCenterOfMass>::New();
           centerOfMassFilter->SetInputData(polydata);
           centerOfMassFilter->SetUseScalarsAsWeights(false);
           centerOfMassFilter->Update();
           num = polydata->GetNumberOfStrips();
           double center[3];
           centerOfMassFilter->GetCenter(center);
           item = new QTreeWidgetItem(QStringList() << "Center of Mass" << QString("%1,%2,%3").arg(center[0]).arg(center[1]).arg(center[2]));
           items.append(item);

           m_ui.Information->addTopLevelItems(items);
           

      }
}
