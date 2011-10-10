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

#ifndef BACKUPENGINE_H
#define BACKUPENGINE_H

#include <QThread>
#include "backupsettings.h"
#include <QFileSystemWatcher>
#include <QTime>
#include <QTimer>
#include <QTextBrowser>
#include <QSet>
#include <QMutex>

class BackupEngine : public QThread
{
    Q_OBJECT
public:
    explicit BackupEngine(BackupSettings *bs, QObject *parent = 0);
    void run();
    void processFile(int baseIdx, QFileInfo file, QString relDir);
    void processDir(int baseIdx, QString dirInfo, QString relDir, bool recursive);
    bool checkDir(QString baseDir, QString dirInfo, QString relDir);
    void cleanDir(DestFolderInfo *dest, QString filter = "");
    //void cleanDir2(QString dirName, bool recursive, QString baseSource, QString baseDest, DestFolderInfo *dest);
    bool removeDir(QString dirName, DestFolderInfo *dest);
    bool removeFile(QString fileName, DestFolderInfo *dest);
    bool removeArchive(QString fileName);
    QString getGarbageName(QString fileName, QString dest);
    int findSourceIdx(QString dirOrFile);
    QFileInfoList safeLS(QString dirName, bool *status) ;

    BackupSettings *externalSettings;

    QTimer * mainTimer;
    QDateTime nextLaunch; //informative

    QFileSystemWatcher *watcher;

    QTimer *processTimer;
    QSet<QString> toProcess;

    /* Stats */
    QTime timer;
    long nbFiles;
    long totalFiles;
    long nbFolders;
    long nbFilesCopied;
    long nbFilesRenamed;
    long nbFilesRemoved;
    long nbFilesRemovedFromBin;
    quint64 nbBytesTransfered;
    quint64 transferTimeMili;

    void initStats();
    void printStats();

    bool restartNeeded;
    bool successful;

    void    setStatus(QString status);
    QString getStatus();
    void    setDetailedStatus(QString detailedStatus);
    QString getDetailedStatus();

    /* logs */

    QTextBrowser * logView;
    QFile logFile;
    QTextStream * logStream;
    qint64 seekPos;
    bool logCleaned;

    /* old version directory size limitation */
    void cleanOldVersion(qint64 minSizeOnDisk);

signals:
    void sendLog(const QString & str);
    void sendInitLog();
    void sendStart(int msec);

public slots:
    void launch();
private slots:
    void backup();
    void initLog();
    void log(const QString & str);
    void setupTimer();
    void treatFile(QString file);
    void treatDir(QString dir);
    void processBuffer();
    void handleLink(QUrl);
    void copyCallBack(qint64 bytes);

private:

    BackupSettings settings;
    QString speed();
    QMutex  statusMutex;
    QString status;
    QMutex  detailedStatusMutex;
    QString detailedStatus;
    bool waiting;
    QMutex waitingMutex;

};

#endif // BACKUPENGINE_H
