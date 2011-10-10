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
#include "destfolderinfo.h"
#include <QDateTime>
#include "filesystemtools.h"
#include <QStringList>
#include <QDir>
#include <QFile>

class BackupEngine;

FileKey::FileKey(uint _timestamp, qint64 _size){
    timestamp = _timestamp;
    size = _size;
}
FileKey::FileKey(QFileInfo fileInfo){
    timestamp = fileInfo.lastModified().toTime_t();
    size = fileInfo.size();
}
bool FileKey::operator< (const FileKey &fk) const{
    if (timestamp < fk.timestamp)
        return true;
    if (timestamp > fk.timestamp)
        return false;
    if (size < fk.size)
        return true;
    if (size > fk.size)
        return false;
    return false;
}

QString FileKey::toString(){
    return QString("(%1,%2)").arg(timestamp).arg(size);
}



DestFile::DestFile(QString str, bool _processed):
    QString(str) {
    processed = _processed;
}


DestFolderInfo::DestFolderInfo(QString str)
    :Folder(str){
    listing = new QMultiMap<FileKey,QString>();
    versionFiles = new QMultiMap<QDateTime,QString>();
    destFilesAndDirs = new QMap<QString,bool>();
}

QString DestFolderInfo::findFile(QFileInfo fileInfo){
    TRACE(qDebug()<<"Looking for "<< FileKey(fileInfo).toString());
    TRACE(qDebug()<<this->toString());
    FileKey key(fileInfo);
    QList<QString> list = listing->values(key);
    for (int i = 0; i<list.size(); i++)  {
        //check dest file's existence
        QString res = list.at(i);
        if (QFileInfo(res).exists()) {
            return res;
        } else {
            // check dest drive removal ?
            qWarning()<<"Destination drive unplugged? wrong dest files listing? "<<res;
            listing->remove(key,res);
        }
    }
    return "";
}

void DestFolderInfo::initListing(FolderList sourceDirs){
    TRACE(qDebug()<<"            --> init listing of "<< *this);

    if (listing->size() == 0) {
        versionDirSize = 0;
        mainDirSize = 0;
        TRACE(qDebug()<<"            --> really init listing of "<< *this);

        for (int sourceIdx = 0; sourceIdx<sourceDirs.size(); sourceIdx++) {
            listDir(FolderList::addPaths(*this,sourceDirs.relativePath(sourceIdx)));
        }
        listDir(versionFolder());
        TRACE(qDebug()<<"            --> listed "<<listing->size()<<" files");
    }
}

void DestFolderInfo::listDir(QString dir){
    QFileInfoList list = FileSystemTools::ls(dir);
    for (int i = 0; i<list.size(); i++){
        if ( list.at(i).isDir() &&
            !list.at(i).isSymLink()) { /* do not follow up on symlinks */
            if (!(list.at(i).absoluteFilePath()+"/").startsWith(versionFolder()+"/"))
                destFilesAndDirs->insert(list.at(i).absoluteFilePath(),FALSE);
            listDir(list.at(i).absoluteFilePath());
        } else {
            addFile(list.at(i),FALSE);
        }
    }
}

QString DestFolderInfo::toString() {
    QString res = "Dest Folder: " + *this;
    for (int i = 0; i<listing->keys().size(); i++){
        res = listing->keys().value(i).toString()+" \t--> "+listing->value(listing->keys().at(i));
        TRACE(qDebug()<<res);
    }
    return res;
}


void DestFolderInfo::addFile(QFileInfo fileInfo, bool processed, QString _filePath){
    QString filePath;
    if (_filePath.size()>0) {
        filePath = _filePath;
    } else {
        filePath = fileInfo.absoluteFilePath();
    }
    if ((fileInfo.absoluteFilePath()+"/").startsWith(versionFolder()+"/")){
        versionFiles->insert(extractDate(filePath),filePath);
        TRACE(qDebug()<< "before = " << FileSystemTools::size2Str(versionDirSize));
        versionDirSize += fileInfo.size();
        TRACE(qDebug()<<"adding to version list (size=" << FileSystemTools::size2Str(fileInfo.size()) << ") : "<< filePath);
        TRACE(qDebug()<< "after  = " << FileSystemTools::size2Str(versionDirSize));
    } else {
        listing->insert(FileKey(fileInfo),filePath);
        destFilesAndDirs->insert(filePath,processed);
        TRACE(qDebug()<< "before = " << FileSystemTools::size2Str(mainDirSize));
        mainDirSize += fileInfo.size();
        TRACE(qDebug()<<"adding to main list (size=" << FileSystemTools::size2Str(fileInfo.size()) << ") : "<< filePath);
        TRACE(qDebug()<< "after  = " << FileSystemTools::size2Str(mainDirSize));
    }
}

void DestFolderInfo::removeFile(QFileInfo fileInfo, QString _filePath){
    QString filePath;
    if (_filePath.size()>0) {
        filePath = _filePath;
    } else {
        filePath = fileInfo.absoluteFilePath();
    }
    if ((fileInfo.absoluteFilePath()+"/").startsWith(versionFolder()+"/")){
        versionFiles->remove(extractDate(filePath),filePath);
        TRACE(qDebug()<< "before = " << FileSystemTools::size2Str(versionDirSize));
        versionDirSize -= fileInfo.size();
        TRACE(qDebug()<<"remove from version list (size=" << FileSystemTools::size2Str(fileInfo.size()) << ") : "<< filePath);
        TRACE(qDebug()<< "after  = " << FileSystemTools::size2Str(versionDirSize));
    } else {
        listing->remove(FileKey(fileInfo),filePath);
        destFilesAndDirs->remove(filePath);
        TRACE(qDebug()<< "before = " << FileSystemTools::size2Str(mainDirSize));
        mainDirSize -= fileInfo.size();
        TRACE(qDebug()<<"remove from main list (size=" << FileSystemTools::size2Str(fileInfo.size()) << ") : "<< filePath);
        TRACE(qDebug()<< "after  = " << FileSystemTools::size2Str(mainDirSize));
    }
}

QDateTime DestFolderInfo::extractDate(QString _fileName) {

    QString fileName = _fileName.split("/").last();
    QStringList fileNameSplit = fileName.split(".");
    QString nameWithoutExtension;
    QString dateTimeStr;
    QDateTime dateTime;
    if (fileNameSplit.size()>2) {
        nameWithoutExtension = fileNameSplit.at(fileNameSplit.size()-2);
    } else {
        nameWithoutExtension = fileNameSplit.last();
    }
    TRACE(qDebug()<<fileName);
    dateTimeStr = nameWithoutExtension.right(19);
    TRACE(qDebug()<<dateTimeStr);
    dateTime = QDateTime::fromString(dateTimeStr,"yyyy-MM-dd_hh-mm-ss");
    TRACE(qDebug()<<dateTime);

    return dateTime;

}


QString DestFolderInfo::versionFolder() {
    return FolderList::addPaths(*this,VERSION_FOLDER);
}


QString DestFolderInfo::printFilesAndDirs() {
    QString res = *this + ":\n";
    QMapIterator<QString, bool> i(*(destFilesAndDirs));
    while (i.hasNext()) {
        i.next();
        res += (i.value()?"true  : ":"false : ") + i.key() + "\n";
    }
    return res;
}
