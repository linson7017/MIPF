/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QmitkPropertyTreeView_h
#define QmitkPropertyTreeView_h

#include "MitkPluginView.h"

#include <mitkDataNode.h>
#include <ui_QmitkPropertyTreeView.h>

class QmitkPropertyItemDelegate;
class QmitkPropertyItemModel;
class QmitkPropertyItemSortFilterProxyModel;

namespace mitk
{
  class IPropertyAliases;
  class IPropertyDescriptions;
  class IPropertyPersistence;
}

class QmitkPropertyTreeView :public QWidget, public MitkPluginView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

 // berryObjectMacro(QmitkPropertyTreeView);

  QmitkPropertyTreeView();
  ~QmitkPropertyTreeView();

  void SetFocus();

 // void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  //void RenderWindowPartDeactivated(mitk::IRenderWindowPart*) override;

  void CreateView() override;
  WndHandle GetPluginHandle() { return this; }
protected:
    void Update(const char* szMessage, int iValue /* = 0 */, void* pValue /* = 0 */);

private:
  QString GetPropertyNameOrAlias(const QModelIndex& index);
  void OnPreferencesChanged() ;
  void OnPropertyNameChanged(const itk::EventObject& event);
  void OnSelectionChanged(const QList<mitk::DataNode::Pointer>& nodes) ;

private slots:
  void OnCurrentRowChanged(const QModelIndex& current, const QModelIndex& previous);
  void OnPropertyListChanged(int index);
  void OnAddNewProperty();
  void OnFilterTextChanged(const QString& filter);
  void OnModelReset();

private:
  QWidget* m_Parent;
  unsigned long m_PropertyNameChangedTag;
  std::string m_SelectionClassName;
  mitk::IPropertyAliases* m_PropertyAliases;
  mitk::IPropertyDescriptions* m_PropertyDescriptions;
  mitk::IPropertyPersistence* m_PropertyPersistence;
  bool m_ShowAliasesInDescription;
  bool m_ShowPersistenceInDescription;
  bool m_DeveloperMode;
  Ui::QmitkPropertyTreeView m_Controls;
  QmitkPropertyItemSortFilterProxyModel* m_ProxyModel;
  QmitkPropertyItemModel* m_Model;
  QmitkPropertyItemDelegate* m_Delegate;
  mitk::DataNode::Pointer m_SelectedNode;
  mitk::BaseRenderer* m_Renderer;
};

#endif
