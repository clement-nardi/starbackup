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

#ifndef FILESYSTEMTOOLS_H
#define FILESYSTEMTOOLS_H
#include <QString>
#include <QFileInfoList>

class FileSystemTools
{
public:
    FileSystemTools();
    static QFileInfoList ls(QString dirName);

    static bool copyWithDirs(QString from, QString to);
    static bool moveWithDirs(QString from, QString to);
    static bool linkWithDirs(QString linkName, QString linkTarget);
    static QString size2Str(qint64 nbBytes);
};

#endif // FILESYSTEMTOOLS_H
