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

#ifndef BACKUPSETTINGS_H
#define BACKUPSETTINGS_H

#include <QObject>
#include <QDir>
#include <QList>
#include <QtXml/QDomNode>
#include <QFile>
#include "folderlist.h"
#include "destfolderinfo.h"
#include <QDateTime>
#include <QTime>

class BackupSettings
{
public:
    BackupSettings();
    /* BackupSettings(profilecreator *pc, QObject *parent = 0);*/
    BackupSettings(QFile *file);

    void setDefault();
    bool loadFromXML() ;
    bool saveToXML(QString FileName = "") ;
    bool loadFromElem(QDomElement elem) ;
    static QString getValueFromXML(QDomElement doc, QString str) ;
    QDomElement getXmlElem(QDomDocument * doc);
    static QDomElement addElemToXML(QDomDocument * doc, QDomElement parent, QString str, QString value) ;


    QFile *file;

    QString title;
    FolderList sourceDirs;
    QList<DestFolderInfo> destDirs;

    bool versioning;
    bool maxiPercent;
    bool maxiValue;
    bool keepPercent;
    int maxiPercent_value;
    int maxiValue_value;
    int maxiValue_unit;
    int keepPercent_value;
    bool maxiDays;
    int maxiDays_value;

    //scheduler
    bool realTime;
    bool scheduled;
    bool noSchedule;
    bool dayly;
    bool weekly;
    bool every;
    QTime daylyTime;
    Qt::DayOfWeek weeklyDay;
    QTime weeklyTime;
    int everyNb;
    int everyUnit;
    bool catchUpMissed;
    int catchUpValue;

    QDateTime lastTentative;
    QDateTime lastSuccess;

};

#endif // BACKUPSETTINGS_H
