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

#include "onlycheckedfoldertree.h"
#include "trace.h"


OnlyCheckedFolderTree::OnlyCheckedFolderTree(CheckableFolderTree * Sources_,DestinationWidget * Dest_, QObject *parent):
    QAbstractItemModel(parent)
{
    TRACE(qDebug("Calling OnlyCheckedFolderTree::constructor");)
    Sources = Sources_;
    Dest = Dest_;
    connect(Sources,SIGNAL(dataChanged(QModelIndex,QModelIndex)),this,SLOT(SourceDataChanged()));
}

void OnlyCheckedFolderTree::SourceDataChanged (){
    TRACE(qDebug("Calling OnlyCheckedFolderTree::SourceDataChanged"));
    dataChanged(QModelIndex(),QModelIndex());
}

QModelIndex OnlyCheckedFolderTree::index(int row, int column, const QModelIndex &parent) const{
    //TRACE(qDebug("Calling OnlyCheckedFolderTree::index(%d,%d,%d,%d)",row,column,parent.row(),parent.column()));
    if (!parent.isValid()) {
        TRACE(qDebug("Calling OnlyCheckedFolderTree::index(%d,%d,invalid)",row,column));
        return createIndex(row,column,(quint32) Dest->lines->at(row));
    } else if (Dest->lines->contains((DestinationLineWidget *)parent.internalPointer())) {
        TRACE(qDebug("Calling OnlyCheckedFolderTree::index(%d,%d,r)",row,column));
        return createIndex(row,column,((quint32)(&Sources->fullyCheckedFolders.at(row))+parent.row()));
    }
    TRACE(qDebug("Calling OnlyCheckedFolderTree::index(%d,%d,else)",row,column));
    return QModelIndex();
}

QModelIndex OnlyCheckedFolderTree::parent(const QModelIndex &child) const{
    if (Dest->lines->contains((DestinationLineWidget *)child.internalPointer())) {
        TRACE(qDebug("Calling OnlyCheckedFolderTree::parent(r)"));
        return QModelIndex();
    } else if (child.isValid()) {
        TRACE(qDebug("Calling OnlyCheckedFolderTree::parent(valid)"));
        int parentIdx = (quint32)(child.internalPointer() - (quint32)(&Sources->fullyCheckedFolders.at(child.row())));
        return createIndex(parentIdx,0,(quint32) Dest->lines->at(parentIdx));
    }
    TRACE(qDebug("Calling OnlyCheckedFolderTree::parent(else)"));
    return QModelIndex();
}

int OnlyCheckedFolderTree::rowCount(const QModelIndex &parent) const{
    if (!parent.isValid()) {
        TRACE(qDebug("Calling OnlyCheckedFolderTree::rowcount(invalid)"));
        return std::max(Dest->lines->size()-1,1);
    } else if (Dest->lines->contains((DestinationLineWidget *)parent.internalPointer())){
        TRACE(qDebug("Calling OnlyCheckedFolderTree::rowcount(r)"));
        return Sources->fullyCheckedFolders.size();
    } else {
        TRACE(qDebug("Calling OnlyCheckedFolderTree::rowcount(else)"));
        return 0;
    }
}

int OnlyCheckedFolderTree::columnCount(const QModelIndex &parent) const{
    TRACE(qDebug("Calling OnlyCheckedFolderTree::columncount");)
    return 1;
}

QVariant OnlyCheckedFolderTree::data(const QModelIndex &index, int role) const{
    if (role==Qt::DisplayRole) {
        if (Dest->lines->contains((DestinationLineWidget *)index.internalPointer())) {
            TRACE(qDebug("Calling OnlyCheckedFolderTree::data(%d,%d,role=%d) on root",index.row(),index.column(),role ));
            QString res = Dest->lines->at(index.row())->destLine->text();
            if (res.size()>0) {
                return res;
            } else {
                return "[" + tr("Destination Folder") + "]";
            }
        } else if (index.isValid()){
            TRACE(qDebug("Calling OnlyCheckedFolderTree::data(%d,%d,role=%d) on valid",index.row(),index.column(),role ));
            return Sources->fullyCheckedFolders.relativePathDisplay(index.row());
        }
    }
    return QVariant();
}

Qt::ItemFlags OnlyCheckedFolderTree::flags(const QModelIndex &index) const{
    TRACE(qDebug("Calling OnlyCheckedFolderTree::flags(%d,%d)",index.row(),index.column());)
    return Qt::ItemIsSelectable;
}
