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
#include "backuplistwidget.h"
#include "trace.h"
#include <QMessageBox>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QLineEdit>
#include <QStackedLayout>
#include <QIcon>
#include <QPushButton>
#include <QLabel>
#include <QStyle>
#include <QListView>
#include "backuplistmodel.h"
#include <QWheelEvent>


NonScrollableListView::NonScrollableListView(QWidget *parent) :
    QListView(parent)
{
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAutoScroll(false);
    setAutoScrollMargin(0);
}

void NonScrollableListView::wheelEvent ( QWheelEvent * e ){
    TRACE(qDebug()<<"wheel event in list widget!!");
    e->ignore();
}

void NonScrollableListView::mousePressEvent ( QMouseEvent * event ) {
    TRACE(qDebug()<<"mouse press event in list widget!!");
    if (indexAt(event->pos()).isValid()) {
        QListView::mousePressEvent(event);
    } else {
        clearSelection();
    }
}


CustomProgressBar::CustomProgressBar(QWidget *parent) :
    QWidget(parent) {

    //setMaximumHeight(64);
    pb = new QProgressBar(this);
    pb->setMinimumSize(64,64);
    pb->setMaximumHeight(64);
    pb->setTextVisible(false);
    pb->setValue(0);
    pb->setEnabled(false);

    QWidget *wh = new QWidget(this);
    QVBoxLayout *vl = new QVBoxLayout(wh);

    QWidget * w = new QWidget(wh);
    //w->setMaximumHeight(44);
    QHBoxLayout * hl = new QHBoxLayout(w);
    l = new QLabel(w);
    l->setMaximumHeight(20);
    l->setLayoutDirection(Qt::RightToLeft);
    //l->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    hl->setSpacing(0);
    hl->setMargin(0);
    hl->addSpacing(86);
    hl->addWidget(l);

    vl->setSpacing(0);
    vl->setMargin(0);
    vl->addSpacing(44);
    vl->addWidget(w);

    QStackedLayout * sl = new QStackedLayout(this);
    sl->setStackingMode(QStackedLayout::StackAll);
    sl->setSpacing(0);
    sl->setMargin(0);
    sl->addWidget(wh);
    sl->addWidget(pb);
    //sl->setSizeConstraint(QLayout::SetMaximumSize);

    setLayout(sl);
    //setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Expanding);
    //setMaximumWidth(250);

}

BackupListWidget::BackupListWidget(QWidget *parent) :
    QWidget(parent)
{

    lw = new NonScrollableListView(this);
    backupListModel = new BackupListModel(this);
    connect(backupListModel,SIGNAL(dataChanged(QModelIndex,QModelIndex)),this,SLOT(updatePBList()));
    lw->setIconSize(QSize(64,64));
    lw->setModel(backupListModel);
    lw->setContextMenuPolicy(Qt::CustomContextMenu);
    lw->setStyleSheet("background:transparent");
    lw->setAutoFillBackground(false);


    QWidget * pbw = new QWidget(this);
    pbl = new QVBoxLayout(pbw);
    pbl->setMargin(0);
    pbl->setSpacing(0);
    pbl->setContentsMargins(QMargins(0,0,0,0));
    pbl->addStretch(10);
    pbw->setLayout(pbl);

    QStackedLayout * sl = new QStackedLayout(this);
    sl->addWidget(lw);
    sl->addWidget(pbw);
    sl->setStackingMode(QStackedLayout::StackAll);

    setLayout(sl);
    updatePBList();

    QScrollArea *sa = new QScrollArea();
    sa->setWidget(this);

    QTimer * timer = new QTimer(this);
    timer->setInterval(100);
    timer->setSingleShot(false);
    connect(timer,SIGNAL(timeout()),this,SLOT(updatePBs()));
    timer->start();
    //setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

}




void BackupListWidget::updatePBs() {
    for (int i = 0; i<backupListModel->backupList.size(); i++) {
        BackupEngine * be = backupListModel->backupList.at(i);
        CustomProgressBar * cpb = pbList.at(i);
        int total = be->totalFiles;
        if (total>0) {
            cpb->pb->setValue(100*be->nbFiles/total);
        } else {
            cpb->pb->setValue(0);
        }
        QString status = be->getStatus();
        if (status.size()>0) {
            cpb->l->setText(status);
        } else {
            QString unit = tr("seconds");
            int number = QDateTime::currentDateTime().secsTo(be->nextLaunch);
            if (number>60) {
                number = (number+30)/60;
                unit = tr("minutes");
                if (number > 60) {
                    number  = (number+30)/60;
                    unit = tr("hours");
                    if (number > 24) {
                        number  = (number+12)/24;
                        unit = tr("days");
                    }
                }
            }
            if (be->successful) {
                cpb->l->setText(QString(tr("Next launch in %1 %2."))
                                .arg(number).arg(unit));
            } else {
                cpb->l->setText(QString(tr("Next tentative in %1 %2."))
                                .arg(number).arg(unit));
            }
        }
    }
}

void BackupListWidget::updatePBList() {
    TRACE(qDebug()<<"updatePBList "<<pbList.size()<<backupListModel->backupList.size());
    for (int i = 0; i<pbList.size(); i++) {
        pbl->removeWidget(pbList.at(i));
    }
    TRACE(qDebug()<<"updatePBList: widget removed");
    while (pbList.size()<backupListModel->backupList.size()) {
        CustomProgressBar * pb = new CustomProgressBar();
        pbList.append(pb);
    }
    TRACE(qDebug()<<"updatePBList: pb created");
    for (int i = 0; i<backupListModel->backupList.size(); i++) {
        pbl->insertWidget(i,pbList.at(i));
    }
    for (int i = backupListModel->backupList.size(); i<pbList.size(); i++) {
        CustomProgressBar * pb = pbList.at(i);
        pbList.removeAt(i);
        pb->deleteLater();
        delete pb;
    }
    TRACE(qDebug()<<"updatePBList: widget added");
}

ScrollAreaWithWheels::ScrollAreaWithWheels(QWidget *parent) :
        QScrollArea(parent) {
    TRACE(qDebug()<<"wheel constructor");

}

bool ScrollAreaWithWheels::eventFilter ( QObject * watched, QEvent * event ) {
    static int eventcount = 0;
    TRACE(qDebug()<<"wheel filter event!!"<<eventcount++);
    if (watched == ((BackupListWidget *)widget())->lw) {
        TRACE(qDebug()<<"  from list widget");
        return true;
    }
    return false;
}
