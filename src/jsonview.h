/****************************************************************************
**
** Copyright (C) 2019 Shawn Rutledge
**
** This file is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** version 3 as published by the Free Software Foundation
** and appearing in the file LICENSE included in the packaging
** of this file.
**
** This code is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
****************************************************************************/

#ifndef JSONVIEW_H
#define JSONVIEW_H

#include <QTreeView>
#include "qjsonmodel.h"

class JsonView : public QTreeView
{
public:
    JsonView(QWidget *parent = nullptr);
    void load(const QByteArray &buf);

protected:

    QJsonModel m_model;
};

#endif // JSONVIEW_H
