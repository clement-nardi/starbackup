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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_profilecreator.h"
#include "trace.h"
#include "backupengine.h"
#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>


MainWindow::MainWindow(ProfileCreator *_pc, LanguageChooser * _lc, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    pc = _pc;
    lc = _lc;
    quitExplicitelyAsked = false;
    ui->setupUi(this);

    backupListModel = ui->backupListWidget->backupListModel;

    defaultLog = new QTextBrowser();
    defaultLog->append(tr("Click on a backup to see its log."));
    ui->logs->addWidget(defaultLog);

    for (int i = 0; i<backupListModel->backupList.size(); i++) {
        //ui->logs->addWidget(backupListModel->backupList.at(i)->logView);
    }
    /*
    TRACE(qDebug()<<"flags=" << backupListModel->flags(QModelIndex()));
    //ui->backupListWidget->lw->setViewMode(QListView::IconMode);
    //ui->backupListWidget->lw->setSelectionRectVisible(false);
    ui->backupListWidget->lw->setIconSize(QSize(40,40));
    ui->backupListWidget->lw->setModel(backupListModel);
    ui->backupListWidget->lw->setContextMenuPolicy(Qt::CustomContextMenu);
    TRACE(qDebug()<<"flags=" << backupListModel->flags(QModelIndex()));
*/
    QMenu *menu = new QMenu;
    menu->addAction(ui->actionShow);
    menu->addAction(ui->actionStart_all_backups);
    menu->addAction(ui->actionQuit);
    sysTray.setContextMenu(menu);
    sysTray.setIcon(QIcon(":/icons/star"));
    sysTray.show();

    connect(ui->backupListWidget->lw,SIGNAL(activated(QModelIndex)),this,SLOT(editProfile_clicked()));
    connect(ui->backupListWidget->lw->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(updateButtonsStatus()));
    connect(ui->backupListWidget->lw->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this,SLOT(updateButtonsStatus()));
    connect(ui->backupListWidget->lw,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(contextMenu(QPoint)));
    connect(ui->actionAdd_Profile,SIGNAL(triggered()),this,SLOT(addProfile_clicked()));
    connect(ui->actionEdit_backup,SIGNAL(triggered()),this,SLOT(editProfile_clicked()));
    connect(ui->actionRemove_backup,SIGNAL(triggered()),this,SLOT(removeProfile_clicked()));
    connect(ui->actionStart_backup,SIGNAL(triggered()),this,SLOT(launch_clicked()));
    connect(ui->actionStart_all_backups,SIGNAL(triggered()),this,SLOT(launch_all_clicked()));
    connect(ui->actionImport_Backup,SIGNAL(triggered()),this,SLOT(import_clicked()));
    connect(ui->actionExport_Backup,SIGNAL(triggered()),this,SLOT(export_clicked()));

    connect(ui->actionQuit, SIGNAL(triggered()),this,SLOT(close_clicked()));
    connect(ui->actionShow, SIGNAL(triggered()),this,SLOT(show()));
    connect(pc->ui->saveButton,SIGNAL(clicked()),this,SLOT(saveProfile_clicked()));
    connect(&sysTray,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(sysTrayHandle(QSystemTrayIcon::ActivationReason)));

    connect(ui->actionShow_logs, SIGNAL(triggered()),this,SLOT(showLogs()));
    connect(ui->actionHide_logs, SIGNAL(triggered()),this,SLOT(hideLogs()));

    connect(ui->actionChange_language, SIGNAL(triggered()), lc, SLOT(reloadAndShow()));
    connect(lc,SIGNAL(languageChanged()),this,SLOT(retranslate()));
    connect(lc,SIGNAL(languageChanged()),pc,SLOT(retranslate()));
    connect(lc,SIGNAL(languageChanged()),lc,SLOT(retranslate()));

    if (lc->s->value("mainWindow/logsVisible").toBool()) {
        showLogs();
    } else {
        hideLogs();
    }

    if (lc->s->contains("mainWindow/geometry")) {
        setGeometry(lc->s->value("mainWindow/geometry").value<QRect>());
    } else {
        setGeometry(QRect(100,60,300,500));
    }

    if (lc->s->contains("mainWindow/splitterState")) {
        ui->splitter->restoreState(lc->s->value("mainWindow/splitterState").toByteArray());
    }


    TRACE(qDebug("after connections"));
    updateButtonsStatus();
    retranslate();

    //statusLabel = new QLabel(this);
    //statusBar()->addWidget(statusLabel);

    QTimer * timer = new QTimer(this);
    timer->setInterval(100);
    timer->setSingleShot(false);
    connect(timer,SIGNAL(timeout()),this,SLOT(updateStatusBar()));
    timer->start();

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::retranslate() {
    ui->retranslateUi(this);
    ui->actionChange_language->setIcon(QIcon(":/icons/resources/flag_" + lc->s->value("language").toString() + ".png"));
}

void MainWindow::showLogs() {
    ui->logs->setVisible(true);
    ui->actionShow_logs->setVisible(false);
    ui->actionHide_logs->setVisible(true);
    QRect g = geometry();
    g.setWidth(g.width()*3);
    setGeometry(g);
}

void MainWindow::hideLogs() {
    ui->logs->setVisible(false);
    ui->actionShow_logs->setVisible(true);
    ui->actionHide_logs->setVisible(false);
    QRect g = geometry();
    g.setWidth(g.width()/3);
    setGeometry(g);
}


void MainWindow::close_clicked(){
    QMessageBox msgBox;
    msgBox.setText(QString(tr("Are you sure you want to quit?")));
    msgBox.setInformativeText(tr("The scheduled backups will not be run and your files and folders will be left unprotected."));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if (ret==QMessageBox::Yes) {
        quitExplicitelyAsked = true;
        close();
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (quitExplicitelyAsked) {
        sysTray.hide();
        for (int i = 0; i<backupListModel->backupList.size(); i++ ) {
            backupListModel->backupList.at(i)->logFile.flush();
            backupListModel->backupList.at(i)->logFile.close();
        }
        lc->s->setValue("mainWindow/geometry",this->geometry());
        lc->s->setValue("mainWindow/logsVisible",ui->logs->isVisible());
        lc->s->setValue("mainWindow/splitterState",ui->splitter->saveState());
        event->accept();
    } else {
        hide();
        event->ignore();
    }
}


void MainWindow::sysTrayHandle(QSystemTrayIcon::ActivationReason reason){
    if (reason == QSystemTrayIcon::DoubleClick) {
        show();
    }
}

void MainWindow::addProfile_clicked() {
    TRACE(qDebug("add!"));
    pc->bs = new BackupSettings();
    pc->loadSettings();
    pc->ui->tabs->setCurrentIndex(0);
    pc->ui->SourceFoldersTree->collapseAll();
    pc->show();
}

void MainWindow::editProfile_clicked() {
    if (ui->backupListWidget->lw->selectionModel()->hasSelection()) {
        int idx = ui->backupListWidget->lw->selectionModel()->currentIndex().row();
        if (idx<backupListModel->backupList.size()) {
            TRACE(qDebug()<<"edit! idx="<< idx);
            pc->bs = backupListModel->backupList.at(idx)->externalSettings;
            pc->loadSettings();
            pc->ui->tabs->setCurrentIndex(0);
            pc->ui->SourceFoldersTree->collapseAll();
            pc->show();
        }
    }
}

void MainWindow::removeProfile_clicked() {
    if (ui->backupListWidget->lw->selectionModel()->hasSelection()) {
        int idx = ui->backupListWidget->lw->selectionModel()->currentIndex().row();
        if (idx<backupListModel->backupList.size()) {
            TRACE(qDebug()<<"remove! idx="<< idx);
            backupListModel->removeBackup(idx);
        }
    }
    updateButtonsStatus();
}

void MainWindow::saveProfile_clicked() {
    pc->saveSettings();
    backupListModel->addBackup(pc->bs);
    pc->hide();
    updateButtonsStatus();
}

void MainWindow::launch_clicked() {
    if (ui->backupListWidget->lw->selectionModel()->hasSelection()) {
        int idx = ui->backupListWidget->lw->selectionModel()->currentIndex().row();
        if (idx<backupListModel->backupList.size()) {
            TRACE(qDebug()<<"launch! idx="<< idx);
            backupListModel->backupList.at(idx)->launch();
        }
    }
}

void MainWindow::import_clicked() {
    BackupSettings *importBS;
    TRACE(qDebug()<<"import! ");

    QStringList files = QFileDialog::getOpenFileNames(this,
                                                 tr("Choose the XML file(s) containing the backup(s) to import"),
                                                 QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation),
                                                 tr("XML files") + " (*.xml)");
    for (int i = 0; i<files.size(); i++) {
        importBS = new BackupSettings(new QFile(files.at(i)));
        delete importBS->file;
        importBS->file = NULL;
        importBS->saveToXML();
        backupListModel->addBackup(importBS);
        updateButtonsStatus();
    }
}

void MainWindow::export_clicked() {
    if (ui->backupListWidget->lw->selectionModel()->hasSelection()) {
        int idx = ui->backupListWidget->lw->selectionModel()->currentIndex().row();
        if (idx<backupListModel->backupList.size()) {
            TRACE(qDebug()<<"export! idx="<< idx);
            QString file = QFileDialog::getSaveFileName(this,
                                                        tr("Save a backup to an XML file"),
                                                        QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation),
                                                        tr("XML files") + " (*.xml)");
            if (file.size()>0) {
                backupListModel->backupList.at(idx)->externalSettings->saveToXML(file);
            }
        }
    }
}

void MainWindow::launch_all_clicked() {
    TRACE(qDebug()<<"launch all!");
    for (int i = 0; i<backupListModel->backupList.size(); i++) {
        backupListModel->backupList.at(i)->launch();
    }
}

void MainWindow::updateButtonsStatus() {
    bool enabled = false;
    int idx = 0;
    if (ui->backupListWidget->lw->selectionModel()->hasSelection()) {
        idx = ui->backupListWidget->lw->selectionModel()->currentIndex().row();
        if (idx<backupListModel->backupList.size()) {
            enabled = true;
        }
    }
    ui->actionEdit_backup->setEnabled(enabled);
    ui->actionRemove_backup->setEnabled(enabled);
    ui->actionStart_backup->setEnabled(enabled);
    ui->actionExport_Backup->setEnabled(enabled);
    if (enabled) {
        ui->logs->addWidget(backupListModel->backupList.at(idx)->logView);
        ui->logs->setCurrentWidget(backupListModel->backupList.at(idx)->logView);
    } else {
        ui->logs->setCurrentWidget(defaultLog);
    }
}


void MainWindow::contextMenu(QPoint point) {
    TRACE(qDebug()<<"Click!"<<point);
    bool enabled = ui->backupListWidget->lw->indexAt(point).isValid();

    QMenu *menu = new QMenu;

    if (enabled) {
        menu->addAction(ui->actionEdit_backup);
        menu->addAction(ui->actionRemove_backup);
        menu->addAction(ui->actionStart_backup);
    } else {
        menu->addAction(ui->actionAdd_Profile);
    }

    menu->exec(ui->backupListWidget->lw->mapToGlobal(point));
}


void MainWindow::showMainWindow() {
    /*
    Qt::WindowFlags savedFlags = windowFlags();
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    this->show();
    this->setWindowFlags(savedFlags);
    this->show();
    */

    //close();
    showNormal();
}

void MainWindow::updateStatusBar() {
    static uint counter = 0;
    QString status = "";
    if (ui->backupListWidget->lw->selectionModel()->hasSelection()) {
        int idx = ui->backupListWidget->lw->selectionModel()->currentIndex().row();
        if (idx<backupListModel->backupList.size()) {
            status = backupListModel->backupList.at(idx)->getDetailedStatus();
        }
    }

    QString appendix = " ";
    if (status.size() > 0) {
        counter++;
        if (counter%4 == 1)
            appendix = " .";
        if (counter%4 == 2)
            appendix = " ..";
        if (counter%4 == 3)
            appendix = " ...";
    }
    //statusLabel->setText(status+appendix);
    statusBar()->showMessage(status+appendix,0);
}
