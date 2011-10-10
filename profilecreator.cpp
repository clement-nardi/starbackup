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

#include "profilecreator.h"
#include "ui_profilecreator.h"
#include "trace.h"
#include "math.h"
#include <QFileDialog>

ProfileCreator::ProfileCreator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::profilecreator)
{
    ui->setupUi(this);
    QButtonGroup *scheduleGroup = new QButtonGroup(this);
    scheduleGroup->addButton(ui->realTimeRadio);
    scheduleGroup->addButton(ui->scheduledRadio);
    scheduleGroup->addButton(ui->noScheduleRadio);

    checkableModel = new CheckableFolderTree(this);
    //destModel = new DestinationModel(this);
    previewModel = new OnlyCheckedFolderTree(checkableModel,ui->destDirs,this);
    ui->SourceFoldersTree->setModel(checkableModel);
    //ui->destView->setModel(destModel);
    ui->PreviewTree->setItemsExpandable(false);
    ui->PreviewTree->setModel(previewModel);
    ui->PreviewTree->expandAll();
    connect(previewModel,SIGNAL(dataChanged(QModelIndex,QModelIndex)),ui->PreviewTree,SLOT(expandAll()));
    connect(ui->destDirs,SIGNAL(dataChanged()),ui->PreviewTree,SLOT(expandAll()));
    connect(ui->organizationBox,SIGNAL(currentIndexChanged(int)),this,SLOT(organizationChanged(int)));
    connect(ui->realTimeRadio,SIGNAL(toggled(bool)),this,SLOT(handleRealTime(bool)));
    //connect(ui->openDest,SIGNAL(clicked()),this,SLOT(openDestClicked()));
}

ProfileCreator::~ProfileCreator()
{
    delete ui;
}

void ProfileCreator::organizationChanged(int idx) {
    checkableModel->fullyCheckedFolders.organization = idx;
    previewModel->SourceDataChanged();
}

void ProfileCreator::loadSettings(){

    ui->title->setText(bs->title);
    ui->destDirs->resetDirs(bs->destDirs);
    checkableModel->setCheckedFolders(bs->sourceDirs);
    ui->organizationBox->setCurrentIndex(bs->sourceDirs.organization);
    ui->versionning         ->setChecked        (bs->versioning);
    ui->maxiPercent         ->setChecked        (bs->maxiPercent);
    ui->maxiValue           ->setChecked        (bs->maxiValue);
    ui->keepPercent         ->setChecked        (bs->keepPercent);
    ui->maxiPercent_value   ->setValue          (bs->maxiPercent_value);
    ui->maxiValue_value     ->setValue          (bs->maxiValue_value);
    ui->keepPercent_value   ->setValue          (bs->keepPercent_value);
    ui->maxiValue_unit      ->setCurrentIndex   (bs->maxiValue_unit);
    ui->maxiDays            ->setChecked        (bs->maxiDays);
    ui->maxiDays_value      ->setValue          (bs->maxiDays_value);

    ui->catchUpBox->setChecked(bs->catchUpMissed);
    ui->catchUpValue->setValue(bs->catchUpValue);
    ui->realTimeRadio->setChecked(bs->realTime);
    ui->scheduledRadio->setChecked(bs->scheduled);
    ui->noScheduleRadio->setChecked(bs->noSchedule);
    ui->daylyRadio->setChecked(bs->dayly);
    ui->weeklyRadio->setChecked(bs->weekly);
    ui->everyRadio->setChecked(bs->every);
    ui->daylyHour->setTime(bs->daylyTime);
    ui->weeklyHour->setTime(bs->weeklyTime);
    ui->weekDay->setCurrentIndex(bs->weeklyDay-1);
    ui->everyNb->setValue(bs->everyNb);
    ui->everyUnit->setCurrentIndex(bs->everyUnit);
}

void ProfileCreator::saveSettings(){
    bs->title = ui->title->text();
    bs->destDirs.clear();
    for (int i = 0; i<ui->destDirs->lines->size()-1; i++) {
        bs->destDirs.append(DestFolderInfo(ui->destDirs->lines->at(i)->destLine->text()));
    }
    bs->sourceDirs.clear();
    bs->sourceDirs.append(checkableModel->fullyCheckedFolders);
    bs->sourceDirs.organization = checkableModel->fullyCheckedFolders.organization;

    for (int i = 0; i<bs->destDirs.size(); i++) {
        bs->destDirs[i].loadHardwareID();
    }
    for (int i = 0; i<bs->sourceDirs.size(); i++) {
        bs->sourceDirs[i].loadHardwareID();
    }


    bs->versioning         = ui->versionning       ->isChecked();
    bs->maxiPercent         = ui->maxiPercent       ->isChecked();
    bs->maxiValue           = ui->maxiValue         ->isChecked();
    bs->keepPercent         = ui->keepPercent       ->isChecked();
    bs->maxiPercent_value   = ui->maxiPercent_value ->value();
    bs->maxiValue_value     = ui->maxiValue_value   ->value();
    bs->keepPercent_value   = ui->keepPercent_value ->value();
    bs->maxiValue_unit      = ui->maxiValue_unit    ->currentIndex();
    bs->maxiDays            = ui->maxiDays          ->isChecked();
    bs->maxiDays_value      = ui->maxiDays_value    ->value();

    bs->catchUpMissed = ui->catchUpBox->isChecked();
    bs->catchUpValue = ui->catchUpValue->value();
    bs->realTime = ui->realTimeRadio->isChecked();
    bs->scheduled = ui->scheduledRadio->isChecked();
    bs->noSchedule = ui->noScheduleRadio->isChecked();
    bs->dayly = ui->daylyRadio->isChecked();
    bs->weekly = ui->weeklyRadio->isChecked();
    bs->every = ui->everyRadio->isChecked();
    bs->daylyTime = ui->daylyHour->time();
    bs->weeklyTime = ui->weeklyHour->time();
    bs->weeklyDay = (Qt::DayOfWeek)(ui->weekDay->currentIndex()+1);
    bs->everyNb = ui->everyNb->value();
    bs->everyUnit = ui->everyUnit->currentIndex();

    bs->saveToXML();
}


void ProfileCreator::handleRealTime(bool status){
    if (status) {
        ui->catchUpBox->setChecked(true);
        ui->catchUpBox->setEnabled(false);
    } else {
        ui->catchUpBox->setEnabled(true);
    }
}

void ProfileCreator::retranslate() {
    ui->retranslateUi(this);
}
