#include "GeometryInformationView.h"

GeometryInformationView::GeometryInformationView()
{
}


GeometryInformationView::~GeometryInformationView()
{
}

void GeometryInformationView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.Information->setHeaderLabels(QStringList() << "Name" << "Value");

}

WndHandle GeometryInformationView::GetPluginHandle()
{
    m_pMain->Attach(this);
    return this;
}

void GeometryInformationView::Update(const char* szMessage, int iValue /* = 0 */, void* pValue /* = 0 */)
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
          const mitk::BaseGeometry* geometry = node->GetData()->GetUpdatedGeometry();
           if (!geometry)
           {
               return;
           }
           m_ui.Information->clear();

           QList<QTreeWidgetItem*> items;
           QTreeWidgetItem* item;

           mitk::Point3D origin = geometry->GetOrigin();
           item = new QTreeWidgetItem(QStringList()<<"Origin"<<QString("%1,%2,%3").arg(origin[0]).arg(origin[1]).arg(origin[2]));
           items.append(item);

           mitk::Point3D center = geometry->GetCenter();
           item = new QTreeWidgetItem(QStringList() << "Center" << QString("%1,%2,%3").arg(center[0]).arg(center[1]).arg(center[2]));
           items.append(item);

           mitk::Vector3D spacing = geometry->GetSpacing();
           item = new QTreeWidgetItem(QStringList() << "Spacing" << QString("%1,%2,%3").arg(spacing[0]).arg(spacing[1]).arg(spacing[2]));
           items.append(item);

           item = new QTreeWidgetItem(QStringList() << "Extent" << QString("%1,%2,%3").arg(geometry->GetExtent(0)).arg(geometry->GetExtent(1)).arg(geometry->GetExtent(2)));
           items.append(item);

           item = new QTreeWidgetItem(QStringList() << "Extent In MM" << QString("%1,%2,%3").arg(geometry->GetExtentInMM(0)).arg(geometry->GetExtentInMM(1)).arg(geometry->GetExtentInMM(2)));
           items.append(item);

           mitk::BaseGeometry::BoundsArrayType bounds = geometry->GetBounds();
           item = new QTreeWidgetItem(QStringList() << "Bounds" << QString("%1,%2,%3,%4,%5,%6").arg(bounds[0]).arg(bounds[1]).arg(bounds[2]).arg(bounds[3]).arg(bounds[4]).arg(bounds[5]));
           items.append(item);

           vtkMatrix4x4* m = const_cast<mitk::BaseGeometry*>(geometry)->GetVtkMatrix();
           item = new QTreeWidgetItem(QStringList() << "Vtk Matrix" << QString("%1 %2 %3 %4\n%5 %6 %7 %8\n%9 %10 %11 %12\n%13 %14 %15 %16\n")
               .arg(m->GetElement(0, 0), 8, 'f', 6).arg(m->GetElement(0, 1), 8, 'f', 6).arg(m->GetElement(0, 2), 8, 'f', 6).arg(m->GetElement(0, 3), 8, 'f', 6)
               .arg(m->GetElement(1, 0), 8, 'f', 6).arg(m->GetElement(1, 1), 8, 'f', 6).arg(m->GetElement(1, 2), 8, 'f', 6).arg(m->GetElement(1, 3), 8, 'f', 6)
               .arg(m->GetElement(2, 0), 8, 'f', 6).arg(m->GetElement(2, 1), 8, 'f', 6).arg(m->GetElement(2, 2), 8, 'f', 6).arg(m->GetElement(2, 3), 8, 'f', 6)
               .arg(m->GetElement(3, 0), 8, 'f', 6).arg(m->GetElement(3, 1), 8, 'f', 6).arg(m->GetElement(3, 2), 8, 'f', 6).arg(m->GetElement(3, 3), 8, 'f', 6));
           items.append(item);


           m_ui.Information->addTopLevelItems(items);
           

      }
}
