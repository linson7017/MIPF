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

#include "NodeDescriptorManager.h"
#include <memory>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>

#include <QList>
#include <QSet>


void NodeDescriptorManager::Initialize()
{
  auto isImage = mitk::NodePredicateDataType::New("Image");
  this->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("Image"), QString(":/Qmitk/Images_48.png"), isImage, this));

  auto isMultiComponentImage =
    mitk::NodePredicateAnd::New(isImage, mitk::NodePredicateProperty::New("Image.Displayed Component"));
  this->AddDescriptor(new QmitkNodeDescriptor(
    tr("MultiComponentImage"), QString(": / Qmitk / Images_48.png"), isMultiComponentImage, this));

  auto isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  auto isBinaryImage = mitk::NodePredicateAnd::New(isBinary, isImage);
  this->AddDescriptor(
    new QmitkNodeDescriptor(tr("ImageMask"), QString(":/Qmitk/Binaerbilder_48.png"), isBinaryImage, this));

  auto isLabelSetImage = mitk::NodePredicateDataType::New("LabelSetImage");
  this->AddDescriptor(
    new QmitkNodeDescriptor(tr("LabelSetImage"), QString(":/Qmitk/LabelSetImage_48.png"), isLabelSetImage, this));

  auto isPointSet = mitk::NodePredicateDataType::New("PointSet");
  this->AddDescriptor(new QmitkNodeDescriptor(tr("PointSet"), QString(":/Qmitk/PointSet_48.png"), isPointSet, this));

  auto isSurface = mitk::NodePredicateDataType::New("Surface");
  this->AddDescriptor(new QmitkNodeDescriptor(tr("Surface"), QString(":/Qmitk/Surface_48.png"), isSurface, this));

  auto isNotBinary = mitk::NodePredicateNot::New(isBinary);
  auto isNoneBinaryImage = mitk::NodePredicateAnd::New(isImage, isNotBinary);
  this->AddDescriptor(
    new QmitkNodeDescriptor(tr("NoneBinaryImage"), QString(":/Qmitk/Images_48.png"), isNoneBinaryImage, this));
}

void NodeDescriptorManager::AddDescriptor(QmitkNodeDescriptor *_Descriptor)
{
  _Descriptor->setParent(this);
  m_NodeDescriptors.push_back(_Descriptor);
}

void NodeDescriptorManager::RemoveDescriptor(QmitkNodeDescriptor *_Descriptor)
{
  int index = m_NodeDescriptors.indexOf(_Descriptor);

  if (index != -1)
  {
    m_NodeDescriptors.removeAt(index);
    _Descriptor->setParent(0);
    delete _Descriptor;
  }
}

QmitkNodeDescriptor *NodeDescriptorManager::GetDescriptor(const mitk::DataNode *_Node) const
{
  QmitkNodeDescriptor *_Descriptor = m_UnknownDataNodeDescriptor;

  for (QList<QmitkNodeDescriptor *>::const_iterator it = m_NodeDescriptors.begin(); it != m_NodeDescriptors.end(); ++it)
  {
    if ((*it)->CheckNode(_Node))
      _Descriptor = *it;
  }

  return _Descriptor;
}

QmitkNodeDescriptor *NodeDescriptorManager::GetDescriptor(const QString &_ClassName) const
{
  QmitkNodeDescriptor *_Descriptor = 0;

  if (_ClassName == "Unknown")
  {
    return m_UnknownDataNodeDescriptor;
  }
  else
  {
    for (QList<QmitkNodeDescriptor *>::const_iterator it = m_NodeDescriptors.begin(); it != m_NodeDescriptors.end();
         ++it)
    {
      if ((*it)->GetNameOfClass() == _ClassName)
        _Descriptor = *it;
    }
  }

  return _Descriptor;
}
QList<QAction *> NodeDescriptorManager::GetActions(const mitk::DataNode *_Node) const
{
  QList<QAction *> actions = m_UnknownDataNodeDescriptor->GetBatchActions();
  actions.append(m_UnknownDataNodeDescriptor->GetActions());
  QmitkNodeDescriptor *lastDescriptor = m_UnknownDataNodeDescriptor;

  for (QList<QmitkNodeDescriptor *>::const_iterator it = m_NodeDescriptors.begin(); it != m_NodeDescriptors.end(); ++it)
  {
    if ((*it)->CheckNode(_Node))
    {
      actions.append(lastDescriptor->GetSeparator());
      lastDescriptor = *it;
      actions.append(lastDescriptor->GetBatchActions());
      actions.append(lastDescriptor->GetActions());
    }
  }

  return actions;
}

QList<QAction *> NodeDescriptorManager::GetActions(const QList<mitk::DataNode::Pointer> &_Nodes) const
{
  QList<QAction *> actions = m_UnknownDataNodeDescriptor->GetBatchActions();
  QSet<QmitkNodeDescriptor *> nodeDescriptors;
  QmitkNodeDescriptor *lastDescriptor;

  // find all descriptors for the nodes (unique)
  foreach (mitk::DataNode::Pointer node, _Nodes)
  {
    lastDescriptor = this->GetDescriptor(node);
    nodeDescriptors.insert(lastDescriptor);
  }
  // add all actions for the found descriptors
  lastDescriptor = m_UnknownDataNodeDescriptor;
  foreach (QmitkNodeDescriptor *descr, nodeDescriptors)
  {
    actions.append(lastDescriptor->GetSeparator());
    lastDescriptor = descr;
    actions.append(lastDescriptor->GetBatchActions());
  }

  return actions;
}

NodeDescriptorManager::NodeDescriptorManager()
  : m_UnknownDataNodeDescriptor(new QmitkNodeDescriptor("Unknown", QString(":/Qmitk/DataTypeUnknown_48.png"), 0, this))
{
  this->Initialize();
}

NodeDescriptorManager::~NodeDescriptorManager()
{
  // delete m_UnknownDataNodeDescriptor;
  // qDeleteAll(m_NodeDescriptors);
}

QmitkNodeDescriptor *NodeDescriptorManager::GetUnknownDataNodeDescriptor() const
{
  return m_UnknownDataNodeDescriptor;
}
