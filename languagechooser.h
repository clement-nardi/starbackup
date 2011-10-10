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

#ifndef LANGUAGECHOOSER_H
#define LANGUAGECHOOSER_H

#include <QWidget>
#include <QApplication>
#include <QTranslator>
#include <QSettings>

namespace Ui {
    class LanguageChooser;
}

class LanguageChooser : public QWidget
{
    Q_OBJECT

public:
   // explicit LanguageChooser(QWidget *parent = 0);
    explicit LanguageChooser(QString LanguageFolder, QApplication * _a, QSettings *_s, QWidget *parent = 0);
    ~LanguageChooser();
    QSettings *s;

signals:
    void languageChanged();

public slots:
    void reloadAndShow();
    void retranslate();
private slots:
    void loadLanguage();

private:
    void loadLanguageFromSettings();
    Ui::LanguageChooser *ui;
    QString LanguageFolder;
    void addLanguage(QString locale);
    QApplication *a;
    QTranslator * starTranslator;
    QMap<QString,QString> languageMap;
    bool humanChoice;
};

#endif // LANGUAGECHOOSER_H
