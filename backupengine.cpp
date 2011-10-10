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

//#define DEBUG

#include "backupengine.h"
#include "trace.h"
#include <QDateTime>
#include "filesystemtools.h"
#include <math.h>


BackupEngine::BackupEngine(BackupSettings *bs, QObject *parent) :
        QThread(parent)
{
    externalSettings = bs;

    watcher = NULL;
    processTimer = NULL;
    waiting = false;
    restartNeeded = false;
    totalFiles = 0;
    setStatus("");
    setDetailedStatus("");
    successful = true;
    logCleaned = false;
    logView = new QTextBrowser();
    logView->document()->setMaximumBlockCount(300);
    connect(logView,SIGNAL(anchorClicked(QUrl)),this,SLOT(handleLink(QUrl)));
    logView->setOpenLinks(false);
    logFile.setFileName(bs->file->fileName() + ".log");
    logFile.open(QIODevice::Append | QIODevice::Text);
    seekPos = logFile.size();
    logStream = new QTextStream(&logFile);
    //logView->setSource(logFile.fileName());
    initLog();
    connect(this,SIGNAL(sendLog(QString)),this,SLOT(log(QString)),Qt::QueuedConnection);
    connect(this,SIGNAL(sendInitLog()),this,SLOT(initLog()));
    start();
}

void BackupEngine::log(const QString & _str){
    QString str = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss : ") + _str;
    //logView->textCursor().setPosition();
    logView->append(str);
    logView->ensureCursorVisible();
    *logStream << str + "\n";
}

void BackupEngine::initLog(){
    logView->clear();
    logView->append(QString("<a href=\"previous\">"+tr("Show previous log entries")+"</a><br/>\n"));
}


void BackupEngine::handleLink(QUrl url){
    logFile.flush();
    logFile.close();
    logFile.open(QIODevice::ReadOnly);
    logView->setCurrentCharFormat(QTextCharFormat());
    seekPos = seekPos-10000;
    if (seekPos>0) {
        logFile.seek(seekPos);
    }
    initLog();
    logFile.readLine();
    logView->insertPlainText(QString(logFile.readAll()));
    logFile.close();
    logFile.open(QIODevice::Append | QIODevice::Text);
}

void BackupEngine::setupTimer() {
    TRACE(qDebug()<<this->currentThreadId());

    settings = *externalSettings;
    if (settings.realTime && successful) {
        TRACE(qDebug()<<"realTime");
        mainTimer->stop();
        setStatus(tr("Real-Time backup"));
        backup();
    } else if (settings.noSchedule){
        TRACE(qDebug()<<"noSchedule");
        mainTimer->stop();
        setStatus(tr("Not scheduled"));
        return;
    } else { // scheduled or missed real-time
        TRACE(qDebug()<<"scheduled");
        qint64 delay = 0;
        bool missed = settings.realTime;
        QDateTime current = QDateTime::currentDateTime();
        QDateTime next = QDateTime(current).addMSecs(settings.catchUpValue * 60 * 1000); // in case of missed real-time
        if (!settings.realTime) {
            if (settings.dayly) {
                TRACE(qDebug()<<"dayly");
                next = QDateTime(settings.lastSuccess.date(),settings.daylyTime);
                if (next <= settings.lastSuccess)
                    next = next.addDays(1);
                while (next <= current) {
                    missed = true;
                    next = next.addDays(1);
                }
            } else if (settings.weekly) {
                TRACE(qDebug()<<"weekly");
                next = QDateTime(settings.lastSuccess.date(),settings.weeklyTime);
                while (next <= settings.lastSuccess ||
                       next.date().dayOfWeek() != settings.weeklyDay) {
                    next = next.addDays(1);
                }
                while (next <= current) {
                    missed = true;
                    next = next.addDays(7);
                }
            } else if (settings.every) {
                TRACE(qDebug()<<"every "<<settings.everyNb);
                next = QDateTime(settings.lastSuccess);
                switch (settings.everyUnit) {
                case 0: // minutes
                    TRACE(qDebug()<<"minutes");
                    next = next.addSecs(60*settings.everyNb);
                    break;
                case 1: // hours
                    TRACE(qDebug()<<"hours");
                    next = next.addSecs(60*60*settings.everyNb);
                    break;
                case 2: // days
                    TRACE(qDebug()<<"days");
                    next = next.addDays(settings.everyNb);
                    break;
                case 3: // months
                    TRACE(qDebug()<<"months");
                    next = next.addMonths(settings.everyNb);
                    break;
                }
                if (next <= current) {
                    missed = true;
                    next = QDateTime(current).addMSecs(settings.catchUpValue * 60 * 1000);
                }
            }
        }

        TRACE(qDebug()<<"lastSuccess   = " << settings.lastSuccess);
        TRACE(qDebug()<<"lastTentative = " << settings.lastTentative);
        TRACE(qDebug()<<"current = " << current);
        TRACE(qDebug()<<"next    = " << next);
        TRACE(qDebug()<<"missed  = " << missed);

        if (missed && settings.catchUpMissed) {
            QDateTime nextTry = QDateTime(settings.lastTentative).addMSecs(settings.catchUpValue * 60 * 1000);
            TRACE(qDebug()<<"nextTry = " << nextTry);
            if (nextTry<=current) {
                delay = 0;
            } else if (next<nextTry) {
                delay = current.msecsTo(next);
            } else {
                delay = current.msecsTo(nextTry);
            }
        } else {
            delay = current.msecsTo(next);
        }

        TRACE(qDebug()<<"delay = "<<delay);
        mainTimer->setInterval(delay);
        //mainTimer->setInterval(0); /* for testing purpose */
        mainTimer->setSingleShot(true);
        TRACE(qDebug()<<"starting from setupTimer");
        mainTimer->start();
        nextLaunch = current.addMSecs(delay);
        setStatus("");
        setDetailedStatus("");

        waitingMutex.lock();
        waiting = true;
        waitingMutex.unlock();
    }
}

void BackupEngine::launch() {
    waitingMutex.lock();
    if (waiting) {
        waiting = false;
        sendStart(0);
    }
    waitingMutex.unlock();
}

void BackupEngine::run() {
    mainTimer = new QTimer();
    TRACE(qDebug()<<mainTimer->thread()->currentThreadId());
    //mainTimer->moveToThread(this);
    TRACE(qDebug()<<this->currentThreadId());
    mainTimer->setSingleShot(true);
    connect(mainTimer,SIGNAL(timeout()),this,SLOT(backup()),Qt::DirectConnection);
    connect(this,SIGNAL(sendStart(int)),mainTimer,SLOT(start(int)),Qt::QueuedConnection);

    setupTimer();
    exec();
}

void BackupEngine::backup() {
    TRACE(qDebug()<<this->currentThreadId() << "backup");
    bool atLestOneSource = false;
    bool atLestOneDest   = false;

    settings = *externalSettings;
    sendInitLog();
    sendLog("");
    sendLog(QString(tr("Launching backup \"%1\"")).arg(settings.title));
    setStatus(tr("initializing..."));
    initStats();
    successful = true;

    QDir::drives(); /* hoping it can bring drives online... */

    /* check source and destination folders plugged-in and serials */
    for (int sourceIdx = 0; sourceIdx<settings.sourceDirs.size(); sourceIdx++) {
        /* retrieve directory content before testing its existence
         * hoping it can bring a network drive online */
        FileSystemTools::ls(settings.sourceDirs.at(sourceIdx));
        if (!QDir(settings.sourceDirs.at(sourceIdx)).exists()) {
            sendLog(QString(tr("ERROR: %1 is not plugged-in => backup of this directory will be skipped"))
                    .arg(settings.sourceDirs.at(sourceIdx)));
            successful = false;
            settings.sourceDirs[sourceIdx].setPlugged(false);
        } else {
            if (!settings.sourceDirs.at(sourceIdx).checkHardwareID()) {
                sendLog(QString(tr("ERROR: The drive signature of %1 changed => backup of this directory will be skipped"))
                        .arg(settings.sourceDirs.at(sourceIdx)));
                successful = false;
                settings.sourceDirs[sourceIdx].setPlugged(false);
            } else {
                settings.sourceDirs[sourceIdx].setPlugged(true);
                atLestOneSource = true;
            }
        }
    }

    for (int destIdx = 0; destIdx<settings.destDirs.size(); destIdx++) {
        QDir dir(settings.destDirs.at(destIdx));
        dir.mkpath(settings.destDirs.at(destIdx)); /* create dest dir if it does not exist */
        /* retrieve directory content before testing its existence
         * hoping it can bring a network drive online */
        FileSystemTools::ls(settings.destDirs.at(destIdx));
        if (!dir.exists()) {
            sendLog(QString(tr("ERROR: %1 is not plugged-in => backup to this directory will be skipped"))
                    .arg(settings.destDirs.at(destIdx)));
            successful = false;
            settings.destDirs[destIdx].setPlugged(false);
        } else {
            if (!settings.destDirs.at(destIdx).checkHardwareID()) {
                sendLog(QString(tr("ERROR: The drive signature of %1 changed => backup to this directory will be skipped"))
                        .arg(settings.destDirs.at(destIdx)));
                successful = false;
                settings.destDirs[destIdx].setPlugged(false);
            } else {
                settings.destDirs[destIdx].setPlugged(true);
                atLestOneDest = true;
            }
        }
    }

    if (watcher != NULL) {
        watcher->deleteLater();
        delete watcher;
        watcher = NULL;
    }
    if (processTimer != NULL) {
        processTimer->deleteLater();
        delete processTimer;
        processTimer = NULL;
    }
    if (settings.realTime) {
        watcher = new QFileSystemWatcher();
        watcher->moveToThread(this);
        connect(watcher,SIGNAL(fileChanged(QString)),this,SLOT(treatFile(QString)),Qt::DirectConnection);
        connect(watcher,SIGNAL(directoryChanged(QString)),this,SLOT(treatDir(QString)),Qt::DirectConnection);
        processTimer = new QTimer();
        processTimer->moveToThread(this);
        processTimer->setInterval(1000);
        processTimer->setSingleShot(true);
        connect(processTimer,SIGNAL(timeout()),this,SLOT(processBuffer()),Qt::DirectConnection);
    }

    if (atLestOneSource && atLestOneDest) {

        for (int destIdx = 0; destIdx<settings.destDirs.size(); destIdx++) {
            //status = QString("initializing destination directory %1").arg(settings.destDirs[destIdx]);
            if (settings.destDirs.at(destIdx).isPlugged) {
                settings.destDirs[destIdx].initListing(settings.sourceDirs);
                if (totalFiles == 0) {
                    if (settings.destDirs.size() > 0)
                        totalFiles = settings.destDirs[0].listing->size();
                }
            }
        }
        cleanOldVersion(0);
        setStatus(tr("running..."));
        for (int sourceIdx = 0; sourceIdx<settings.sourceDirs.size(); sourceIdx++) {
            processDir(sourceIdx,
                       settings.sourceDirs.path(sourceIdx),
                       settings.sourceDirs.relativePath(sourceIdx),
                       true);
        }

        if (settings.destDirs.size() > 0)
            TRACE(qDebug()<<settings.destDirs[0].printFilesAndDirs());

        /* processDir must be called before cleanDir so that renamed/moved files detection can work properly */
        for (int destIdx = 0; destIdx<settings.destDirs.size(); destIdx++) {
            if (settings.destDirs.at(destIdx).isPlugged) {
                cleanDir(&(settings.destDirs[destIdx]));
            }
        }


        if (settings.destDirs.size() > 0)
            TRACE(qDebug()<<settings.destDirs[0].printFilesAndDirs());

        cleanOldVersion(0);
        printStats();
    } else {
        successful = false;
    }

    externalSettings->lastTentative = QDateTime::currentDateTime();
    if (successful) {
        externalSettings->lastSuccess = QDateTime(externalSettings->lastTentative);
    }
    externalSettings->saveToXML();

    setDetailedStatus("");
    if (settings.realTime) {
        if (successful) {
            setStatus(tr("waiting for changes..."));
            TRACE(qDebug()<<"quit");
        } else {
            sendLog(QString(tr("ERROR: => real-time disabled, will try again in %1 minutes.")).arg(settings.catchUpValue));
            setupTimer();
        }
    } else {
        setupTimer();
    }

    waitingMutex.lock();
    waiting = true;
    waitingMutex.unlock();
    //status = "finished " + QDateTime::currentDateTime().toString();
}

void BackupEngine::processBuffer(){
    logCleaned = false;
    sendLog(tr("Treating notifications."));
    //toProcess.sort();
    TRACE(qDebug()<<toProcess);
/*
    TRACE(qDebug()<<watcher->directories());
    for (int i = toProcess.size()-1; i>=0; i--) {
        watcher->removePath(toProcess.at(i));
    }
    watcher->removePaths(toProcess);
    QStringList toWatch(watcher->directories());
    toWatch.append(watcher->files());
    watcher->deleteLater();
    for (int i = toProcess.size()-1; i>=0; i--) {
        toWatch.removeAll(toProcess.at(i));
    }
    watcher = new QReliableFileWatch(toWatch);*/
    if (watcher!=NULL) {
        TRACE(qDebug()<<watcher->directories());
        TRACE(qDebug()<<watcher->files());
    }

    initStats();

    QSetIterator<QString> i(toProcess);
    while (i.hasNext()) {
        QString dirOrFile = i.next();
        setDetailedStatus(QString(tr("Processing %1")).arg(dirOrFile));
        QFileInfo dirOrFileInfo = QFileInfo(dirOrFile);
        if (dirOrFileInfo.exists()) {
            TRACE(qDebug()<<"dir "<<dirOrFile<<" was modified");
            int sourceIdx = findSourceIdx(dirOrFile);
            if ( dirOrFileInfo.isDir() &&
                !dirOrFileInfo.isSymLink()) {
                processDir(sourceIdx,
                           dirOrFile,
                           settings.sourceDirs.relativePath(sourceIdx),
                           false);
            } else {
                processFile(sourceIdx,
                            dirOrFileInfo,
                            settings.sourceDirs.relativePath(sourceIdx));
            }
        }
    }

    i = QSetIterator<QString>(toProcess);
    while (i.hasNext()) {
        QString dirOrFile = i.next();
        if ( QFileInfo(dirOrFile).isDir() &&
            !QFileInfo(dirOrFile).isSymLink()) { /* if it is a Directory, maybe some files were removed */
            for (int sourceIdx = 0; sourceIdx<settings.sourceDirs.size(); sourceIdx++) { /* find the source directory from the notification */
                if (settings.sourceDirs.at(sourceIdx).isPlugged &&
                    dirOrFile.startsWith(FolderList::addPaths(settings.sourceDirs.path(sourceIdx),""))) {
                    for (int destIdx = 0; destIdx<settings.destDirs.size(); destIdx++) { /* clean each destination */
                        QString baseSource = settings.sourceDirs.path(sourceIdx);
                        QString baseDest = FolderList::addPaths(settings.destDirs.at(destIdx),settings.sourceDirs.relativePath(sourceIdx));
                        /* but clean only in the processed directory */

                        TRACE(qDebug()<<settings.destDirs[destIdx].printFilesAndDirs());
                        TRACE(qDebug()<<"filter = " << FolderList::addPaths(baseDest,QDir(baseSource).relativeFilePath(dirOrFile)));
                        cleanDir(&(settings.destDirs[destIdx]),
                                 FolderList::addPaths(baseDest,QDir(baseSource).relativeFilePath(dirOrFile)));

                        TRACE(qDebug()<<settings.destDirs[destIdx].printFilesAndDirs());
                    }
                }
            }
        }
    }

    toProcess.clear();
    printStats();
    setStatus(tr("waiting for changes..."));
    setDetailedStatus("");
}

void BackupEngine::cleanDir(DestFolderInfo *dest, QString filter) {
    QMapIterator<QString, bool> i(*(dest->destFilesAndDirs));
    while (i.hasNext()) {
        i.next();
        if (filter == "" ||
                (QString(i.key()).startsWith(filter + "/") &&
                 !((i.key().mid( (filter.size()) + 1 )).contains("/")))) { /* only the files/dirs in the specified folder */

            TRACE(qDebug()<<i.key().mid(filter.size()+1));
            TRACE(qDebug()<<"filter passed: "<<i.key());
            if (i.value() == false) { /* file/dir was not processed */
                /* remove the file or dir */
                QString relPath = QDir(*dest).relativeFilePath(i.key());
                TRACE(qDebug()<< "not processed: " << i.key());
                TRACE(qDebug()<< "not processed: relpath=" << relPath);
                for (int sourceIdx = 0; sourceIdx<settings.sourceDirs.size(); sourceIdx++) {
                    QString SourceRelDir = settings.sourceDirs.relativePath(sourceIdx);
                    TRACE(qDebug()<< "sourceRelDir=" << SourceRelDir);

                    if (settings.sourceDirs.at(sourceIdx).isPlugged &&
                        (relPath.startsWith(SourceRelDir+"/") ||
                         SourceRelDir.size() == 0)){ /* in case content is directly stored on dest */
                        if (QDir(settings.sourceDirs.path(sourceIdx)).exists()) {
                            setDetailedStatus(QString(tr("Cleaning %1").arg(i.key())));
                            if ( QFileInfo(i.key()).isDir()&&
                                !QFileInfo(i.key()).isSymLink()) {
                                removeDir(i.key(), dest);
                            } else {
                                removeFile(i.key(), dest);
                            }
                        } else {
                            sendLog(QString(tr("ERROR: Source folder removed or device unplugged: %1")).arg(settings.sourceDirs.path(sourceIdx)));
                            successful = false;
                            settings.sourceDirs[sourceIdx].setPlugged(false);
                        }

                        break;
                    }
                }
            } else {
                dest->destFilesAndDirs->insert(i.key(),false);
            }
        }
    }
}
/*
void BackupEngine::cleanDir2(QString dirName, bool recursive, QString baseSource, QString baseDest, DestFolderInfo *dest) {
    QString dirSource = FolderList::addPaths(baseSource,QDir(baseDest).relativeFilePath(dirName));
    QFileInfoList list = FileSystemTools::ls(dirName);
    TRACE(qDebug()<<"clean "<<dirName);
    detailedStatus = QString(tr("Cleaning %1")).arg(dirName);
    TRACE(qDebug()<<"source "<<dirSource);
    for (int entryIdx = 0; entryIdx<list.size(); entryIdx++) {
        QString sourcePath = FolderList::addPaths(dirSource,list.at(entryIdx).fileName());
        TRACE(qDebug()<<sourcePath);
        QFileInfo sourceInfo = QFileInfo(sourcePath);
        if (!sourceInfo.exists()){
            if (QDir(baseSource).exists()) {
                TRACE(qDebug()<<"   --> remove");
                detailedStatus = QString(tr("Cleaning %1")).arg(list.at(entryIdx).absoluteFilePath());
                if (list.at(entryIdx).isDir()) {
                    removeDir(list.at(entryIdx).absoluteFilePath(), dest);
                } else {
                    removeFile(list.at(entryIdx).absoluteFilePath(), dest);
                }
            } else {
                sendLog(QString(tr("Source folder removed or device unplugged: %1")).arg(baseSource));
                successful = false;
            }
        }else if (recursive && list.at(entryIdx).isDir()) {
            cleanDir2(list.at(entryIdx).absoluteFilePath(), recursive, baseSource, baseDest, dest);
        }
    }
}
*/
bool BackupEngine::removeDir(QString dirName, DestFolderInfo *dest){
    TRACE(qDebug()<<"removedir( "<<dirName<<" )");
    QFileInfoList list = FileSystemTools::ls(dirName);
    for (int entryIdx = 0; entryIdx<list.size(); entryIdx++) {
        if ( list.at(entryIdx).isDir() &&
            !list.at(entryIdx).isSymLink()) {
            removeDir(list.at(entryIdx).absoluteFilePath(), dest);
        } else {
            removeFile(list.at(entryIdx).absoluteFilePath(), dest);
        }
    }
    if (QDir().rmdir(dirName)) {
        TRACE(qDebug()<<"dir  "<<dirName<<" correctly removed");
        sendLog(QString(tr("Removed directory: %1")).arg(dirName));
        return true;
    } else {
        sendLog(QString(tr("Problem while removing directory: %1")).arg(dirName));
        return false;
    }
}

bool BackupEngine::removeFile(QString fileName, DestFolderInfo *dest){
    dest->removeFile(QFileInfo(fileName));
    if (settings.versioning) {
        QString destPath = getGarbageName(fileName,*dest);
        TRACE(qDebug()<<"dest file "<<destPath);
        if (FileSystemTools::moveWithDirs(fileName, destPath)) {
                TRACE(qDebug()<<"file "<<fileName<<" correctly archived.");
            sendLog(QString(tr("Archived: %1")).arg(fileName));
            nbFilesRemoved++;
            dest->addFile(destPath,true);
        } else {
            /* maybe this is a folder ending with .lnk */
            if (!removeDir(fileName, dest)) {
                sendLog(QString(tr("Problem while archiving: %1 to %2")).arg(fileName).arg(destPath));
                return false;
            }
        }
    } else {
        if (QFile(fileName).remove()) {
            TRACE(qDebug()<<"file "<<fileName<<" correctly removed");
            sendLog(QString(tr("Removed file: %1")).arg(fileName));
            nbFilesRemoved++;
        } else {
            /* maybe this is a folder ending with .lnk */
            if (!removeDir(fileName, dest)) {
                sendLog(QString(tr("Problem while removing file: %1")).arg(fileName));
                return false;
            }
        }
    }
    return true;
}

QString BackupEngine::getGarbageName(QString filePath, QString dest) {
    QString fileName = filePath.split("/").last();
    QStringList fileNameSplit = fileName.split(".");
    QString fileExtension = "";
    if (fileNameSplit.size()>1) {
        fileExtension = "." + fileNameSplit.last();
        fileNameSplit.removeLast();
        fileName = fileNameSplit.join(".");
    }

    QDir fileFolder = QDir(filePath);
    fileFolder.cdUp();
    QString destPath = FolderList::addPaths(dest,
                       FolderList::addPaths(VERSION_FOLDER,
                       FolderList::addPaths(QDir(dest).relativeFilePath(fileFolder.absolutePath()),
                                            fileName +
                                            QDateTime::currentDateTime().toString(".yyyy-MM-dd_hh-mm-ss") +
                                            fileExtension)));
    return destPath;
}

#define MAX_TRIES 3

QFileInfoList BackupEngine::safeLS(QString dirName, bool *pb) {

    QDir dir(dirName.endsWith("/")?dirName:dirName+"/");
    QFileInfoList list;
    int tryCount = 0;
    bool problem;
    bool found;
    bool isRoot;
    int idx;
    do {
        if (tryCount>0) {
            sendLog("          => " + tr("trying again"));
        }
        tryCount++;
        dir.setFilter(QDir::System|QDir::AllEntries);
        list = dir.entryInfoList();
        problem = false;
        found = false;
        isRoot = dir.isRoot();
        for (idx = 0; idx<list.size(); idx++) {
            if (list.at(idx).fileName() == ".") {
                found = true;
                break;
            }
        }
        if (found) {
            list.removeAt(idx);
        } else if (!isRoot) {
            problem = true;
            TRACE(qDebug()<<"WARNING: . not found in " + dirName);
        }
        found = false;
        for (idx = 0; idx<list.size(); idx++) {
            if (list.at(idx).fileName() == "..") {
                found = true;
                break;
            }
        }
        if (found) {
            list.removeAt(idx);
        } else if (!isRoot) {
            problem = true;
            TRACE(qDebug()<<"WARNING: .. not found in " + dirName);
        }
        if (problem) {
            sendLog(QString(tr("WARNING: Problem while retrieving content of %1:")).arg(dirName));
            for (idx = 0; idx<list.size(); idx++) {
                sendLog("           "+list.at(idx).fileName());
            }
        }

    } while (problem && tryCount < MAX_TRIES) ;
    *pb = problem;
    return list;
}

void BackupEngine::processDir(int baseIdx, QString dirName, QString relDir, bool recursive) {
    if (settings.sourceDirs.at(baseIdx).isPlugged) {
        QString baseDir = settings.sourceDirs.path(baseIdx);
        nbFolders++;
        setDetailedStatus(QString(tr("Processing %1")).arg(dirName));
        TRACE(qDebug()<<dirName);
        bool pb;
        QFileInfoList list = safeLS(dirName, &pb);
        if (!pb) {
            for (int entryIdx = 0; entryIdx<list.size(); entryIdx++) {
                if ( list.at(entryIdx).isDir() &&
                    !list.at(entryIdx).isSymLink()) { /* do not follow up on symlinks */
                    bool dirCreated = checkDir(baseDir, list.at(entryIdx).absoluteFilePath(),relDir);
                    if (recursive || dirCreated) {
                        processDir(baseIdx, list.at(entryIdx).absoluteFilePath(),relDir,recursive || dirCreated);
                    }
                } else {
                    processFile(baseIdx, list.at(entryIdx),relDir);
                }
            }
            if (list.size()==0){
                //create directory on dest
                checkDir(baseDir, dirName,relDir);
            }
            if (watcher != NULL)
                watcher->addPath(dirName);
        } else {
            sendLog(QString(tr("ERROR: aborting backup of %1")).arg(dirName));
            successful = false;
            //settings.sourceDirs[baseIdx].setPlugged(false);
            /* mark all the subfiles as treated so that they are not later removed from the backup */

            QString baseDir = settings.sourceDirs.path(baseIdx);
            QString destPath = FolderList::addPaths(relDir,QDir(baseDir).relativeFilePath(dirName));

            for (int destIdx = 0; destIdx<settings.destDirs.size(); destIdx++) {
                QString destdirPath = FolderList::addPaths(settings.destDirs.at(destIdx), destPath);
                if (!destdirPath.endsWith("/"))
                    destdirPath.append("/");
                QMapIterator<QString, bool> i(*(settings.destDirs[destIdx].destFilesAndDirs));
                while (i.hasNext()) {
                    i.next();
                    if (QString(i.key()).startsWith(destdirPath)) {
                        settings.destDirs[destIdx].destFilesAndDirs->insert(i.key(),true);
                    }
                }
            }
        }
    }
}

/* checks that dirName exists in each destination directory
 * create it if necessary */
bool BackupEngine::checkDir(QString baseDir, QString dirName, QString relDir) {
    bool dirCreated = false;
    QString destPath = FolderList::addPaths(relDir,QDir(baseDir).relativeFilePath(dirName));
    for (int i = 0; i<settings.destDirs.size(); i++) {
        if (settings.destDirs.at(i).isPlugged) {
            QString destDirPath = FolderList::addPaths(settings.destDirs.at(i), destPath);
            if (!QFileInfo(destDirPath).exists()) {
                QDir().mkpath(destDirPath);
                dirCreated = true;
            }
            settings.destDirs[i].destFilesAndDirs->insert(destDirPath,true);
        }
    }
    return dirCreated;
}

void BackupEngine::processFile(int baseIdx, QFileInfo file, QString relDir) {
    QString baseDir = settings.sourceDirs.path(baseIdx);
    setDetailedStatus(QString(tr("Processing %1")).arg(file.absoluteFilePath()));
    if (file.exists() && settings.sourceDirs.at(baseIdx).isPlugged) {
        bool alreadyLogged = false;
        //status = "treating " + file.absoluteFilePath();
        QString destPath = FolderList::addPaths(relDir,QDir(baseDir).relativeFilePath(file.absoluteFilePath()));
        nbFiles++;
        if (nbFiles>totalFiles) {
            totalFiles = nbFiles *2;
        }
        TRACE(qDebug()<<"  (file)--> "<<file.absoluteFilePath());
        for (int i = 0; i<settings.destDirs.size(); i++) {
            if (settings.destDirs.at(i).isPlugged) { /* dest dir exists */
                QString fileStatus = "";
                QString destFilePath = FolderList::addPaths(settings.destDirs.at(i), destPath);
                TRACE(qDebug()<<"        --> "<<destFilePath);
                QFileInfo destInfo = QFileInfo(destFilePath);
                bool copy = false;
                if (destInfo.exists()) {
                    if (abs(destInfo.lastModified().toTime_t() - file.lastModified().toTime_t())<=2 && /* http://bugreports.qt.nokia.com/browse/QTBUG-12006 */
                            destInfo.size()         == file.size()           ) {
                        TRACE(qDebug()<<"            --> "<<"already there!");
                        settings.destDirs[i].destFilesAndDirs->insert(destFilePath,true); /* pb with this ???? */
                    } else {
                        TRACE(qDebug()<<"            --> "<<"already there but different. " <<
                              ((abs(destInfo.lastModified().toTime_t() - file.lastModified().toTime_t())<=2)?"lastmodified is different":"") <<
                              ((destInfo.size() != file.size())?"size is different":"") );
                        TRACE(qDebug()<<file.lastModified().toTime_t() );
                        TRACE(qDebug()<<destInfo.lastModified().toTime_t() );
                        // move to old versions folder
                        removeFile(destFilePath, &(settings.destDirs[i]));
                        fileStatus += tr("modified");
                        copy = true;
                    }
                } else {
                    TRACE(qDebug()<<"            --> not there...");
                    copy = true;
                    // check if moved or renamed
                    QString oldFileName = "";// = settings.destDirs[i].findFile(file);
                    bool renamed = false;
                    FileKey key(file);
                    QList<QString> nameList = settings.destDirs[i].listing->values(key);
                    for (int nameIdx = 0; nameIdx<nameList.size(); nameIdx++)  {
                        //check dest file's existence
                        QString res = nameList.at(nameIdx);
                        if (QFileInfo(res).exists()) {
                            oldFileName = res;
                            TRACE(qDebug()<<"Found on dest: "<<oldFileName);
                            QString oldSourceName = FolderList::addPaths(baseDir,
                                                                         QDir(FolderList::addPaths(settings.destDirs.at(i),
                                                                                                   relDir))
                                                                         .relativeFilePath(oldFileName));

                            TRACE(qDebug()<<"Equivalent on soucre: "<<oldSourceName);
                            if (QFileInfo(oldSourceName).exists()) {
                                //copied
                            } else {
                                //renamed or moved
                                renamed = true;
                                break;
                            }
                        } else {
                            // check dest drive removal ?
                            qWarning()<<"Destination drive unplugged? wrong dest files listing? "<<res;
                            settings.destDirs[i].removeFile(file,res);
                        }
                    }

                    if (oldFileName.size() > 0) {
                        if (!renamed) {
                            fileStatus += tr("copied");
                            // copied on source
                            TRACE(qDebug()<<"               --> file was probably copied");
                            // re-transfer the file (don't copy it on dest)
                        } else {
                            fileStatus += tr("moved or renamed");
                            // moved or renamed on source
                            TRACE(qDebug()<<"               --> file was probably moved or renamed");
                            // --> rename it on dest
                            if (FileSystemTools::moveWithDirs(oldFileName,destFilePath)) {
                                nbFilesRenamed++;
                                fileStatus += " -> "+tr("apply changes");
                                TRACE(qDebug()<<"                 ==> correctly moved on dest");
                                copy = false;
                                //TODO: update the dest listing
                                settings.destDirs[i].addFile(file,true,destFilePath);
                                settings.destDirs[i].removeFile(file,oldFileName);
                                /* and remove old file */
                                TRACE(qDebug()<<"                     listing updated");
                                if (settings.versioning) {
                                    //create simlink in oldversions folder
                                    QString garbagePath = getGarbageName(oldFileName+".lnk",settings.destDirs.at(i));
                                    if (FileSystemTools::linkWithDirs(garbagePath,destFilePath)) {
                                        TRACE(qDebug()<<"                        link created");
                                    } else {

                                        TRACE(qDebug()<<"                        problem while creating link\n"<<garbagePath<<"-->"<<destFilePath);
                                    }
                                }
                            } else {
                                sendLog(tr("Problem in move attempt within destination folder... ") + oldFileName + destFilePath);
                            }
                        }
                    } else {
                        TRACE(qDebug()<<"            --> really not there...");
                    }
                }
                int copyAttemps = 0;
                while (copy) {
                    setStatus(tr("copying..."));
                    setDetailedStatus(QString(tr("Copying%1 %2")).arg(speed()).arg(file.absoluteFilePath()));
                    fileStatus += " -> "+tr("Transfer");
                    TRACE(qDebug()<<"            ==> "<<"Copy...");
                    QTime fileTimer;
                    fileTimer.start();
                    if (FileSystemTools::copyWithDirs(file.absoluteFilePath(), destFilePath)) {
                        transferTimeMili += fileTimer.elapsed();
                        nbBytesTransfered += file.size();
                        TRACE(qDebug()<<"                "<<"Copied!");
                        nbFilesCopied++;
                        settings.destDirs[i].addFile(file,true,destFilePath);
                        copy = false;
                    } else {
                        if (copyAttemps < 1) {
                            bool missingDir = false;
                            qWarning()<<"                problem in copy attempt..." << file.absoluteFilePath() << destFilePath;
                            /* Either there is not enough space on destination, or destination or source was unplugged */
                            if (!file.exists() ||
                                !QDir(file.absolutePath()).exists()) {
                                qWarning()<<"                   original file or folder not found";
                                settings.sourceDirs[baseIdx].setPlugged(false);
                                sendLog(QString(tr("ERROR: cannot find source directory %1")).arg(file.absolutePath()));
                                sendLog(QString(tr("ERROR:  => aborting backup of %1")).arg(settings.sourceDirs.path(baseIdx)));
                                missingDir = true;
                            }
                            if (!QDir(QFileInfo(destFilePath).absolutePath()).exists()) {
                                qWarning()<<"                   destination folder not found";
                                settings.destDirs[i].setPlugged(false);
                                sendLog(QString(tr("ERROR: cannot find/create %1")).arg(QFileInfo(destFilePath).absolutePath()));
                                sendLog(QString(tr("ERROR:  => aborting backup to %1")).arg(settings.destDirs.at(i)));
                                missingDir = true;
                            }
                            if (!missingDir) {
                                /*  ==> trying to make space */
                                cleanOldVersion(file.size());
                            } else {
                                copy = false;
                                successful = false;
                                fileStatus = tr("Problem while copying");
                                sendLog(tr("ERROR: drive unplugged ?"));
                            }

                        } else {
                            copy = false;
                            sendLog(QString(tr("ERROR: could not copy %1 to %2")).arg(file.absoluteFilePath()).arg(destFilePath));
                        }
                    }
                    setStatus(tr("running..."));
                    copyAttemps++;
                }
                if (fileStatus.size() >0) {
                    if (!alreadyLogged) {
                        sendLog(file.absoluteFilePath());
                        alreadyLogged = true;
                    }
                    sendLog("   " + fileStatus + " " + tr("in") + " " + settings.destDirs.at(i));
                }
            }
        }
        if (watcher != NULL)
            watcher->addPath(file.absoluteFilePath());
    } else {
        /* notification on file removed or renamed: will be handled with directory notification */
        if (watcher != NULL)
            watcher->removePath(file.absoluteFilePath());
    }
}
QString BackupEngine::speed() {
    QString res = "";
    if (nbBytesTransfered>0) {
        res = QString(" @%1/s").arg(FileSystemTools::size2Str(nbBytesTransfered*1000/transferTimeMili));
    }
    return res;
}

void BackupEngine::treatFile(QString file){
    if (!logCleaned) {
        sendInitLog();
        logCleaned = true;
        sendLog("");
    }
    sendLog(QString(tr("Received notification on file %1")).arg(file));
    watcher->removePath(file);
    toProcess.insert(file);
    processTimer->start();
}

void BackupEngine::treatDir(QString dir){
    if (!logCleaned) {
        sendInitLog();
        logCleaned = true;
        sendLog("");
    }
    sendLog(QString(tr("Received notification on directory %1")).arg(dir));
    if (watcher->directories().contains(dir)) {
        TRACE(qDebug()<<"   -->not removed");
    } else {

        qDebug()<<"   -->REMOVED [IF YOU SEE THIS TRACE, THE WATCHER CLASS WAS PROBABLY CORRECTED]";
    }
    watcher->removePath(dir);
    toProcess.insert(dir);
    processTimer->start();
}

int BackupEngine::findSourceIdx(QString dirOrFile) {
    for (int sourceIdx = 0; sourceIdx<settings.sourceDirs.size(); sourceIdx++) {
        if ((dirOrFile+"/").startsWith(settings.sourceDirs.path(sourceIdx) + "/")) {
            return sourceIdx;
        }
    }
    qWarning()<<"Should Not happen!!! Unable to find parent source folder: "<<dirOrFile;
    return 0;
}

void BackupEngine::initStats(){
    timer.start();
    nbFiles = 0;
    nbFilesCopied = 0;
    nbFilesRenamed = 0;
    nbFilesRemoved = 0;
    nbFilesRemovedFromBin = 0;
    nbFolders = 0;
    nbBytesTransfered = 0;
    transferTimeMili = 0;
}

void BackupEngine::printStats(){
    totalFiles = nbFiles;
    sendLog(QString(tr("%1 files treated in %2 directories in %3 ms.")) .arg(nbFiles).arg(nbFolders).arg(timer.elapsed()));
    sendLog(QString(tr("%1 files transfered%2.")).arg(nbFilesCopied).arg(speed()));
    sendLog(QString(tr("%1 files moved or renamed.")).arg(nbFilesRenamed));
    sendLog(QString(tr("%1 files %2.")).arg(nbFilesRemoved).arg(settings.versioning?tr("Archived"):tr("Removed")));
    if (nbFilesRemovedFromBin > 0)
        sendLog(QString(tr("%1 archives removed.")).arg(nbFilesRemovedFromBin));
    if (watcher != NULL) {
        sendLog(QString(tr("Watched: %1 files and %2 directories."))
                .arg(watcher->files().size()).arg(watcher->directories().size()));
    }

    /*.arg((settings.destDirs.size()>0)?settings.destDirs.at(0).listing->size():0);*/
    //TRACE(qDebug()<<((settings.destDirs.size()>0)?settings.destDirs[0].toString():""));
}



void BackupEngine::copyCallBack(qint64 bytes) {
    TRACE(qDebug()<<bytes);
}

/* found at http://lists.trolltech.com/qt-interest/2004-11/thread00873-0.html
 * and slightly modified */

#ifdef _WIN32
        #include <windows.h>
#else // linux stuff
        #include <sys/vfs.h>
        #include <sys/stat.h>
#endif // _WIN32

bool getFreeTotalSpace(QString sDirPath,qlonglong *fTotal, qlonglong *fFree)
{
#ifdef _WIN32
    Folder::currentFolderMutex()->lock();
    QString sCurDir = QDir::current().absolutePath();
    QDir::setCurrent( sDirPath );

    ULARGE_INTEGER free,total;
    bool bRes = ::GetDiskFreeSpaceExA( 0 , &free , &total , NULL );
    if ( !bRes ) return false;

    QDir::setCurrent( sCurDir );

    *fFree  = static_cast<__int64>(free .QuadPart);
    *fTotal = static_cast<__int64>(total.QuadPart);
    Folder::currentFolderMutex()->unlock();

#else //linux

    struct stat stst;
    struct statfs stfs;

    if ( ::stat(sDirPath.local8Bit(),&stst) == -1 ) return false;
    if ( ::statfs(sDirPath.local8Bit(),&stfs) == -1 ) return false;

    *fFree  = stfs.f_bavail * ( stst.st_blksize );
    *fTotal = stfs.f_blocks * ( stst.st_blksize );

#endif // _WIN32
    return true;
}


void BackupEngine::cleanOldVersion(qint64 minSizeOnDisk) {

    TRACE(qDebug() << "min size to save: " << minSizeOnDisk);

    qlonglong fTotal;
    qlonglong fFree;
    for (int destIdx = 0; destIdx < settings.destDirs.size(); destIdx ++) {
        if (settings.destDirs.at(destIdx).isPlugged) {
            qint64 sizeToClean = 0;
            qint64 toClean;

            getFreeTotalSpace(settings.destDirs.at(destIdx),&fTotal, &fFree);
            TRACE(qDebug()<< settings.destDirs.at(destIdx));
            TRACE(qDebug()<< "freeSpace = " << FileSystemTools::size2Str(fFree) << " over " << FileSystemTools::size2Str(fTotal)
                  << " versionDirSize = " << FileSystemTools::size2Str(settings.destDirs.at(destIdx).versionDirSize)
                  << " mainDirSize = " << FileSystemTools::size2Str(settings.destDirs.at(destIdx).mainDirSize)  );

            if (fFree < minSizeOnDisk) {
                toClean = minSizeOnDisk - fFree;
                sendLog(QString(tr("Not enough space (%1) to copy the file (%2) => clean %3"))
                        .arg(FileSystemTools::size2Str(fFree))
                        .arg(FileSystemTools::size2Str(minSizeOnDisk))
                        .arg(FileSystemTools::size2Str(toClean)));
                sizeToClean = toClean;
            }

            if (settings.maxiPercent) {
                if (((double)settings.destDirs.at(destIdx).versionDirSize)
                        /((double)settings.destDirs.at(destIdx).mainDirSize)
                        > ((double)settings.maxiPercent_value)/100) {
                    toClean = (((double)settings.destDirs.at(destIdx).versionDirSize)
                               /((double)settings.destDirs.at(destIdx).mainDirSize)
                               - ((double)settings.maxiPercent_value)/100)
                            * settings.destDirs.at(destIdx).mainDirSize;
                    sendLog(QString(tr("The old files directory weigths %1, but should not exceed %2 (= %3% of %4) => clean %5"))
                            .arg(FileSystemTools::size2Str(settings.destDirs.at(destIdx).versionDirSize))
                            .arg(FileSystemTools::size2Str(settings.destDirs.at(destIdx).mainDirSize * settings.maxiPercent_value/100))
                            .arg(settings.maxiPercent_value)
                            .arg(FileSystemTools::size2Str(settings.destDirs.at(destIdx).mainDirSize))
                            .arg(FileSystemTools::size2Str(toClean)));
                    sizeToClean = std::max(sizeToClean,toClean);
                }
            }
            if (settings.maxiValue) {
                qint64 maxSize = settings.maxiValue_value * pow(1024,settings.maxiValue_unit+1);
                if (settings.destDirs.at(destIdx).versionDirSize > maxSize) {
                    toClean = settings.destDirs.at(destIdx).versionDirSize - maxSize;
                    sendLog(QString(tr("The old files directory weigths %1, but should not exceed %2 => clean %3"))
                            .arg(FileSystemTools::size2Str(settings.destDirs.at(destIdx).versionDirSize))
                            .arg(FileSystemTools::size2Str(maxSize))
                            .arg(FileSystemTools::size2Str(toClean)));
                    sizeToClean = std::max(sizeToClean,toClean);
                }

            }
            if (settings.keepPercent) {
                if (fFree < settings.keepPercent_value*fTotal/100) {
                    toClean = settings.keepPercent_value*fTotal/100 - fFree;
                    sendLog(QString(tr("There is %1 left on device, but should not go under %2 (= %3% of %4) => clean %5"))
                            .arg(FileSystemTools::size2Str(fFree))
                            .arg(FileSystemTools::size2Str(fTotal * settings.keepPercent_value/100))
                            .arg(settings.keepPercent_value)
                            .arg(FileSystemTools::size2Str(fTotal))
                            .arg(FileSystemTools::size2Str(toClean)));
                    sizeToClean = std::max(sizeToClean,toClean);
                }
            }
            if (sizeToClean > 0) {
                sendLog(QString(tr("Cleaning %1 from %2"))
                        .arg(FileSystemTools::size2Str(sizeToClean))
                        .arg(settings.destDirs[destIdx].versionFolder()));
                QMapIterator<QDateTime,QString> i(*(settings.destDirs[destIdx].versionFiles));
                while (i.hasNext() && sizeToClean > 0 ) {
                    i.next();
                    QFileInfo fileInfo(i.value());
                    setDetailedStatus(QString(tr("Removing %1")).arg(i.value()));
                    sendLog(QString(tr("%1 left to clean. Removing %2"))
                            .arg(FileSystemTools::size2Str(sizeToClean))
                            .arg(i.value()));
                    settings.destDirs[destIdx].removeFile(fileInfo);
                    sizeToClean -= fileInfo.size();
                    removeArchive(i.value());
                }
            }

            if (settings.maxiDays) {
                bool more = true;
                bool first = true;
                QDateTime current = QDateTime::currentDateTime();
                QMapIterator<QDateTime,QString> i(*(settings.destDirs[destIdx].versionFiles));
                while (i.hasNext() && more ) {
                    i.next();
                    int ageInDays = DestFolderInfo::extractDate(i.value()).daysTo(current);
                    TRACE(qDebug()<<DestFolderInfo::extractDate(i.value()));
                    TRACE(qDebug()<<ageInDays);
                    if (ageInDays > settings.maxiDays_value) {
                        if (first) {
                            sendLog(QString(tr("Some archives are older than %1 days. Cleaning now."))
                                    .arg(settings.maxiDays_value));
                            first = false;
                        }
                        setDetailedStatus(QString(tr("Removing %1")).arg(i.value()));
                        sendLog(QString(tr("%1 days old. Removing %2"))
                                .arg(ageInDays)
                                .arg(i.value()));

                        settings.destDirs[destIdx].removeFile(QFileInfo(i.value()));
                        removeArchive(i.value());
                    } else {
                        more = false;
                    }
                }
            }
        }
    }
}


bool BackupEngine::removeArchive(QString fileName) {
    if (QFile(fileName).remove()) {
        TRACE(qDebug()<<"file "<<fileName<<" correctly removed");
        nbFilesRemovedFromBin++;
    } else {
        /* perhaps this is a folder ending with .lnk ? */
        if (!QDir(fileName).rmdir(fileName)) {
            sendLog(QString(tr("Problem while removing archive: %1")).arg(fileName));
            return false;
        }
    }
    return true;
}


void    BackupEngine::setStatus(QString _status){
    statusMutex.lock();
    status = _status;
    statusMutex.unlock();
}

QString BackupEngine::getStatus(){
    QString res;
    statusMutex.lock();
    res = status;
    statusMutex.unlock();
    return res;
}

void    BackupEngine::setDetailedStatus(QString _detailedStatus){
    detailedStatusMutex.lock();
    detailedStatus = _detailedStatus;
    detailedStatusMutex.unlock();
}

QString BackupEngine::getDetailedStatus(){
    QString res;
    detailedStatusMutex.lock();
    res = detailedStatus;
    detailedStatusMutex.unlock();
    return res;
}
