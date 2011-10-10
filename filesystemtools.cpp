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

#include "filesystemtools.h"
#include <QDir>

FileSystemTools::FileSystemTools()
{
}

bool FileSystemTools::copyWithDirs(QString from, QString to){
    bool copied = QFile(from).copy(to);
    if (!copied) {
        QDir().mkpath(QFileInfo(to).absolutePath());
        copied = QFile(from).copy(to);
    }
    return copied;
}

bool FileSystemTools::linkWithDirs(QString linkName, QString linkTarget){
    bool linked = QFile(linkTarget).link(linkName);
    if (!linked) {
        QDir().mkpath(QFileInfo(linkName).absolutePath());
        linked = QFile(linkTarget).link(linkName);
    }
    return linked;
}

bool FileSystemTools::moveWithDirs(QString from, QString to){
    bool copied = QFile(from).rename(to);
    if (!copied) {
        QDir().mkpath(QFileInfo(to).absolutePath());
        copied = QFile(from).rename(to);
    }
    return copied;
}

QFileInfoList FileSystemTools::ls(QString dirName) {
    QDir dir = QDir(dirName);
    dir.setFilter(dir.filter()|QDir::NoDotAndDotDot|QDir::System);
    return dir.entryInfoList();
}

QString FileSystemTools::size2Str(qint64 nbBytes) {
    QString baseUnit = "B"; // Byte
    QStringList prefixes;
    prefixes << "" << "K" << "M" << "G" << "T" << "P" << "E" << "Z" << "Y";
    int prefixIdx = 0;
    qint64 number = nbBytes;
    while (number >= 1024*1000) {
        number /= 1024;
        prefixIdx++;
    }
    double value = (double) number;
    if (value >= 1000){
        value /= 1024;
        prefixIdx++;
    }
    return QString("%1 %2%3").arg(value,0,'g',3).arg(prefixes[prefixIdx]).arg(baseUnit);
}
