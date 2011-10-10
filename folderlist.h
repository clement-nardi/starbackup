/* This file is part of StarBackup
 * StarBackup is a Free Real-Time Backup and Archiving Software
 *
 * Copyright (C) 2011  Cl�ment Nardi
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

#ifndef FOLDERLIST_H
#define FOLDERLIST_H

#include <QStringList>
#include "folder.h"

enum organizeT {withoutCommonParent,withCommonParent,flat};

class FolderList : public QList<Folder>
{
public:
    explicit FolderList();

    QString commonParent();
    QString relativePath(int idx);
    QString relativePathDisplay(int idx);
    QString baseName(int idx);
    QString path(int idx);
    static QString addPaths(QString prefix,QString suffix);

    int organization;

};

#endif // FOLDERLIST_H
