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

#include <QtGui/QApplication>
#include "mainwindow.h"
#include "profilecreator.h"
#include <stdio.h>
#include "backupsettings.h"
#include "trace.h"
#include <QDateTime>
#include <QSettings>
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <QMessageBox>
#include "instancemanager.h"
#include <QTranslator>
#include <languagechooser.h>
#include "filesystemtools.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    /*
    QDir d("K:/backup/source test/toto");
    d.setFilter(QDir::System | QDir::AllEntries);

    FTRACE(qDebug()<<QDir(d.entryInfoList().at(3).absoluteFilePath()).exists());
    //FTRACE(qDebug()<<d.rmdir(d.entryInfoList().at(3).absoluteFilePath()));
    //FTRACE(qDebug()<<QFile(d.entryInfoList().at(3).absoluteFilePath()).remove());
    FTRACE(qDebug()<<QDir(d.entryInfoList().at(3).absoluteFilePath()).count());
    FTRACE(qDebug()<<QDir(d.entryInfoList().at(3).absoluteFilePath()).entryList().at(0));
    FTRACE(qDebug()<<d.count());

    FTRACE(qDebug()<<d.entryList().at(0));
    FTRACE(qDebug()<<d.entryList().at(1));
    FTRACE(qDebug()<<d.entryList().at(2));
    FTRACE(qDebug()<<d.entryList().at(3));
    FTRACE(qDebug()<<d.entryInfoList().at(3).isSymLink());
    FTRACE(qDebug()<<d.entryInfoList().at(3).isDir());

    return 15;
    */
    /*
    QSystemSemaphore sem("StarBackup", 1, QSystemSemaphore::Open);

    int ret = sem.error();

    FTRACE(qDebug()<<ret);

    if (ret == QSystemSemaphore::AlreadyExists) {
        QMessageBox msgBox;
        msgBox.setText(QString("StarBackup is already running!"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        int ret = msgBox.exec();
        exit(0);
    }
    */

    QSharedMemory sm ("StarBackupInstance");
    if (!sm.create(1)) {
        printf("%s",sm.errorString().toAscii().constData());
        QMessageBox msgBox;
        msgBox.setText(QString("StarBackup is already running!"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::No);
/* Fuck you, what if the previous instance crashed^wwaskilled and the sm still exists ??

  If I understand well what you're trying to do, why don't you use QtSingleApplication ?
*/
/*      sm.attach();
#if 0*/
        int ret = msgBox.exec();

        QSystemSemaphore sem("StarBackupSemaphore", 0, QSystemSemaphore::Open);
        sem.release(1);

        exit(0);

/*#endif*/
    }

    QCoreApplication::setOrganizationName("StarBackup");
    QCoreApplication::setApplicationName("StarBackup");
    QCoreApplication::setApplicationVersion("0.1");
    QCoreApplication::setOrganizationDomain("StarBackup.net");

    QSettings settings(QSettings::IniFormat,
                       QSettings::UserScope,
                       QCoreApplication::organizationName(),
                       QCoreApplication::applicationName());
    QDir dir = QDir(QFileInfo(settings.fileName()).absolutePath());
    TRACE(qDebug()<<dir);
    TRACE(qDebug()<<QDir::current());

    if (!dir.exists()) {
        dir.mkpath(dir.absolutePath());
    }

    LanguageChooser * lc = new LanguageChooser(QDir::current().absolutePath(),&a,&settings);

    TRACE(qDebug()<<QDir::current());

    if (QDir::setCurrent(dir.absolutePath())) {
        TRACE(qDebug()<<QDir::current());
    } else {
        TRACE(qDebug()<<"boo");
    }


    ProfileCreator *pc = new ProfileCreator();
    MainWindow *w = new MainWindow(pc,lc);
    InstanceManager im(w);
    im.start();
    if (!(argc > 1 && QString::compare(argv[1],"-hide")==0)) {
        w->show();
    }

    if (w->backupListModel->backupList.size() == 0) {
        QMessageBox msgBox;
        msgBox.setText(QString(a.tr("No settings found!")));
        msgBox.setInformativeText(QString(a.tr("Would you like to configure a backup now?")));
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);

        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes) {
            w->addProfile_clicked();
        }
    }

    return a.exec();
}
