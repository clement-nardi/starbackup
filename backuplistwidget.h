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

#ifndef BACKUPLISTWIDGET_H
#define BACKUPLISTWIDGET_H

#include <QWidget>
#include <QList>
#include "backupengine.h"
#include <QVBoxLayout>
#include "backuplistmodel.h"
#include <QProgressBar>
#include <QListView>
#include <QScrollArea>
#include <QLabel>
#include <QStackedWidget>

class NonScrollableListView : public QListView
{
    Q_OBJECT
public:
    NonScrollableListView(QWidget *parent = 0);
protected:
    void wheelEvent ( QWheelEvent * e );
    void mousePressEvent ( QMouseEvent * event );


};

class CustomProgressBar : public QWidget
{
    Q_OBJECT
public:
    CustomProgressBar(QWidget *parent = 0);

    QProgressBar * pb;
    QLabel * l;

protected:

};

class BackupListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BackupListWidget(QWidget *parent = 0);

    QStackedWidget *sw;
    NonScrollableListView *lw;
    BackupListModel *backupListModel;
    QVBoxLayout * layout;
    QVBoxLayout * pbl;
    QList<CustomProgressBar *> pbList;

signals:

public slots:
    void updatePBList();
    void updatePBs();

};

class ScrollAreaWithWheels : public QScrollArea
{
    Q_OBJECT
public:
    ScrollAreaWithWheels(QWidget *parent = 0);
protected:
    bool eventFilter ( QObject * watched, QEvent * event );

};

#endif // BACKUPLISTWIDGET_H
