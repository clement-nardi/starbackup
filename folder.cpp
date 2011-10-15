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
#include <linux/hdreg.h>
#include <fcntl.h>
#include <sys/ioctl.h>
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
    //for linux implementation: http://lists.trolltech.com/qt-interest/2004-11/msg01098.html

    struct hd_driveid id;

    int fd = open("/dev/hda", O_RDONLY|O_NONBLOCK);

    if (fd < 0)
    {
        perror("/dev/hda");
    }

    if(!ioctl(fd, HDIO_GET_IDENTITY, &id)) {
        const char* serial = (char*)id.serial_no;
        return serial;
    }
    return 0;
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
