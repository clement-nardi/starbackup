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

#include "destinationwidget.h"
#include <QFileDialog>
#include "trace.h"
#include <QDesktopServices>

DestinationLineWidget::DestinationLineWidget(QWidget *parent) :
    QWidget(parent)
{
    removeButton = new QPushButton(QIcon(":/icons/remove"),"");
    removeButton->setToolTip(tr("Remove this destination folder"));
    editButton = new QPushButton(QIcon(":/icons/folder"),"");
    editButton->setToolTip(tr("Choose destination folder"));
    destLine = new QLineEdit();
    destLine->setEnabled(false);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(destLine);
    layout->addWidget(editButton);
    layout->addWidget(removeButton);
    this->setMinimumHeight(20);
    layout->setMargin(0);
    setLayout(layout);
    connect(editButton,SIGNAL(clicked()),this,SLOT(browseFolder()));
    connect(removeButton,SIGNAL(clicked()),this,SLOT(eraseFolder()));
}

void DestinationLineWidget::browseFolder(){
    QString dir = destLine->text();
    dir = QFileDialog::getExistingDirectory(this,
                                            tr("Choose the destination directory"),
                                            QDesktopServices::storageLocation(QDesktopServices::DesktopLocation),
                                            QFileDialog::ShowDirsOnly);
    if (dir.size()>0) {
        QString path = QDir(dir).canonicalPath();
        destLine->setText(path);
    }
}

void DestinationLineWidget::eraseFolder(){
    destLine->setText("");
}

DestinationWidget::DestinationWidget(QWidget *parent) :
    QWidget(parent) {
    lines = new QList<DestinationLineWidget *>();
    layout = new QVBoxLayout(this);
    layout->setMargin(3);
    updateLines();
    layout->addStretch(10);


    setLayout(layout);
}

void DestinationWidget::addLine(QString str){
    DestinationLineWidget * line = new DestinationLineWidget(this);
    lines->append(line);
    layout->insertWidget(lines->size()-1,line);
    line->destLine->setText(str);
    connect(line->destLine,SIGNAL(textChanged(QString)),this, SLOT(updateLines()));
}

void DestinationWidget::updateLines() {
    int nbEmpty = 0;
    for (int i = 0; i<lines->size(); i++) {
        if (lines->at(i)->destLine->text().size() == 0) {
            nbEmpty++;
        }
        lines->at(i)->destLine->setToolTip(lines->at(i)->destLine->text());
        lines->at(i)->destLine->setCursorPosition(0);
    }
    TRACE(qDebug()<<nbEmpty);
    for (int i = 0; i<lines->size(); i++) {
        if (lines->at(i)->destLine->text().size() == 0) {
            if (nbEmpty>1) {
                DestinationLineWidget *tempLine = lines->at(i);
                layout->removeWidget(tempLine);
                lines->removeAt(i);
                i--;
                tempLine->deleteLater();
                nbEmpty--;
            }
        }
    }
    if (nbEmpty == 0) {
        addLine();
    }
    dataChanged();
}

void DestinationWidget::resetDirs(QList<DestFolderInfo> list){
    while (lines->size()>0){
        DestinationLineWidget *tempLine = lines->at(0);
        layout->removeWidget(tempLine);
        lines->removeAt(0);
        tempLine->deleteLater();
    }
    for (int i = 0; i<list.size(); i++) {
        addLine(list.at(i));
    }
    updateLines();
}
