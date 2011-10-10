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

#ifndef BACKUPLISTMODEL_H
#define BACKUPLISTMODEL_H

#include <QAbstractListModel>
#include <QIcon>
#include "backupengine.h"

class BackupListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit BackupListModel(QObject *parent = 0);

    int rowCount ( const QModelIndex & parent ) const ;
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    Qt::ItemFlags flags ( const QModelIndex & index ) const;
    int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    //QModelIndex index(int, int, const QModelIndex&) const;
    //QModelIndex parent(const QModelIndex&) const;
    void addBackup(BackupSettings *bs);
    void removeBackup(int idx);

    QList<BackupEngine *> backupList;
    QIcon *backupIcon;

signals:

public slots:

};

#endif // BACKUPLISTMODEL_H
