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

#include "backupsettings.h"
#include <QTextStream>
#include <QtDebug>
#include <QMessageBox>
#include <QApplication>
#include "trace.h"

BackupSettings::BackupSettings(){
    file = NULL;
    setDefault();
}

BackupSettings::BackupSettings(QFile *_file){
    file = _file;
    loadFromXML();
}

void BackupSettings::setDefault(){

    title = QFile::tr("Backup");
    /* SourceDirs;
    DestDir;*/

    versioning = true;
    maxiPercent = false;
    maxiValue = false;
    keepPercent = true;
    maxiPercent_value = 20;
    maxiValue_value = 30;
    maxiValue_unit = 2;
    keepPercent_value = 2;
    maxiDays = true;
    maxiDays_value = 365;

    realTime = false;
    scheduled = true;
    noSchedule = false;
    dayly = true;
    weekly = false;
    every = false;
    daylyTime = QTime(1,0);
    weeklyDay = Qt::Sunday ;
    weeklyTime = QTime(1,0);
    everyNb = 2;
    everyUnit = 2;
    catchUpMissed = true;
    catchUpValue = 5;

    lastTentative = QDateTime::fromMSecsSinceEpoch(0);
    lastSuccess = QDateTime::fromMSecsSinceEpoch(0);

}

bool BackupSettings::saveToXML(QString FileName){
    QFile *saveFile;
    QDomDocument * doc = new QDomDocument();
    QDomElement root = getXmlElem(doc);
    doc->appendChild(root);

    if (FileName.size() > 0) {
        saveFile = new QFile(FileName);
    } else {
        if (file == NULL) {
            int i = 1;
            file = new QFile();
            do {
                file->setFileName(QString("backup%1.xml").arg(i,3,10,QChar('0')));
                i++;
            } while (file->exists() || file->exists(file->fileName()+=".removed"));
        }
        saveFile = file;
    }

    bool res = saveFile->open(QIODevice::WriteOnly | QFile::Text);
    if (res) {
        QTextStream *ts = new QTextStream(saveFile);
        doc->save(*ts,4);
        saveFile->close();
        delete ts;
    }
    delete doc;
    return res;
}

bool BackupSettings::loadFromXML() {
#define MAX_TRIES   3
    int tries = 0;

    QDomDocument doc;
    while (!file->open(QIODevice::ReadOnly) && tries < MAX_TRIES) {
        tries++;
        TRACE(qDebug()<<"Could not open " + file->fileName() + " : " + file->errorString());
    }
    if (tries >= MAX_TRIES) {
        QMessageBox msgBox;
        msgBox.setText(QString(QFile::tr("problem while loading %1")).arg(file->fileName()));
        msgBox.setInformativeText(file->errorString());
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok);
        int ret = msgBox.exec();
        return false;
    }

    tries = 0;
    QString errMsg;
    int line;
    int col;
    while (!doc.setContent(file,&errMsg,&line,&col) && tries < MAX_TRIES) {
        tries++;
        TRACE(qDebug()<<errMsg<<line<<col);
    }
    if (tries >= MAX_TRIES) {
        QMessageBox msgBox;
        msgBox.setText(QString(QFile::tr("problem while loading %1")).arg(file->fileName()));
        msgBox.setInformativeText(QString("(%1,%2) : %3").arg(line).arg(col).arg(errMsg));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok);
        int ret = msgBox.exec();
        file->close();
        return false;
    }

    file->close();

    setDefault();
    loadFromElem(doc.documentElement());
    return true;
}


bool BackupSettings::loadFromElem(QDomElement elem) {
    title = getValueFromXML(elem,"title");

    QDomNodeList destDirNodes = elem.elementsByTagName("destDir");
    destDirs.clear();
    for (int i=0; i<destDirNodes.size(); i++) {
        QDomElement el = destDirNodes.item(i).toElement();
        DestFolderInfo f = DestFolderInfo(el.text());
        f.hardwareID = el.attribute("hardwareID");
        destDirs << f;
    }
    QDomNodeList sourceDirNodes = elem.elementsByTagName("sourceDir");
    sourceDirs.clear();
    for (int i=0; i<sourceDirNodes.size(); i++) {
        QDomElement el = sourceDirNodes.item(i).toElement();
        Folder f = Folder(el.text());
        f.hardwareID = el.attribute("hardwareID");
        sourceDirs << f;
    }

    sourceDirs.organization  = getValueFromXML(elem,"organization") .toInt();
    if (getValueFromXML(elem,"versioning").size()>0) {
        versioning      = getValueFromXML(elem,"versioning")        .toInt()!=0;
    } else {
        versioning      = getValueFromXML(elem,"versionning")       .toInt()!=0;
    }
    maxiPercent         = getValueFromXML(elem,"maxiPercent")       .toInt()!=0;
    maxiValue           = getValueFromXML(elem,"maxiValue")         .toInt()!=0;
    keepPercent         = getValueFromXML(elem,"keepPercent")       .toInt()!=0;
    maxiPercent_value   = getValueFromXML(elem,"maxiPercent_value") .toInt();
    maxiValue_value     = getValueFromXML(elem,"maxiValue_value")   .toInt();
    maxiValue_unit      = getValueFromXML(elem,"maxiValue_unit")    .toInt();
    keepPercent_value   = getValueFromXML(elem,"keepPercent_value") .toInt();
    maxiDays            = getValueFromXML(elem,"maxiDays")          .toInt()!=0;
    maxiDays_value      = getValueFromXML(elem,"maxiDays_value")    .toInt();
    realTime            = getValueFromXML(elem,"realTime")          .toInt()!=0;
    scheduled           = getValueFromXML(elem,"scheduled")         .toInt()!=0;
    noSchedule          = getValueFromXML(elem,"noSchedule")        .toInt()!=0;
    dayly               = getValueFromXML(elem,"dayly")             .toInt()!=0;
    weekly              = getValueFromXML(elem,"weekly")            .toInt()!=0;
    every               = getValueFromXML(elem,"every")             .toInt()!=0;
    daylyTime     = QTime(getValueFromXML(elem,"daylyTimeH")        .toInt(),
                          getValueFromXML(elem,"daylyTimeM")        .toInt());
    weeklyDay = (Qt::DayOfWeek) getValueFromXML(elem,"weeklyDay")   .toInt();
    weeklyTime    = QTime(getValueFromXML(elem,"weeklyTimeH")       .toInt(),
                          getValueFromXML(elem,"weeklyTimeM")       .toInt());
    everyNb             = getValueFromXML(elem,"everyNb")           .toInt();
    everyUnit           = getValueFromXML(elem,"everyUnit")         .toInt();
    catchUpMissed       = getValueFromXML(elem,"catchUpMissed")     .toInt()!=0;
    catchUpValue        = getValueFromXML(elem,"catchUpValue")      .toInt();
    if (catchUpValue < 1)
        catchUpValue = 1;
    lastTentative = QDateTime::fromString(getValueFromXML(elem,"lastTentative"),"yyyy-MM-dd HH:mm:ss");
    lastSuccess   = QDateTime::fromString(getValueFromXML(elem,"lastSuccess")  ,"yyyy-MM-dd HH:mm:ss");

    return true;
}

QString BackupSettings::getValueFromXML(QDomElement doc, QString str) {
    QDomNodeList list = doc.elementsByTagName(str);
    if (list.count()==1) {
        QDomElement elem = list.item(0).toElement();
        if (!elem.isNull()) {
            return elem.text();
        } else {

            TRACE(qDebug() << str << " is not an element.");
            return QString();
        }
    } else if (list.count()<1) {
        TRACE(qDebug() << "Unable to find element " << str );
        return QString();
    } else {
        TRACE(qDebug() << "Too much instances of the element " << str << ". Count=" << list.count());
        return QString();
    }
}


QDomElement BackupSettings::getXmlElem(QDomDocument * doc){
    QDomElement root = doc->createElement("BackupSettings");
    QDomElement destElement = doc->createElement("DestDirs");
    QDomElement sourceElement = doc->createElement("SourceDirs");

    addElemToXML(doc,root,QString("title"),                     title);
    for (int i = 0; i < destDirs.size(); ++i) {
        QDomElement el = addElemToXML(doc,destElement,QString("destDir"),  destDirs.at(i));
        el.setAttribute("hardwareID",destDirs.at(i).hardwareID);
    }
    root.appendChild(destElement);
    for (int i = 0; i < sourceDirs.size(); ++i) {
        QDomElement el = addElemToXML(doc,sourceElement,QString("sourceDir"),  sourceDirs.at(i));
        el.setAttribute("hardwareID",sourceDirs.at(i).hardwareID);
    }
    addElemToXML(doc,sourceElement,QString("organization"),  QString().setNum(sourceDirs.organization));
    root.appendChild(sourceElement);

    addElemToXML(doc,root,QString("versioning"),        QString().setNum(versioning     ?1:0));
    addElemToXML(doc,root,QString("maxiPercent"),       QString().setNum(maxiPercent    ?1:0));
    addElemToXML(doc,root,QString("maxiValue"),		QString().setNum(maxiValue      ?1:0));
    addElemToXML(doc,root,QString("keepPercent"),       QString().setNum(keepPercent    ?1:0));
    addElemToXML(doc,root,QString("maxiPercent_value"),	QString().setNum(maxiPercent_value));
    addElemToXML(doc,root,QString("maxiValue_value"),	QString().setNum(maxiValue_value));
    addElemToXML(doc,root,QString("maxiValue_unit"),	QString().setNum(maxiValue_unit));
    addElemToXML(doc,root,QString("keepPercent_value"),	QString().setNum(keepPercent_value));
    addElemToXML(doc,root,QString("maxiDays"),          QString().setNum(maxiDays       ?1:0));
    addElemToXML(doc,root,QString("maxiDays_value"),	QString().setNum(maxiDays_value));

    addElemToXML(doc,root,QString("realTime"),          QString().setNum(realTime       ?1:0));
    addElemToXML(doc,root,QString("scheduled"),         QString().setNum(scheduled      ?1:0));
    addElemToXML(doc,root,QString("noSchedule"),        QString().setNum(noSchedule     ?1:0));
    addElemToXML(doc,root,QString("dayly"),             QString().setNum(dayly          ?1:0));
    addElemToXML(doc,root,QString("weekly"),            QString().setNum(weekly         ?1:0));
    addElemToXML(doc,root,QString("every"),             QString().setNum(every          ?1:0));
    addElemToXML(doc,root,QString("daylyTimeH"),	QString().setNum(daylyTime.hour()));
    addElemToXML(doc,root,QString("daylyTimeM"),	QString().setNum(daylyTime.minute()));
    addElemToXML(doc,root,QString("weeklyTimeH"),	QString().setNum(weeklyTime.hour()));
    addElemToXML(doc,root,QString("weeklyTimeM"),	QString().setNum(weeklyTime.minute()));
    addElemToXML(doc,root,QString("weeklyDay"),         QString().setNum(weeklyDay));
    addElemToXML(doc,root,QString("everyNb"),           QString().setNum(everyNb));
    addElemToXML(doc,root,QString("everyUnit"),         QString().setNum(everyUnit));
    addElemToXML(doc,root,QString("catchUpMissed"),     QString().setNum(catchUpMissed  ?1:0));
    addElemToXML(doc,root,QString("catchUpValue"),      QString().setNum(catchUpValue));
    addElemToXML(doc,root,QString("lastTentative"),     lastTentative.toString("yyyy-MM-dd HH:mm:ss"));
    addElemToXML(doc,root,QString("lastSuccess"),       lastSuccess  .toString("yyyy-MM-dd HH:mm:ss"));


    //QString xml = doc.toString();
    //std::cout << xml.toStdString();
    return root;
}

QDomElement BackupSettings::addElemToXML(QDomDocument * doc, QDomElement parent, QString str, QString value) {
    QDomElement tag = doc->createElement(str);
    parent.appendChild(tag);
    QDomText t = doc->createTextNode(value);
    tag.appendChild(t);
    return tag;
}
















