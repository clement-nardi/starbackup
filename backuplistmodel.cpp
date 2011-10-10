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

#include "backuplistmodel.h"
#include "trace.h"
#include <QFont>
#include <QBrush>
#include <QMessageBox>

BackupListModel::BackupListModel(QObject *parent) :
    QAbstractListModel(parent)
{
    TRACE(qDebug("BackupListModel constructor"));
    QFileInfoList files = QDir().entryInfoList(QStringList()<<"*.xml");
    QList<BackupSettings *> settingsList;
    for (int i = 0; i<files.size(); i++) {
        TRACE(qDebug()<<"BackupListModel " << files.at(i).absoluteFilePath());
        BackupSettings * curSettings = new BackupSettings(new QFile(files.at(i).absoluteFilePath()));
        if (curSettings->title != "")
            settingsList << curSettings;
    }
    /* create the settings first and then engines to avoid file loading issues... ? */
    for (int i = 0; i< settingsList.size(); i++) {
        backupList.append(new BackupEngine(settingsList.at(i)));
    }
    backupIcon = new QIcon(":/icons/backup");
    TRACE(qDebug("BackupListModel constructor finished"));
}

void BackupListModel::addBackup(BackupSettings *bs){
    for (int i = 0; i<backupList.size(); i++) {
        if (backupList.at(i)->externalSettings == bs) {
            TRACE(qDebug("Backup already there"));
            backupList.at(i)->launch();
            return;
        }
    }
    //QAbstractListModel::beginInsertRows();
    backupList.append(new BackupEngine(bs));
    //QAbstractListModel::endInsertRows();
    TRACE(qDebug("Backup added to the list"));

    dataChanged(QModelIndex(),QModelIndex());
}

void BackupListModel::removeBackup(int idx){
    QMessageBox msgBox;
    msgBox.setText(QString(tr("Are you sure you want to remove the backup \"%1\" ?")).arg(backupList.at(idx)->externalSettings->title));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();
    if (ret==QMessageBox::Yes) {
        backupList.at(idx)->externalSettings->file->rename(QString(backupList.at(idx)->externalSettings->file->fileName())+=".removed");
        backupList.removeAt(idx);
        dataChanged(QModelIndex(),QModelIndex());
    }
}

int BackupListModel::rowCount ( const QModelIndex & parent ) const {
    if (parent.isValid()) {
        return 0;
    } else {
        return backupList.size();
    }
}

QVariant BackupListModel::data ( const QModelIndex & index, int role ) const{
    //TRACE(qDebug()<<"BackupListModel::data row="<<index.row()<<"role="<<role);
    if (index.row() < 0 || index.row() >= backupList.size())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        //TRACE(qDebug()<<"row="<<index.row();)
        return backupList.at(index.row())->externalSettings->title;
        break;
    case Qt::DecorationRole:
        return *backupIcon;
        break;
    case Qt::CheckStateRole:
        //TRACE(qDebug()<<"CheckStateRole";)
        return QVariant(); // <-- VERY IMPORTANT
        break;
    case Qt::SizeHintRole:
        return QSize(128,64);
        break;
    case Qt::FontRole:
        return QFont("",12);
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignVCenter;
        break;
    case Qt::BackgroundRole:
        return QBrush();
        break;
    case Qt::ForegroundRole:
        return QBrush();
        break;

    }
    TRACE(qDebug()<<"BackupListModel::data !! Role not Handled !! "<<"role="<<role);
    return QVariant();
}

Qt::ItemFlags BackupListModel::flags ( const QModelIndex & index ) const {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int BackupListModel::columnCount ( const QModelIndex & parent ) const  {
    return 1;
}
/*
QModelIndex BackupListModel::index(int row, int column, const QModelIndex &parent = QModelIndex()) const{
    if (parent.isValid()) {
        return QModelIndex();
    } else {
        return createIndex(row,column,(void *)BackupList.at(row));
    }
}

QModelIndex BackupListModel::parent(const QModelIndex &child) const {
    return QModelIndex();
}
*/
