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

#ifndef DESTFOLDERINFO_H
#define DESTFOLDERINFO_H

#include <QString>
#include <QFileInfo>
#include <QMultiMap>
#include "folderlist.h"

#define VERSION_FOLDER "StarBackup - Archives"

class FileKey {
public:
    FileKey(){}
    FileKey(uint timestamp, qint64 size);
    FileKey(QFileInfo fileInfo);

    bool operator< (const FileKey &fk) const;
    QString toString();

    uint timestamp;
    qint64 size;
};

class DestFile : public QString {
public:
    DestFile(QString str, bool processed);
    bool processed;
};

class DestFolderInfo : public Folder {
public:
    DestFolderInfo(QString str) ;

    QString findFile(QFileInfo fileInfo);
    void addFile(QFileInfo fileInfo, bool processed, QString filePath = "");
    void removeFile(QFileInfo fileInfo, QString filePath = "");
    QString toString();

    void initListing(FolderList sourceDirs);
    void listDir(QString dir);
    static QDateTime extractDate(QString fileName);
    QString versionFolder();

    QMultiMap<FileKey,QString> *listing; /* used for rename/move detection */
    QMap<QString,bool> *destFilesAndDirs; /* used for clean-up */
    QMultiMap<QDateTime,QString> *versionFiles;

    QString printFilesAndDirs();

    qint64 versionDirSize;
    qint64 mainDirSize;

};



#endif // DESTFOLDERINFO_H
