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

#ifndef ONLYCHECKEDFOLDERTREE_H
#define ONLYCHECKEDFOLDERTREE_H
#include <QAbstractItemModel>
#include <QLineEdit>
#include "checkablefoldertree.h"
#include "destinationwidget.h"

class OnlyCheckedFolderTree : public QAbstractItemModel
{
    Q_OBJECT
public:
    OnlyCheckedFolderTree(CheckableFolderTree * Sources_,DestinationWidget * Dest_, QObject *parent = 0);

    QModelIndex index(int, int, const QModelIndex&) const;
    QModelIndex parent(const QModelIndex&) const;
    int rowCount(const QModelIndex&) const;
    int columnCount(const QModelIndex&) const;
    QVariant data(const QModelIndex&, int) const;
    //bool hasIndex ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    //void fetchMore ( const QModelIndex & parent );
    //bool canFetchMore ( const QModelIndex & parent ) const;
    //bool setData(const QModelIndex &idx, const QVariant &value, int role);
    //QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    //void sort(int column, Qt::SortOrder order);
    //bool hasChildren(const QModelIndex &parent) const;
    //Qt::DropActions supportedDropActions() const;


public slots:
    void SourceDataChanged ();
private:
    CheckableFolderTree * Sources;
    DestinationWidget * Dest;

};

#endif // ONLYCHECKEDFOLDERTREE_H
