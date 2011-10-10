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


#ifndef PROFILECREATOR_H
#define PROFILECREATOR_H

#include <QWidget>
#include "backupsettings.h"

#include "checkablefoldertree.h"
#include "onlycheckedfoldertree.h"

namespace Ui {
    class profilecreator;
}

class ProfileCreator : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileCreator(QWidget *parent = 0);
    ~ProfileCreator();

    void loadSettings();
    void saveSettings();

    BackupSettings *bs;
    OnlyCheckedFolderTree *previewModel;
    CheckableFolderTree   *checkableModel;
    Ui::profilecreator *ui;

public slots:
    void organizationChanged(int idx);
    void handleRealTime(bool status);
    void retranslate();

};

#endif // PROFILECREATOR_H
