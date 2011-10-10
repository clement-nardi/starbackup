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

#include "languagechooser.h"
#include "ui_languagechooser.h"
#include <QFileInfoList>
#include <QDir>
#include "trace.h"

LanguageChooser::LanguageChooser(QString _LanguageFolder, QApplication *_a, QSettings *_s, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LanguageChooser)
{
    LanguageFolder = _LanguageFolder;
    a = _a;
    s = _s;
    humanChoice = false;

    languageMap.clear();
    languageMap.insert("en","English");
    languageMap.insert("fr","Français");
    languageMap.insert("it","Italiano");
    languageMap.insert("es","Español");
    static const QChar chinese[3] = { 0x4E2D, 0x56FD, 0x000A };
    languageMap.insert("zh",QString(chinese,2));

    starTranslator = new QTranslator(a);

    if (!s->contains("language")) {
        s->setValue("language",QLocale::system().name().left(2));
    }
    loadLanguageFromSettings();

    ui->setupUi(this);
    connect(ui->languageList,SIGNAL(currentIndexChanged(int)),this,SLOT(loadLanguage()));
    connect(ui->OKButton,SIGNAL(clicked()),this,SLOT(hide()));

}

LanguageChooser::~LanguageChooser()
{
    delete ui;
}

void LanguageChooser::addLanguage(QString locale){

    TRACE(qDebug()<<locale);
    ui->languageList->addItem(QIcon(":/icons/resources/flag_" + locale + ".png"),(languageMap.value(locale).size()>0)?languageMap.value(locale):
                                                                   locale);
}

void LanguageChooser::reloadAndShow(){
    humanChoice = false;
    ui->languageList->clear();
    addLanguage("en");
    QFileInfoList list = QDir(LanguageFolder).entryInfoList(QStringList()<<"starbackup_*.qm",QDir::NoFilter,QDir::Name);
    for (int fileIdx = 0; fileIdx < list.size(); fileIdx++) {
        addLanguage(list.at(fileIdx).fileName().mid(11,2));
    }
    for (int listIdx = 0; listIdx < ui->languageList->count(); listIdx++) {
        TRACE(qDebug()<<s->value("language").toString()<<languageMap.key(ui->languageList->itemText(listIdx),ui->languageList->itemText(listIdx)));
        if (s->value("language").toString() == languageMap.key(ui->languageList->itemText(listIdx),ui->languageList->itemText(listIdx))) {
            ui->languageList->setCurrentIndex(listIdx);
            break;
        }
    }
    humanChoice = true;
    show();
}

void LanguageChooser::loadLanguage(){
    if (humanChoice) {
        s->setValue("language",languageMap.key(ui->languageList->currentText(),ui->languageList->currentText()));
        TRACE(qDebug()<<s->value("language").toString());
        loadLanguageFromSettings();
    }
}

void LanguageChooser::loadLanguageFromSettings() {
    starTranslator->load(LanguageFolder + "/starbackup_" + s->value("language").toString() + ".qm");
    a->installTranslator(starTranslator);
    languageChanged();
}

void LanguageChooser::retranslate() {
    ui->retranslateUi(this);
}
