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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "profilecreator.h"
#include "backuplistmodel.h"
#include <QSystemTrayIcon>
#include <QLabel>
#include "languagechooser.h"


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(ProfileCreator *_pc, LanguageChooser * _lc, QWidget *parent = 0);
    ~MainWindow();

    QSystemTrayIcon sysTray;
    BackupListModel *backupListModel;

    bool quitExplicitelyAsked;

    void closeEvent(QCloseEvent *event);

public slots:
    void addProfile_clicked();
    void saveProfile_clicked();
    void removeProfile_clicked();
    void editProfile_clicked();
    void launch_clicked();
    void launch_all_clicked();
    void close_clicked();
    void import_clicked();
    void export_clicked();
    void updateButtonsStatus();
    void contextMenu(QPoint);
    void sysTrayHandle(QSystemTrayIcon::ActivationReason);
    void showLogs();
    void hideLogs();
    void showMainWindow();
    void updateStatusBar();
    void retranslate();

private:
    Ui::MainWindow *ui;
    ProfileCreator *pc;
    LanguageChooser *lc;
    QTextEdit * defaultLog;
};

#endif // MAINWINDOW_H
