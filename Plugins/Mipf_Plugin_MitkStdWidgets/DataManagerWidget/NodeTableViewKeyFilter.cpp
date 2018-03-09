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

#include "NodeTableViewKeyFilter.h"

#include <QKeyEvent>
#include <QKeySequence>
#include "DataManagerWidget.h"


NodeTableViewKeyFilter::NodeTableViewKeyFilter( QObject* _DataManagerView )
: QObject(_DataManagerView)
{
  
}

bool NodeTableViewKeyFilter::eventFilter( QObject *obj, QEvent *event )
{
    DataManagerWidget* _DataManagerView = qobject_cast<DataManagerWidget*>(this->parent());
  if (event->type() == QEvent::KeyPress && _DataManagerView)
  {
    //hotkeys
    QKeySequence _MakeAllInvisible = QKeySequence(tr("Ctrl+, V"));    //"Make all nodes invisible", 
    QKeySequence _ToggleVisibility = QKeySequence(tr( "V"));                  //"Toggle visibility of selected nodes",
    QKeySequence _DeleteSelectedNodes = QKeySequence(tr( "Del"));      //                           "Delete selected nodes",
    QKeySequence _Reinit = QKeySequence(tr("R"));                                //        "Reinit selected nodes"
    QKeySequence _GlobalReinit = QKeySequence(tr( "Ctrl+, R"));            //    "Global Reinit",
    QKeySequence _ShowInfo = QKeySequence(tr("Ctrl+, I"));                   //  "Show Node Information",

    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

    QKeySequence _KeySequence = QKeySequence(keyEvent->modifiers(), keyEvent->key());
    // if no modifier was pressed the sequence is now empty
    if(_KeySequence.isEmpty())
      _KeySequence = QKeySequence(keyEvent->key());

    if(_KeySequence == _MakeAllInvisible)
    {
      // trigger deletion of selected node(s)
      _DataManagerView->MakeAllNodesInvisible(true);
      // return true: this means the delete key event is not send to the table
      return true;
    }
    else if(_KeySequence == _DeleteSelectedNodes)
    {
      // trigger deletion of selected node(s)
      _DataManagerView->RemoveSelectedNodes(true);
      // return true: this means the delete key event is not send to the table
      return true;
    }
    else if(_KeySequence == _ToggleVisibility)
    {
      // trigger deletion of selected node(s)
      _DataManagerView->ToggleVisibilityOfSelectedNodes(true);
      // return true: this means the delete key event is not send to the table
      return true;
    }
    else if(_KeySequence == _Reinit)
    {
      _DataManagerView->ReinitSelectedNodes(true);
      return true;
    }
    else if(_KeySequence == _GlobalReinit)
    {
      _DataManagerView->GlobalReinit(true);
      return true;
    }
    else if(_KeySequence == _ShowInfo)
    {
      _DataManagerView->ShowInfoDialogForSelectedNodes(true);
      return true;
    }
  }
  // standard event processing
  return QObject::eventFilter(obj, event);
}
