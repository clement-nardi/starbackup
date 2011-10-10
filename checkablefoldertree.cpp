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



#include "trace.h"
#include "checkablefoldertree.h"

CheckableFolderTree::CheckableFolderTree(QObject *parent) :
    QFileSystemModel(parent)
{
    fullyCheckedFolders = FolderList();
    setRootPath(QDir::rootPath());
    setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
}

int CheckableFolderTree::columnCount ( const QModelIndex & parent) const {
    return 1;
}

Qt::ItemFlags CheckableFolderTree::flags ( const QModelIndex & index ) const {
    TRACE(qDebug("Calling CheckableFolderTree::flags(%d,%d)",index.row(),index.column() );)
    return QFileSystemModel::flags(index) | Qt::ItemIsUserCheckable;
}

QVariant CheckableFolderTree::data ( const QModelIndex & index, int role ) const {
    if (role == Qt::CheckStateRole) {
        TRACE(qDebug("Calling CheckableFolderTree::data(%d,%d,role=%d)",index.row(),index.column(),role ));
        if (fullyCheckedFolders.contains((Folder)filePath(index))) {
            return Qt::Checked;
        } else {
            int result = Qt::Unchecked;
            for (int i = 0; i<fullyCheckedFolders.size(); i++) {
                if (filePath(index).startsWith(fullyCheckedFolders.at(i)+"/")) { /* parent is checked */
                    result = Qt::Checked;
                    break;
                }
                if (fullyCheckedFolders.at(i).startsWith(filePath(index)+"/")) {
                    result = Qt::PartiallyChecked;
                }
            }
            return result;
        }
        return Qt::Unchecked;
    } else {
        return QFileSystemModel::data(index,role);
    }
}

bool CheckableFolderTree::setData ( const QModelIndex & index, const QVariant & value, int role ) {

    if (role == Qt::CheckStateRole) {
        TRACE(qDebug("Calling CheckableFolderTree::setData(%d,%d,role=%d,value=%d)",index.row(),index.column(),role,value.toInt() ));
        TRACE(qDebug()<<filePath(index));
        if (value == Qt::Checked || value == Qt::Unchecked) {
            for (int i = 0; i<fullyCheckedFolders.size(); i++) {
                QString path = fullyCheckedFolders.at(i);
                if (path.startsWith(filePath(index)+"/")) {
                    fullyCheckedFolders.removeAt(i);
                    i--;
                }
            }
        }
        if (value == Qt::Checked) {
            fullyCheckedFolders.append(filePath(index));
            bool allSiblingsChecked = true;
            QModelIndex parent = this->parent(index);
            if (parent.isValid()) {
                for (int i = 0; i<rowCount(parent) && allSiblingsChecked == true; i++) {
                    if (!fullyCheckedFolders.contains(filePath(this->index(i,0,parent)))) {
                        allSiblingsChecked = false;
                    }
                }
                if (allSiblingsChecked && rowCount(parent)>1) {
                    //setData(parent,value,role);
                }
            }
        } else if (value == Qt::Unchecked) {
            int idx = fullyCheckedFolders.indexOf(filePath(index));
            if (idx>=0) {
                fullyCheckedFolders.removeAt(idx);
            } else { // a direct parent must be checked
                for (int i = 0; i<fullyCheckedFolders.size(); i++) {
                    QString path = fullyCheckedFolders.at(i);
                    if (filePath(index).startsWith(path+"/")) {
                        QModelIndex checkedParentIndex = this->index(path);
                        fullyCheckedFolders.removeAt(i);
                        checkAllChildrenBut(checkedParentIndex,filePath(index));
                        break;
                    }
                }
            }
        }
        qSort(fullyCheckedFolders.begin(), fullyCheckedFolders.end());
        dataChanged(QModelIndex(),QModelIndex());
        TRACE(qDebug()<<fullyCheckedFolders.commonParent());
        return TRUE;
    } else {
        return QFileSystemModel::setData(index, value, role);
    }
}

void CheckableFolderTree::checkAllChildrenBut(QModelIndex checkedParentIndex,QString path){
    if (hasChildren(checkedParentIndex)) {
        QModelIndex child;
        for (int i = 0; i<rowCount(checkedParentIndex); i++) {
            child = index(i,0,checkedParentIndex);
            if (!(filePath(child) == path)) {
                if (path.startsWith(filePath(child)+"/")) {
                    checkAllChildrenBut(child, path);
                } else {
                    fullyCheckedFolders.append(filePath(child));
                }
            }
        }
    }
}

bool CheckableFolderTree::hasChildren(const QModelIndex &parent) const {
    if (parent.column() > 0)
        return false;

    if (!parent.isValid()) // drives
        return true;

    QDir dir = QDir(filePath(parent));
    QFileInfoList list = dir.entryInfoList();

    for (int i = 0; i<list.size(); i++){
        if (list.at(i).isDir() && list.at(i).baseName().size()>0) {
            TRACE(qDebug()<<filePath(parent)<<" has at list one dir child: "<< list.at(i).baseName() << list.at(i).absolutePath() );
            return true;
        }
    }
    TRACE(qDebug()<<filePath(parent)<<" has no children" );
    return false;
}


void CheckableFolderTree::setCheckedFolders(FolderList list){
    fullyCheckedFolders.clear();
    fullyCheckedFolders.append(list);
    fullyCheckedFolders.organization = list.organization;
    dataChanged(QModelIndex(),QModelIndex());
}
