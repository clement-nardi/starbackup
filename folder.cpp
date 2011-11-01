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

#include "folder.h"
#include "trace.h"
#include <QDir>
#include <QMutex>

Folder::Folder() :
    QString()
{
}
Folder::Folder(QString str) :
    QString(str)
{
}

QMutex* Folder::currentFolderMutex(){
    static QMutex* _currentFolderMutex = new QMutex();
    return _currentFolderMutex;
}

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>

#else // linux stuff
#include <sys/stat.h>
#include <mntent.h>
#include <cstdlib>
#include <blkid/blkid.h>
#endif // _WIN32


QString Folder::getDriveSerial() const {

#ifdef _WIN32
    // adapted from http://www.developpez.net/forums/d523835/c-cpp/cpp/utilisation-getvolumeinformation/

    TCHAR volName[MAX_PATH+1];
    DWORD volSerial;
    DWORD nameLen;
    DWORD volFlags;
    TCHAR volFS[MAX_PATH+1];

    Folder::currentFolderMutex()->lock();
    QString sCurDir = QDir::current().absolutePath();
    QDir::setCurrent( *this );

    GetVolumeInformation(NULL, volName, MAX_PATH+1, &volSerial, &nameLen, &volFlags, volFS, MAX_PATH+1);

    QDir::setCurrent( sCurDir );
    Folder::currentFolderMutex()->unlock();

    TRACE(qDebug() << "Serial=" << volSerial
          << " Name="  << QString((QChar*)volName)
          << " FS="    << QString((QChar*)volFS)
          << " for "   << *this);
    return QString("%1 %2 %3").arg(volSerial).arg(QString((QChar*)volName)).arg(QString((QChar*)volFS)) ;

#else
    /* We'll return the UUID ofthe partition holding the file */

    const char* curDir = QDir::current().absolutePath().toAscii().constData();
    char* partitionDevice = NULL;
    /* First, we need to find the partition */
    struct stat file_st, mount_st;
    if (stat(curDir, &file_st) != 0) return "";     /* Woups. */

    struct mntent mnt;
    char buf[512];
    char const *table = _PATH_MOUNTED;
    FILE *fp;

    fp = setmntent (table, "r");
    if (fp == NULL)
        return "";

    while (getmntent_r(fp,&mnt,buf,sizeof(buf))){
        if(stat(mnt.mnt_dir,&mount_st)==0&&(file_st.st_dev==mount_st.st_dev)){
            partitionDevice = canonicalize_file_name(mnt.mnt_fsname);
            break;
        }
    }
    endmntent (fp);
    if(partitionDevice==NULL)
        return "";

    /* OK; we have the partition device. Go for the UUID */
    QString str;
    char *value;
    if ((value = blkid_get_tag_value(NULL, "UUID", partitionDevice))) {
        str = QString(value);
        std::free(value);
    }
    TRACE(qDebug() << "UUID " << str << " for " << *this);
    return str;

#endif

}

void Folder::loadHardwareID() {
    hardwareID = getDriveSerial();
}

bool Folder::checkHardwareID() const {
    bool sameID = (hardwareID == getDriveSerial()) ||
            (hardwareID == "");
    return sameID;
}


void Folder::setPlugged(bool value) {
    isPlugged = value;
}
