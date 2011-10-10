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

#include "folderlist.h"
#include <QDir>
#include <QFile>
#include "trace.h"

FolderList::FolderList()
    :QList<Folder>()
{
    organization = withoutCommonParent;
}


QString FolderList::path(int idx) {
    QString res = at(idx);
    if (res.endsWith("/"))
        res = res.left(res.size()-1);
    return res;
}

QString FolderList::relativePath(int idx) {
    QString result;
    if (organization == withCommonParent || organization == withoutCommonParent) {
        QString cParent = commonParent();
        QString sourcePath;
        if (cParent.size()>0) {
            sourcePath = QDir(cParent).relativeFilePath(path(idx));
        } else {
            sourcePath = path(idx);
        }
        if (organization == withCommonParent) {
            QString cParentBase = cParent.split("/").last();
            if (cParentBase.size()>0) {
                sourcePath = addPaths(cParentBase, sourcePath);
            } else {
                sourcePath = "ROOT/" + sourcePath;
            }
        }
        result = sourcePath;
    } else {
        QString currentName = baseName(idx);
        TRACE(qDebug()<<currentName);
        int idem = 0;
        for (int i = 0; i<idx; i++) {
            if (currentName == baseName(i))
                idem++;
        }
        result = currentName + (idem>0?QString("__%1__").arg(idem,4,10,QChar('0')):"");
    }
    return result.replace(":","_DRIVE");
}


QString FolderList::baseName(int idx) {
    return at(idx).split("/").last();
}

QString FolderList::relativePathDisplay(int idx) {
    QString relPath = relativePath(idx);
    if (relPath.size()>0) {
        return relPath + "  ( " + at(idx) + " )";
    } else {
        return "[" + QString(QFile::tr("content of %1")).arg(at(idx)) + "]";
    }
}

QString FolderList::commonParent(){
    if (size() <= 0) {
        return "";
    } else {
        QStringList splitList = at(0).split("/");
        int lastCommon = splitList.size()-1;
        for (int i = 1; i<size(); i++) {
            QStringList splitList2 = at(i).split("/");
            for (int j = 0; j< std::min(lastCommon+1,splitList2.size()); j++) {
                TRACE(qDebug()<<"i="<<i<<" j="<<j<<" splitList.at(j)="<<splitList.at(j)<<" splitList2.at(j)="<<splitList2.at(j));
                if (splitList.at(j) != splitList2.at(j)){
                    lastCommon = j-1;
                    break;
                }
            }
            if (lastCommon == -1)
                break;
        }
        QString result;
        for (int i = 0; i <= lastCommon; i++) {
            result = addPaths(result, splitList.at(i));
        }
        return result;
    }
}

QString FolderList::addPaths(QString prefix,QString suffix){
    if (prefix.endsWith  ("/") ||
        suffix.startsWith("/") ||
        prefix.size() == 0     ||
        suffix.size() == 0     ) {
        return prefix + suffix;
    } else {
        return prefix + "/" + suffix;
    }
}

