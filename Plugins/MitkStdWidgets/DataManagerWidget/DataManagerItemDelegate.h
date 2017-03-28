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

#ifndef DataManagerItemDelegate_h
#define DataManagerItemDelegate_h

#include <QStyledItemDelegate>

class DataManagerItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DataManagerItemDelegate(QObject* parent = nullptr);
    ~DataManagerItemDelegate();

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
};

#endif
