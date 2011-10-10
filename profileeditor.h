#ifndef PROFILEEDITOR_H
#define PROFILEEDITOR_H

#include <QTabWidget>

namespace Ui {
    class ProfileEditor;
}

class ProfileEditor : public QTabWidget
{
    Q_OBJECT

public:
    explicit ProfileEditor(QWidget *parent = 0);
    ~ProfileEditor();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ProfileEditor *ui;
};

#endif // PROFILEEDITOR_H
