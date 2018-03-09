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

#ifndef NodeTableViewKeyFilter_H_
#define NodeTableViewKeyFilter_H_

#include <QObject>

///
/// A small class which "eats" all Del-Key-pressed events on the node table.
/// When the Del Key is pressed selected nodes should be removed.
///
class NodeTableViewKeyFilter : public QObject
{
  Q_OBJECT
public:
  NodeTableViewKeyFilter(QObject* _DataManagerView = nullptr);
protected:
  bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // NodeTableViewKeyFilter_H_
