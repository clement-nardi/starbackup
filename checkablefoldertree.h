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

#ifndef CHECKABLEFOLDERTREE_H
#define CHECKABLEFOLDERTREE_H

#include <QFileSystemModel>
#include "folderlist.h"


class CheckableFolderTree : public QFileSystemModel
{
    Q_OBJECT
public:
    explicit CheckableFolderTree(QObject *parent = 0);
    QVariant data ( const QModelIndex & index, int role ) const;
    int columnCount ( const QModelIndex & parent) const;
    Qt::ItemFlags flags ( const QModelIndex & index ) const;
    bool setData ( const QModelIndex & index, const QVariant & value, int role );
    bool hasChildren(const QModelIndex &parent) const;

    void setCheckedFolders(FolderList list);

    FolderList fullyCheckedFolders;

private:

    void checkAllChildrenBut(QModelIndex checkedParentIndex,QString path);

signals:

public slots:

};

#endif // CHECKABLEFOLDERTREE_H
