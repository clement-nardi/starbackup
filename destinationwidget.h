/* This file is part of StarBackup
 * StarBackup is a Free Real-Time Backup and Archiving Software
 *
 * Copyright (C) 2011  Clément Nardi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DESTINATIONWIDGET_H
#define DESTINATIONWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "destfolderinfo.h"

class DestinationLineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DestinationLineWidget(QWidget *parent = 0);

    QLineEdit   *destLine;
    QPushButton *removeButton;
    QPushButton *editButton;

signals:

public slots:
    void browseFolder();
    void eraseFolder();

};

class DestinationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DestinationWidget(QWidget *parent = 0);

    void addLine(QString str = "");
    void resetDirs(QList<DestFolderInfo> list);

    QVBoxLayout *layout;
    QList<DestinationLineWidget *> *lines;

signals:
    void dataChanged();

public slots:
    void updateLines();
};

#endif // DESTINATIONWIDGET_H
